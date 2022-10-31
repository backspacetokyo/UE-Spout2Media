// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MediaIOCorePlayerBase.h"

#include "Spout2MediaTracks.h"

class SPOUT2MEDIA_API FSpout2MediaPlayer
	: public IMediaPlayer
	, protected IMediaCache
	, protected IMediaView
	, public IMediaControls
{
public:
	FSpout2MediaPlayer(IMediaEventSink& InEventSink);
	~FSpout2MediaPlayer();


public:
	
	//~ IMediaPlayer interface

	virtual void Close() override;
	virtual IMediaCache& GetCache() override;
	virtual IMediaControls& GetControls() override;
	virtual FString GetInfo() const override;
	virtual FGuid GetPlayerPluginGUID() const override;
	virtual IMediaSamples& GetSamples() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual IMediaView& GetView() override;
	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* Options) override;
	virtual void TickAudio() override {}
	virtual void TickFetch(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;

protected:
	
	//~ IMediaControls interface

	virtual bool CanControl(EMediaControl Control) const override { return  false; }
	virtual FTimespan GetDuration() const override { return FTimespan(); }
	virtual float GetRate() const override { return 1; }
	virtual EMediaState GetState() const override { return CurrentState; }
	virtual EMediaStatus GetStatus() const override { return EMediaStatus::None; }
	virtual TRangeSet<float> GetSupportedRates(EMediaRateThinning Thinning) const override
		{ return TRangeSet<float>(); }
	virtual FTimespan GetTime() const override { return 0; }
	virtual bool IsLooping() const override { return false; }
	virtual bool Seek(const FTimespan& Time) override { return false; }
	virtual bool SetLooping(bool Looping) override { return false; }
	virtual bool SetRate(float Rate) override { return false; }

private:

	EMediaState CurrentState = EMediaState::Closed;
	FString MediaUrl;

	TSharedPtr<FSpout2MediaTracks, ESPMode::ThreadSafe> Tracks;
};
