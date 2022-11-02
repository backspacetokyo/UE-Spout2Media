// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MediaIOCorePlayerBase.h"

class SPOUT2MEDIA_API FSpout2MediaPlayer
	: public IMediaPlayer
	, protected IMediaCache
	, protected IMediaView
	, public IMediaControls
	, public IMediaSamples
	, public IMediaTracks
{
	struct FSpoutReceiverContext;
	TSharedPtr<FSpoutReceiverContext> Context;

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

	virtual bool GetPlayerFeatureFlag(EFeatureFlag flag) const override;
	
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

public:
	
	//~ IMediaSamples interface

	virtual bool FetchAudio(TRange<FTimespan> TimeRange, TSharedPtr<IMediaAudioSample, ESPMode::ThreadSafe>& OutSample) override { return false; }
	virtual bool FetchCaption(TRange<FTimespan> TimeRange, TSharedPtr<IMediaOverlaySample, ESPMode::ThreadSafe>& OutSample) override { return false; }
	virtual bool FetchMetadata(TRange<FTimespan> TimeRange, TSharedPtr<IMediaBinarySample, ESPMode::ThreadSafe>& OutSample) override { return false; }
	virtual bool FetchVideo(TRange<FTimespan> TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample) override;
	virtual void FlushSamples() override;
	virtual EFetchBestSampleResult FetchBestVideoSampleForTimeRange(const TRange<FMediaTimeStamp> & TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample, bool bReverse) override;
	virtual bool PeekVideoSampleTime(FMediaTimeStamp & TimeStamp) override;

public:
	
	//~ IMediaTracks interface

	virtual bool GetAudioTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaAudioTrackFormat& OutFormat) const override { return false; }
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override { return 1; }
	virtual int32 GetNumTrackFormats(EMediaTrackType TrackType, int32 TrackIndex) const override { return 1; }
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual int32 GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual bool GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaVideoTrackFormat& OutFormat) const override;
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;
	virtual bool SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex) override;
	virtual bool SetVideoTrackFrameRate(int32 TrackIndex, int32 FormatIndex, float FrameRate) override;
	
private:

	EMediaState CurrentState = EMediaState::Closed;
	FString MediaUrl;

	FName SubscribeName = "";
	bool bSRGB = true;
	
	TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe> TextureSample;
};
