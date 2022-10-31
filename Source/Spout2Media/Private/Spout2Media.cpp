// Copyright Epic Games, Inc. All Rights Reserved.

#include "Spout2Media.h"
#include "IMediaModule.h"

#include "Spout2MediaPlayer.h"

#define LOCTEXT_NAMESPACE "FSpout2MediaModule"

void FSpout2MediaModule::StartupModule()
{
	SupportedPlatforms.Add(TEXT("Windows"));
	SupportedUriSchemes.Add(TEXT("spout2mediain"));

	auto MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");
	if (MediaModule != nullptr)
	{
		MediaModule->RegisterPlayerFactory(*this);
	}
}

void FSpout2MediaModule::ShutdownModule()
{
}

bool FSpout2MediaModule::CanPlayUrl(const FString& Url, const IMediaOptions*, TArray<FText>*,
	TArray<FText>* OutErrors) const
{
	FString Scheme;
	FString Location;

	// check scheme
	if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
	{
		if (OutErrors != nullptr)
		{
			OutErrors->Add(LOCTEXT("NoSchemeFound", "No URI scheme found"));
		}

		return false;
	}

	if (!SupportedUriSchemes.Contains(Scheme))
	{
		if (OutErrors != nullptr)
		{
			OutErrors->Add(FText::Format(
				LOCTEXT("SchemeNotSupported", "The URI scheme '{0}' is not supported"), FText::FromString(Scheme)));
		}

		return false;
	}

	return true;
}

TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> FSpout2MediaModule::CreatePlayer(IMediaEventSink& EventSink)
{
	return MakeShared<FSpout2MediaPlayer, ESPMode::ThreadSafe>(EventSink);
}

FText FSpout2MediaModule::GetDisplayName() const
{
	return LOCTEXT("MediaPlayerDisplayName", "Spout2 Media Interface");
}

FName FSpout2MediaModule::GetPlayerName() const
{
	static FName PlayerName(TEXT("Spout2Media"));
	return PlayerName;
}

FGuid FSpout2MediaModule::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0x6ca11ccb, 0x3f20bc6f, 0xd62160ea, 0xaaddb29b);
	return PlayerPluginGUID;
}

const TArray<FString>& FSpout2MediaModule::GetSupportedPlatforms() const
{
	return SupportedPlatforms;
}

bool FSpout2MediaModule::SupportsFeature(EMediaFeature Feature) const
{
	return (Feature == EMediaFeature::VideoSamples)
		|| (Feature == EMediaFeature::VideoTracks);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSpout2MediaModule, Spout2Media)