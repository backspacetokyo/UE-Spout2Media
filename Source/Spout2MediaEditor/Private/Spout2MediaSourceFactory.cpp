// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaSourceFactory.h"
#include <AssetTypeCategories.h>

#include "Spout2Media/Public/Spout2MediaSource.h"

#define LOCTEXT_NAMESPACE "Spout2MediaSourceFactory"

USpout2MediaSourceFactory::USpout2MediaSourceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->bCreateNew = true;
	this->bEditAfterNew = true;

	this->SupportedClass = USpout2MediaSource::StaticClass();
}

FText USpout2MediaSourceFactory::GetDisplayName() const
{
	return LOCTEXT("Spout2MediaSourceFactoryDisplayName", "Spout2 Source");
}

uint32 USpout2MediaSourceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Media;
}

bool USpout2MediaSourceFactory::ShouldShowInNewMenu() const
{
	return Super::ShouldShowInNewMenu();
}

UObject* USpout2MediaSourceFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<USpout2MediaSource>(InParent, InClass, InName, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE