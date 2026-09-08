// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MediaOutput.h"
#include "MediaIOCoreDefinitions.h"

#include "Spout2MediaOutput.generated.h"

UENUM(BlueprintType)
enum class ESpout2MediaOutputFormat : uint8
{
	RGB10A2 UMETA(DisplayName = "10-bit RGB / 2-bit Alpha (RGB10A2)"),
	RGBA8 UMETA(DisplayName = "8-bit RGBA (RGBA8)")
};

UCLASS(BlueprintType, meta=(DisplayName="Spout2 Media Output"))
class SPOUT2MEDIA_API USpout2MediaOutput
	: public UMediaOutput
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spout2 Media")
	FString SenderName = FString("UnrealEngile");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spout2 Media")
	FIntPoint OutputSize = FIntPoint(1920, 1080);

	/** Restart capture after changing this. Scene alpha also requires the project's Alpha Output setting. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Spout2 Media")
	ESpout2MediaOutputFormat OutputFormat = ESpout2MediaOutputFormat::RGB10A2;
	
	virtual bool Validate(FString& OutFailureReason) const override;

	virtual FIntPoint GetRequestedSize() const override;
	virtual EPixelFormat GetRequestedPixelFormat() const override;
	virtual EMediaCaptureConversionOperation
	GetConversionOperation(EMediaCaptureSourceType InSourceType) const override;

protected:
	virtual UMediaCapture* CreateMediaCaptureImpl() override;
};
