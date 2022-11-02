// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IMediaTextureSample.h"
#include "MediaObjectPool.h"
#include "RHI.h"
#include "RHIUtilities.h"

#include "Windows/AllowWindowsPlatformTypes.h" 
#include <d3d11on12.h>
#include "Spout.h"
#include "Windows/HideWindowsPlatformTypes.h"

class FRHITexture2D;
class FSpout2MediaPlayer;

class SPOUT2MEDIA_API FSpout2MediaTextureSample
	: public IMediaTextureSample
	, public IMediaPoolable
{
	FTexture2DRHIRef Texture;

	FString RHIName;
	ID3D11Resource* WrappedDX11Resource = nullptr;
	
public:
	
	FSpout2MediaTextureSample();
	~FSpout2MediaTextureSample();

	struct InitializeArguments
	{
		int Width, Height;
		
		DXGI_FORMAT DXFormat;
		EPixelFormat PixelFormat;
		
		HANDLE SpoutSharehandle;

		ID3D11DeviceContext* Context;
		
		ID3D11Device* D3D11Device;
		ID3D12Device* D3D12Device;
		ID3D11On12Device* D3D11on12Device;

		bool bSRGB;
	} Args;
	
	void Initialize(const InitializeArguments& Args);
	void Destroy();

	void CopyResource(ID3D11Resource* SrcTexture);
	
public:
	//~ IMediaTextureSample interface
	virtual const void* GetBuffer() override;
	virtual FIntPoint GetDim() const override;
	virtual FTimespan GetDuration() const override;
	virtual EMediaTextureSampleFormat GetFormat() const override;
	virtual FIntPoint GetOutputDim() const override;
	virtual uint32 GetStride() const override;
	virtual FRHITexture* GetTexture() const override;
	virtual FMediaTimeStamp GetTime() const override;
	virtual bool IsCacheable() const override;
	virtual bool IsOutputSrgb() const override;

public:
	//~ IMediaPoolable interface
	virtual void ShutdownPoolable() override;
};
