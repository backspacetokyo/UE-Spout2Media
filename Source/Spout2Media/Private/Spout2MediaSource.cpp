// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaSource.h"


USpout2MediaSource::USpout2MediaSource(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FString USpout2MediaSource::GetUrl() const
{
	return FString("spout2mediain://") + SourceName;
}
