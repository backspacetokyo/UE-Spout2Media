// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaCapture.h"
#include "RHICommandList.h"
#include "ID3D12DynamicRHI.h"

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
	TMap<FTextureRHIRef, ID3D11Resource*> WrappedDX11ResourceMap;

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
		DXGI_FORMAT TextureFormat = DXGI_FORMAT_UNKNOWN;

		if (RHIName == TEXT("D3D11"))
		{
			D3D11Device = static_cast<ID3D11Device*>(GDynamicRHI->RHIGetNativeDevice());
			// Retain the borrowed RHI device for the matching Release in DisposeSpout.
			D3D11Device->AddRef();
			D3D11Device->GetImmediateContext(&DeviceContext);
			D3D11_TEXTURE2D_DESC Desc = {};
			static_cast<ID3D11Texture2D*>(InTexture->GetNativeResource())->GetDesc(&Desc);
			TextureFormat = Desc.Format;
		}
		else if (RHIName == TEXT("D3D12"))
		{
			ID3D12Device* Device12 = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
			UINT DeviceFlags11 = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
			IUnknown* GraphicsQueue = static_cast<ID3D12CommandQueue*>(GDynamicRHI->RHIGetNativeGraphicsQueue());

			verify(D3D11On12CreateDevice(
				Device12,
				DeviceFlags11,
				nullptr,
				0,
				&GraphicsQueue,
				1,
				0,
				&D3D11Device,
				&DeviceContext,
				nullptr
			) == S_OK);

			verify(D3D11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&D3D11on12Device) == S_OK);
			TextureFormat = static_cast<ID3D12Resource*>(InTexture->GetNativeResource())->GetDesc().Format;
		}

		verify(senders.CreateSender(SenderName_str.c_str(), Width, Height, SharedSendingHandle, TextureFormat));
		verify(sdx.CreateSharedDX11Texture(D3D11Device, Width, Height, TextureFormat, &SendingTexture, SharedSendingHandle));
	}

	void DisposeSpout()
	{
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

		{
			for (auto Iter : WrappedDX11ResourceMap)
			{
				Iter.Value->Release();
			}
			WrappedDX11ResourceMap.Reset();
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
				return static_cast<ID3D11Texture2D*>(*Iter);

			// Borrowed from the RHI texture; do not Release this pointer.
			ID3D12Resource* NativeTex = static_cast<ID3D12Resource*>(InTexture->GetNativeResource());
			ID3D11Resource* WrappedDX11Resource = nullptr;
			
			D3D11_RESOURCE_FLAGS rf11 = {};
			verify(D3D11on12Device->CreateWrappedResource(
				NativeTex, &rf11,
				D3D12_RESOURCE_STATE_COPY_SOURCE,
				D3D12_RESOURCE_STATE_COPY_SOURCE, __uuidof(ID3D11Resource),
				(void**)&WrappedDX11Resource) == S_OK);

			WrappedDX11ResourceMap.Add(InTexture, WrappedDX11Resource);
			// New wrappers start acquired; use the same acquire/release cycle on every copy.
			D3D11on12Device->ReleaseWrappedResources(&WrappedDX11Resource, 1);

			return static_cast<ID3D11Texture2D*>(WrappedDX11Resource);
		}
		
		return nullptr;
	}

	void SendTexture(FTextureRHIRef InTexture)
	{
		if (!DeviceContext)
			return;

		auto Texture = GetTextureResource(InTexture);
		ID3D11Resource* WrappedResource = Texture;
		if (D3D11on12Device)
			D3D11on12Device->AcquireWrappedResources(&WrappedResource, 1);
		DeviceContext->CopyResource(SendingTexture, Texture);
		if (D3D11on12Device)
			D3D11on12Device->ReleaseWrappedResources(&WrappedResource, 1);
		DeviceContext->Flush();
		
		verify(senders.UpdateSender(SenderName_str.c_str(),
			Width, Height,
			SharedSendingHandle));
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
	DisposeSpout();
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
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
#endif
	// Submit UE's producer work; D3D11 also shares its immediate context with the RHI thread.
	RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
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
			Context.Reset();
		
		Context = MakeShared<FSpoutSenderContext, ESPMode::ThreadSafe>(
			SenderName, Width, Height, PixelFormat, InTexture);
	}

	if (Context)
	{
		if (IsRHID3D12())
		{
			// The RHI thread can finish before the D3D12 submission thread submits UE's work.
			GetID3D12DynamicRHI()->RHIRunOnQueue(ED3D12RHIRunOnQueueType::Graphics,
				[SenderContext = Context, InTexture](ID3D12CommandQueue*)
				{
					SenderContext->SendTexture(InTexture);
				}, /*bWaitForSubmission*/ true);
		}
		else
		{
			Context->SendTexture(InTexture);
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
