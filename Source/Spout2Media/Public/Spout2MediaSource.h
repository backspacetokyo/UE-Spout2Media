// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMediaSource.h"

#include "Spout2MediaSource.generated.h"

UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Spout2 Media Source"), HideCategories=("Platforms"))
class SPOUT2MEDIA_API USpout2MediaSource
	: public UBaseMediaSource
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spout2 Media")
	FString SourceName = FString("Test Pattern");

	virtual bool Validate() const override { return true; }
	virtual FString GetUrl() const override;
};
