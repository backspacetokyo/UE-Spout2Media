// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaCapture.h"
#include "ID3D12DynamicRHI.h"
#include "RenderingThread.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpoutInterop, Log, All);

#include "Windows/AllowWindowsPlatformTypes.h" 
#include <d3d11on12.h>
#include "Spout.h"
#include "Windows/HideWindowsPlatformTypes.h"

struct USpout2MediaCapture::FSpoutSenderContext
{
	FString SenderName;
	uint32 Width, Height;
	EPixelFormat PixelFormat;

	std::string SenderName_str;

	ID3D11DeviceContext* DeviceContext = nullptr;
	ID3D11Device* D3D11Device = nullptr;
	
	ID3D11On12Device* D3D11on12Device = nullptr;
	TMap<FTextureRHIRef, TRefCountPtr<ID3D11Resource>> WrappedDX11ResourceMap;
	bool bReady = false;
	uint64 SentFrames = 0;
	DXGI_FORMAT SharedFormat = DXGI_FORMAT_UNKNOWN;

	spoutSenderNames senders;
	spoutDirectX sdx;

	ID3D11Texture2D* SendingTexture = nullptr;
	HANDLE SharedSendingHandle = nullptr;

	FSpoutSenderContext(const FString& SenderName,
		uint32 Width, uint32 Height, EPixelFormat PixelFormat,
		FTextureRHIRef InTexture)
		: SenderName(SenderName)
		, Width(Width)
		, Height(Height)
		, PixelFormat(PixelFormat)
	{
		SenderName_str = TCHAR_TO_ANSI(*SenderName);
		InitSpout(InTexture);
	}

	~FSpoutSenderContext()
	{
		DisposeSpout();
	}

	void InitSpout(FTextureRHIRef InTexture)
	{
		const FString RHIName = GDynamicRHI->GetName();

		if (RHIName == TEXT("D3D11"))
		{
			D3D11Device = static_cast<ID3D11Device*>(GDynamicRHI->RHIGetNativeDevice());
			D3D11Device->AddRef(); // RHIGetNativeDevice returns a borrowed pointer.
			D3D11Device->GetImmediateContext(&DeviceContext);
		}
		else if (RHIName == TEXT("D3D12"))
		{
			ID3D12Device* Device12 = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
			UINT DeviceFlags11 = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

			IUnknown* Queues[] = { GetID3D12DynamicRHI()->RHIGetCommandQueue() };
			const HRESULT Result = D3D11On12CreateDevice(
				Device12,
				DeviceFlags11,
				nullptr,
				0,
				Queues,
				1,
				0,
				&D3D11Device,
				&DeviceContext,
				nullptr
			);
			if (FAILED(Result))
			{
				UE_LOG(LogSpoutInterop, Error, TEXT("D3D11On12 initialization failed: 0x%08x"), Result);
				return;
			}
			const HRESULT InterfaceResult = D3D11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&D3D11on12Device);
			if (FAILED(InterfaceResult))
			{
				UE_LOG(LogSpoutInterop, Error, TEXT("D3D11On12 interface failed: 0x%08x"), InterfaceResult);
				return;
			}
		}
		else
		{
			UE_LOG(LogSpoutInterop, Error, TEXT("Unsupported RHI: %s"), *RHIName);
			return;
		}

		DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
		if (D3D11on12Device)
		{
			Format = static_cast<ID3D12Resource*>(InTexture->GetNativeResource())->GetDesc().Format;
		}
		else
		{
			D3D11_TEXTURE2D_DESC Desc;
			static_cast<ID3D11Texture2D*>(InTexture->GetNativeResource())->GetDesc(&Desc);
			Format = Desc.Format;
		}
		if (!sdx.CreateSharedDX11Texture(D3D11Device, Width, Height, Format, &SendingTexture, SharedSendingHandle)
			|| !senders.CreateSender(SenderName_str.c_str(), Width, Height, SharedSendingHandle, Format))
		{
			UE_LOG(LogSpoutInterop, Error, TEXT("Shared Spout texture creation failed"));
			return;
		}
		bReady = true;
		SharedFormat = Format;
		UE_LOG(LogSpoutInterop, Log, TEXT("Sender ready: %s %ux%u format=%u shared-graphics-queue"), *SenderName, Width, Height, uint32(Format));
	}

	void DisposeSpout()
	{
		if (bReady)
		{
			senders.ReleaseSenderName(SenderName_str.c_str());
			UE_LOG(LogSpoutInterop, Log, TEXT("Sender stopped: %s frames=%llu"), *SenderName, SentFrames);
		}
		bReady = false;
		// Release COM wrappers while their device/context are still alive. Each
		// per-frame acquisition has already been released and flushed.
		WrappedDX11ResourceMap.Reset();
		if (SendingTexture)
		{
			SendingTexture->Release();
			SendingTexture = nullptr;
		}

		if (DeviceContext)
		{
			DeviceContext->Release();
			DeviceContext = nullptr;
		}

		if (D3D11on12Device)
		{
			D3D11on12Device->Release();
			D3D11on12Device = nullptr;
		}

		if (D3D11Device)
		{
			D3D11Device->Release();
			D3D11Device = nullptr;
		}
	}

	ID3D11Texture2D* GetTextureResource(FTextureRHIRef InTexture)
	{
		const FString RHIName = GDynamicRHI->GetName();
		
		if (RHIName == TEXT("D3D11"))
		{
			return static_cast<ID3D11Texture2D*>(InTexture->GetNativeResource());
		}
		else if (RHIName == TEXT("D3D12"))
		{
			if (auto Iter = WrappedDX11ResourceMap.Find(InTexture))
				return static_cast<ID3D11Texture2D*>(Iter->GetReference());

			ID3D12Resource* NativeTex = static_cast<ID3D12Resource*>(InTexture->GetNativeResource());
			ID3D11Resource* WrappedDX11Resource = nullptr;
			
			D3D11_RESOURCE_FLAGS rf11 = {};
			const HRESULT Result = D3D11on12Device->CreateWrappedResource(
				NativeTex, &rf11,
				D3D12_RESOURCE_STATE_COPY_SOURCE,
				D3D12_RESOURCE_STATE_COPY_SOURCE, __uuidof(ID3D11Resource),
				(void**)&WrappedDX11Resource);
			if (FAILED(Result))
			{
				UE_LOG(LogSpoutInterop, Error, TEXT("CreateWrappedResource failed: 0x%08x"), Result);
				return nullptr;
			}

			// GetNativeResource is borrowed; never Release the RHI's reference.
			WrappedDX11ResourceMap.Add(InTexture, TRefCountPtr<ID3D11Resource>(WrappedDX11Resource, false));

			return static_cast<ID3D11Texture2D*>(WrappedDX11Resource);
		}
		
		return nullptr;
	}

	bool Tick_RenderThread(FTextureRHIRef InTexture)
	{
		const FString RHIName = GDynamicRHI->GetName();

		if (!bReady || !DeviceContext)
			return false;

		auto Texture = GetTextureResource(InTexture);
		if (!Texture)
			return false;
		if (D3D11on12Device)
		{
			ID3D11Resource* Wrapped = Texture;
			D3D11on12Device->AcquireWrappedResources(&Wrapped, 1);
		}
		DeviceContext->CopyResource(SendingTexture, Texture);
		if (D3D11on12Device)
		{
			ID3D11Resource* Wrapped = Texture;
			D3D11on12Device->ReleaseWrappedResources(&Wrapped, 1);
		}
		DeviceContext->Flush();
		++SentFrames;
		
		return senders.UpdateSender(SenderName_str.c_str(),
			Width, Height,
			SharedSendingHandle, SharedFormat);
	}
};

USpout2MediaCapture::USpout2MediaCapture(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

bool USpout2MediaCapture::HasFinishedProcessing() const
{
	return Super::HasFinishedProcessing();
}

bool USpout2MediaCapture::ValidateMediaOutput() const
{
	USpout2MediaOutput* Output = CastChecked<USpout2MediaOutput>(MediaOutput);
	check(Output);
	return true;
}

bool USpout2MediaCapture::InitializeCapture()
{
	USpout2MediaOutput* Output = CastChecked<USpout2MediaOutput>(MediaOutput);
	check(Output);
	return InitSpout(Output);
}

bool USpout2MediaCapture::UpdateSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport)
{
	return true;
}

void USpout2MediaCapture::StopCaptureImpl(bool bAllowPendingFrameToBeProcess)
{
	Super::StopCaptureImpl(bAllowPendingFrameToBeProcess);
	SetState(EMediaCaptureState::Stopped);
	TSharedPtr<FSpoutSenderContext, ESPMode::ThreadSafe> OldContext = MoveTemp(Context);
	ENQUEUE_RENDER_COMMAND(StopSpoutInterop)([OldContext = MoveTemp(OldContext)](FRHICommandListImmediate& RHICmdList) mutable
	{
		if (OldContext)
		{
			RHICmdList.SubmitAndBlockUntilGPUIdle();
			OldContext.Reset();
		}
	});
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
void USpout2MediaCapture::OnRHIResourceCaptured_RenderingThread(FRHICommandListImmediate& RHICmdList, const FCaptureBaseData& InBaseData,
	TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, FTextureRHIRef InTexture)
#else
void USpout2MediaCapture::OnRHIResourceCaptured_RenderingThread(const FCaptureBaseData& InBaseData,
	TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, FTextureRHIRef InTexture)
#endif
{
	USpout2MediaOutput* Output = CastChecked<USpout2MediaOutput>(MediaOutput);

	const FString SenderName = Output->SenderName;
	uint32 Width = InTexture->GetSizeX();
	uint32 Height = InTexture->GetSizeY();
	EPixelFormat PixelFormat = InTexture->GetFormat();

	if (!Context
		|| Context->SenderName != SenderName
		|| Context->Width != Width
		|| Context->Height != Height
		|| Context->PixelFormat != PixelFormat)
	{
		if (Context)
		{
			RHICmdList.SubmitAndBlockUntilGPUIdle();
			Context.Reset();
		}
		
		Context = MakeShared<FSpoutSenderContext, ESPMode::ThreadSafe>(
			SenderName, Width, Height, PixelFormat, InTexture);
	}

	if (Context)
	{
		bool bSent = false;
		if (IsRHID3D12())
		{
			// Flush UE's producer work before inserting D3D11On12 commands on
			// the same graphics queue. Restore CopySrc so RHI state tracking
			// agrees with the state left by ReleaseWrappedResources.
			RHICmdList.Transition(FRHITransitionInfo(InTexture, ERHIAccess::Unknown, ERHIAccess::CopySrc));
			RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
			GetID3D12DynamicRHI()->RHIRunOnQueue(ED3D12RHIRunOnQueueType::Graphics,
				[Sender = Context, InTexture, &bSent](ID3D12CommandQueue*) { bSent = Sender->Tick_RenderThread(InTexture); }, true);
		}
		else
		{
			bSent = Context->Tick_RenderThread(InTexture);
		}
		if (!bSent)
		{
			UE_LOG(LogSpoutInterop, Error, TEXT("Spout submission failed; stopping capture"));
			SetState(EMediaCaptureState::Error);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

bool USpout2MediaCapture::InitSpout(USpout2MediaOutput* Output)
{
	SetState(EMediaCaptureState::Capturing);
	return true;
}

bool USpout2MediaCapture::DisposeSpout()
{
	SetState(EMediaCaptureState::Stopped);

	Context.Reset();
	return true;
}
