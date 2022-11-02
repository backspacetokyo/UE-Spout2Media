// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaPlayer.h"

#include "Windows/AllowWindowsPlatformTypes.h" 
#include <d3d11on12.h>
#include "Spout.h"
#include "Windows/HideWindowsPlatformTypes.h"

// #include "GlobalShader.h"
// #include "UniformBuffer.h"
#include "RHICommandList.h"
// #include "RHIUtilities.h"
#include "MediaShaders.h"
#include "Spout2MediaTextureSample.h"

static spoutSenderNames senders;

// class FTextureCopyVertexShader : public FGlobalShader
// {
// 	DECLARE_SHADER_TYPE(FTextureCopyVertexShader, Global);
// public:
//
// 	static bool ShouldCache(EShaderPlatform Platform) { return true; }
//
// 	FTextureCopyVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
// 		FGlobalShader(Initializer)
// 	{}
// 	FTextureCopyVertexShader() {}
//
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
// 	}
// };
//
// class FTextureCopyPixelShader : public FGlobalShader
// {
// 	DECLARE_SHADER_TYPE(FTextureCopyPixelShader, Global);
// public:
//
// #if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 25) || (ENGINE_MAJOR_VERSION == 5)
// 	LAYOUT_FIELD(FShaderResourceParameter, SrcTexture);
// #else ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 24
// 	FShaderResourceParameter SrcTexture;
//
// 	virtual bool Serialize(FArchive& Ar) override
// 	{
// 		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);
// 		Ar << SrcTexture;
// 		return bShaderHasOutdatedParams;
// 	}
// #endif
//
// 	static bool ShouldCache(EShaderPlatform Platform) { return true; }
//
// 	FTextureCopyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
// 		FGlobalShader(Initializer)
// 	{
// 		SrcTexture.Bind(Initializer.ParameterMap, TEXT("SrcTexture"));
// 	}
// 	FTextureCopyPixelShader() {}
//
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
// 	}
// };
//
// struct FTextureVertex
// {
// 	FVector4 Position;
// 	FVector2D UV;
// };

// IMPLEMENT_SHADER_TYPE(, FTextureCopyPixelShader, TEXT("/Plugin/Spout2Media/SpoutReceiverCopyShader.usf"), TEXT("MainPixelShader"), SF_Pixel)

/////////////////////////////////////////////////////////////////////////////

struct FSpout2MediaPlayer::FSpoutReceiverContext
{
	unsigned int Width = 0, Height = 0;
	DXGI_FORMAT DXFormat = DXGI_FORMAT_UNKNOWN;
	EPixelFormat PixelFormat = PF_Unknown;

	ID3D11DeviceContext* Context = nullptr;
	
	ID3D11Device* D3D11Device = nullptr;
	ID3D12Device* D3D12Device = nullptr;
	ID3D11On12Device* D3D11on12Device = nullptr;

	FSpoutReceiverContext(unsigned int Width, unsigned int Height, DXGI_FORMAT DXFormat)
		: Width(Width)
		, Height(Height)
		, DXFormat(DXFormat)
	{
		if (DXFormat == DXGI_FORMAT_B8G8R8A8_UNORM)
			PixelFormat = PF_B8G8R8A8;
		else if (DXFormat == DXGI_FORMAT_R16G16B16A16_FLOAT)
			PixelFormat = PF_FloatRGBA;
		else if (DXFormat == DXGI_FORMAT_R32G32B32A32_FLOAT)
			PixelFormat = PF_A32B32G32R32F;

		FString RHIName = GDynamicRHI->GetName();

		if (RHIName == TEXT("D3D11"))
		{
			D3D11Device = (ID3D11Device*)GDynamicRHI->RHIGetNativeDevice();
			D3D11Device->GetImmediateContext(&Context);
		}
		else if (RHIName == TEXT("D3D12"))
		{
			D3D12Device = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
			UINT DeviceFlags11 = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

			verify(D3D11On12CreateDevice(
				D3D12Device,
				DeviceFlags11,
				nullptr,
				0,
				nullptr,
				0,
				0,
				&D3D11Device,
				&Context,
				nullptr
			) == S_OK);

			verify(D3D11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&D3D11on12Device) == S_OK);
		}
		else throw;
	}

	~FSpoutReceiverContext()
	{
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

		if (D3D12Device)
		{
			D3D12Device->Release();
			D3D12Device = nullptr;
		}

		if (Context)
		{
			Context->Release();
			Context = nullptr;
		}

	}
};

//////////////////////////////////////////////////////////////////////////

FSpout2MediaPlayer::FSpout2MediaPlayer(IMediaEventSink& InEventSink)
{
}

FSpout2MediaPlayer::~FSpout2MediaPlayer()
{
	Context.Reset();
}

void FSpout2MediaPlayer::Close()
{
	Context.Reset();
}

IMediaCache& FSpout2MediaPlayer::GetCache()
{
	return *this;
}

IMediaControls& FSpout2MediaPlayer::GetControls()
{
	return *this;
}

FString FSpout2MediaPlayer::GetInfo() const
{
	return FString("");
}

FGuid FSpout2MediaPlayer::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0x6ca11ccb, 0x3f20bc6f, 0xd62160ea, 0xaaddb29b);
	return PlayerPluginGUID;
}

IMediaSamples& FSpout2MediaPlayer::GetSamples()
{
	return *this;
}

FString FSpout2MediaPlayer::GetStats() const
{
	return FString("");
}

IMediaTracks& FSpout2MediaPlayer::GetTracks()
{
	return *this;
}

FString FSpout2MediaPlayer::GetUrl() const
{
	return MediaUrl;
}

IMediaView& FSpout2MediaPlayer::GetView()
{
	return *this;
}

bool FSpout2MediaPlayer::Open(const FString& Url, const IMediaOptions* Options)
{
	MediaUrl = Url;

	FString Scheme;
	FString Location;

	// check scheme
	if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
	{
		return false;
	}

	FString SourceName = Location;

	if (Scheme == FString("spout2mediain"))
	{
		CurrentState = EMediaState::Playing;
		SubscribeName = FName(SourceName);
	}
	
	return true;
}

bool FSpout2MediaPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl,
	const IMediaOptions* Options)
{
	return Open(OriginalUrl, Options);
}

void FSpout2MediaPlayer::TickFetch(FTimespan DeltaTime, FTimespan Timecode)
{
	unsigned int SpoutWidth = 0, SpoutHeight = 0;
	HANDLE SpoutSharehandle = nullptr;
	DXGI_FORMAT SpoutFormat = DXGI_FORMAT_UNKNOWN;

	const bool find_sender = senders.FindSender(
		TCHAR_TO_ANSI(*SubscribeName.ToString()), SpoutWidth, SpoutHeight, SpoutSharehandle, (DWORD&)SpoutFormat);

	EPixelFormat PixelFormat = PF_Unknown;

	if (SpoutFormat == DXGI_FORMAT_B8G8R8A8_UNORM)
		PixelFormat = PF_B8G8R8A8;
	else if (SpoutFormat == DXGI_FORMAT_R16G16B16A16_FLOAT)
		PixelFormat = PF_FloatRGBA;
	else if (SpoutFormat == DXGI_FORMAT_R32G32B32A32_FLOAT)
		PixelFormat = PF_A32B32G32R32F;

	if (!find_sender
	|| PixelFormat == PF_Unknown)
		return;

	{
		if (!Context
			|| Context->Width != SpoutWidth
			|| Context->Height != SpoutHeight
			|| Context->DXFormat != SpoutFormat)
		{
			Context = MakeShared<FSpoutReceiverContext>(SpoutWidth, SpoutHeight, SpoutFormat);
		}
		
		ENQUEUE_RENDER_COMMAND(SpoutRecieverRenderThreadOp)([this, SpoutSharehandle](FRHICommandListImmediate& RHICmdList) {
			check(IsInRenderingThread());

			auto Sample = MakeShared<FSpout2MediaTextureSample, ESPMode::ThreadSafe>();
			FSpout2MediaTextureSample::InitializeArguments Args;
			Args.Width = Context->Width;
			Args.Height = Context->Height;
			Args.DXFormat = Context->DXFormat;
			Args.PixelFormat = Context->PixelFormat;
			Args.SpoutSharehandle = SpoutSharehandle;
			
			Args.Context = Context->Context;
			Args.D3D11Device = Context->D3D11Device;
			Args.D3D12Device = Context->D3D12Device;
			Args.D3D11on12Device = Context->D3D11on12Device;
			
			Sample->Initialize(Args);
			
			if (TextureSample)
				TextureSample.Reset();
			
			TextureSample = Sample;
		});
	}
}

void FSpout2MediaPlayer::TickInput(FTimespan DeltaTime, FTimespan Timecode)
{
	
}

bool FSpout2MediaPlayer::GetPlayerFeatureFlag(EFeatureFlag flag) const
{
	return flag == IMediaPlayer::EFeatureFlag::AlwaysPullNewestVideoFrame;
}

/////

bool FSpout2MediaPlayer::FetchVideo(TRange<FTimespan> TimeRange,
	TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample)
{
	if ((CurrentState != EMediaState::Paused) && (CurrentState != EMediaState::Playing))
	{
		return false; // nothing to play
	}

	if (!TextureSample)
		return false;
	
	OutSample = TextureSample;
	TextureSample.Reset();
	
	return true;
}

void FSpout2MediaPlayer::FlushSamples()
{
	IMediaSamples::FlushSamples();
	TextureSample.Reset();
}

IMediaSamples::EFetchBestSampleResult FSpout2MediaPlayer::FetchBestVideoSampleForTimeRange(
	const TRange<FMediaTimeStamp>& TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample,
	bool bReverse)
{
	return IMediaSamples::FetchBestVideoSampleForTimeRange(TimeRange, OutSample, bReverse);
}

bool FSpout2MediaPlayer::PeekVideoSampleTime(FMediaTimeStamp& TimeStamp)
{
	return false;
}

int32 FSpout2MediaPlayer::GetSelectedTrack(EMediaTrackType TrackType) const
{
	if (TrackType == EMediaTrackType::Video)
		return 0;

	return INDEX_NONE;
}

FText FSpout2MediaPlayer::GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video)
		return FText::FromString("Spout Video");
	
	return FText();
}

int32 FSpout2MediaPlayer::GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video)
		return 0;

	return INDEX_NONE;
}

FString FSpout2MediaPlayer::GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FString();
}

FString FSpout2MediaPlayer::GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video)
		return FString("Spout Video");
	
	return FString();
}

bool FSpout2MediaPlayer::GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex,
	FMediaVideoTrackFormat& OutFormat) const
{
	OutFormat.FrameRate = 60;
	OutFormat.FrameRates = TRange<float>(60, 60);
	OutFormat.Dim = FIntPoint(320, 240);
	
	return true;
}

bool FSpout2MediaPlayer::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	return true;
}

bool FSpout2MediaPlayer::SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex)
{
	return true;
}

bool FSpout2MediaPlayer::SetVideoTrackFrameRate(int32 TrackIndex, int32 FormatIndex, float FrameRate)
{
	return true;
}