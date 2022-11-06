// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaTextureSample.h"

FSpout2MediaTextureSample::FSpout2MediaTextureSample()
	: Args({})
{
}

FSpout2MediaTextureSample::~FSpout2MediaTextureSample()
{
	Destroy();
}

void FSpout2MediaTextureSample::Initialize(const InitializeArguments& Args_)
{
	Args = Args_;

	ETextureCreateFlags Flags = ETextureCreateFlags::RenderTargetable;

	if (Args.bSRGB)
		Flags |= ETextureCreateFlags::SRGB;
	
	FRHIResourceCreateInfo CreateInfo(L"Spout2MediaTextureSample");
	Texture = RHICreateTexture2D(Args.Width, Args.Height, Args.PixelFormat,
		1, 1,
		Flags, CreateInfo);
	
	RHIName = GDynamicRHI->GetName();
	
	if (RHIName == TEXT("D3D12"))
	{
		D3D11_RESOURCE_FLAGS rf11 = {};
		ID3D12Resource* NativeTex = (ID3D12Resource*)Texture->GetNativeResource();
	
		verify(Args.D3D11on12Device->CreateWrappedResource(
			NativeTex, &rf11,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PRESENT, __uuidof(ID3D11Resource),
			(void**)&WrappedDX11Resource) == S_OK);\

		NativeTex->Release();
	}
	
	ENQUEUE_RENDER_COMMAND(SpoutRecieverRenderThreadOp)([this](FRHICommandListImmediate& RHICmdList) {
		check(IsInRenderingThread());
		
		ID3D11Resource* SrcTexture = nullptr;
		
		verify(Args.D3D11Device->OpenSharedResource(Args.SpoutSharehandle, __uuidof(ID3D11Resource), (void**)(&SrcTexture)) == S_OK);
		check(SrcTexture);
	
		CopyResource(SrcTexture);
		
		SrcTexture->Release();
	});
}

void FSpout2MediaTextureSample::Destroy()
{
	if (WrappedDX11Resource)
	{
		Args.D3D11on12Device->ReleaseWrappedResources(&WrappedDX11Resource, 1);
		WrappedDX11Resource = nullptr;
	}

	if (Texture)
	{
		Texture.SafeRelease();
		Texture = nullptr;
	}
}

void FSpout2MediaTextureSample::CopyResource(ID3D11Resource* SrcTexture)
{
	check(IsInRenderingThread());
	if (!GWorld || !SrcTexture) return;

	if (RHIName == TEXT("D3D11"))
	{
		ID3D11Texture2D* NativeTex = (ID3D11Texture2D*)Texture->GetNativeResource();

		Args.Context->CopyResource(NativeTex, SrcTexture);
		Args.Context->Flush();
	}
	else if (RHIName == TEXT("D3D12"))
	{
		Args.D3D11on12Device->AcquireWrappedResources(&WrappedDX11Resource, 1);
		Args.Context->CopyResource(WrappedDX11Resource, SrcTexture);
		Args.D3D11on12Device->ReleaseWrappedResources(&WrappedDX11Resource, 1);
		Args.Context->Flush();
	}
}

const void* FSpout2MediaTextureSample::GetBuffer()
{
	return nullptr;
}

FIntPoint FSpout2MediaTextureSample::GetDim() const
{
	return FIntPoint(Args.Width, Args.Height);
}

FTimespan FSpout2MediaTextureSample::GetDuration() const
{
	return FTimespan();
}

EMediaTextureSampleFormat FSpout2MediaTextureSample::GetFormat() const
{
	switch (Args.PixelFormat)
	{
	case PF_B8G8R8A8:
		return EMediaTextureSampleFormat::CharBGRA;
	case PF_FloatRGB:
		return EMediaTextureSampleFormat::FloatRGB;
	case PF_FloatRGBA:
		return EMediaTextureSampleFormat::FloatRGBA;
	default:
		return EMediaTextureSampleFormat::Undefined;
	}
}

FIntPoint FSpout2MediaTextureSample::GetOutputDim() const
{
	return FIntPoint(Args.Width, Args.Height);
}

uint32 FSpout2MediaTextureSample::GetStride() const
{
	if (!Texture.IsValid())
	{
		return 0;
	}

	return Texture->GetSizeX() * 4;
}

FRHITexture* FSpout2MediaTextureSample::GetTexture() const
{
	return Texture.GetReference();
}

FMediaTimeStamp FSpout2MediaTextureSample::GetTime() const
{
	return FMediaTimeStamp();
}

bool FSpout2MediaTextureSample::IsCacheable() const
{
	return false;
}

bool FSpout2MediaTextureSample::IsOutputSrgb() const
{
	return true;
}

void FSpout2MediaTextureSample::ShutdownPoolable()
{
	Destroy();
}
