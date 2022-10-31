// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaPlayer.h"


FSpout2MediaPlayer::FSpout2MediaPlayer(IMediaEventSink& InEventSink)
	: Tracks(MakeShared<FSpout2MediaTracks, ESPMode::ThreadSafe>())
{
}

FSpout2MediaPlayer::~FSpout2MediaPlayer()
{
}

void FSpout2MediaPlayer::Close()
{
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
	return FString("info...");
}

FGuid FSpout2MediaPlayer::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0x6ca11ccb, 0x3f20bc6f, 0xd62160ea, 0xaaddb29b);
	return PlayerPluginGUID;
}

IMediaSamples& FSpout2MediaPlayer::GetSamples()
{
	return *Tracks;
}

FString FSpout2MediaPlayer::GetStats() const
{
	return FString("status...");
}

IMediaTracks& FSpout2MediaPlayer::GetTracks()
{
	return *Tracks;
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
	// FMediaIOCorePlayerBase::TickFetch(DeltaTime, Timecode);
}

void FSpout2MediaPlayer::TickInput(FTimespan DeltaTime, FTimespan Timecode)
{
	
}
