// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaTracks.h"


FSpout2MediaTracks::FSpout2MediaTracks()
{
}

FSpout2MediaTracks::~FSpout2MediaTracks()
{
}

bool FSpout2MediaTracks::FetchVideo(TRange<FTimespan> TimeRange,
	TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample)
{
	return IMediaSamples::FetchVideo(TimeRange, OutSample);
}

void FSpout2MediaTracks::FlushSamples()
{
	IMediaSamples::FlushSamples();
}

IMediaSamples::EFetchBestSampleResult FSpout2MediaTracks::FetchBestVideoSampleForTimeRange(
	const TRange<FMediaTimeStamp>& TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample,
	bool bReverse)
{
	return IMediaSamples::FetchBestVideoSampleForTimeRange(TimeRange, OutSample, bReverse);
}

bool FSpout2MediaTracks::PeekVideoSampleTime(FMediaTimeStamp& TimeStamp)
{
	return false;
}

int32 FSpout2MediaTracks::GetSelectedTrack(EMediaTrackType TrackType) const
{
	if (TrackType == EMediaTrackType::Video)
		return 0;

	return INDEX_NONE;
}

FText FSpout2MediaTracks::GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video)
		return FText::FromString("Spout Video");
	
	return FText();
}

int32 FSpout2MediaTracks::GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video)
		return 0;

	return INDEX_NONE;
}

FString FSpout2MediaTracks::GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FString();
}

FString FSpout2MediaTracks::GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video)
		return FString("Spout Video");
	
	return FString();
}

bool FSpout2MediaTracks::GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex,
	FMediaVideoTrackFormat& OutFormat) const
{
	OutFormat.FrameRate = 60;
	OutFormat.FrameRates = TRange<float>(60, 60);
	OutFormat.Dim = FIntPoint(320, 240);
	
	return true;
}

bool FSpout2MediaTracks::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	return true;
}

bool FSpout2MediaTracks::SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex)
{
	return true;
}

bool FSpout2MediaTracks::SetVideoTrackFrameRate(int32 TrackIndex, int32 FormatIndex, float FrameRate)
{
	return true;
}


