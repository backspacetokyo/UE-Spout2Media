// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaOutput.h"
#include "Spout2MediaCapture.h"

USpout2MediaOutput::USpout2MediaOutput(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool USpout2MediaOutput::Validate(FString& OutFailureReason) const
{
	return true;
}

FIntPoint USpout2MediaOutput::GetRequestedSize() const
{
	return OutputSize;
}

EPixelFormat USpout2MediaOutput::GetRequestedPixelFormat() const
{
	EPixelFormat Result = PF_A2B10G10R10;
	// Result = PF_FloatRGBA;
	return Result;
}

EMediaCaptureConversionOperation USpout2MediaOutput::GetConversionOperation(EMediaCaptureSourceType InSourceType) const
{
	EMediaCaptureConversionOperation Result = EMediaCaptureConversionOperation::NONE;
	return Result;
}

UMediaCapture* USpout2MediaOutput::CreateMediaCaptureImpl()
{
	USpout2MediaCapture* Result = NewObject<USpout2MediaCapture>();
	if (Result)
	{
		Result->SetMediaOutput(this);
	}
	return Result;
}
