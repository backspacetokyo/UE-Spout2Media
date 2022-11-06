// Fill out your copyright notice in the Description page of Project Settings.


#include "Spout2MediaOutputFactory.h"
#include <AssetTypeCategories.h>
#include "Spout2Media/Public/Spout2MediaOutput.h"

#define LOCTEXT_NAMESPACE "Spout2MediaOutputFactory"

USpout2MediaOutputFactory::USpout2MediaOutputFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->bCreateNew = true;
	this->bEditAfterNew = true;

	this->SupportedClass = USpout2MediaOutput::StaticClass();
}

FText USpout2MediaOutputFactory::GetDisplayName() const
{
	return LOCTEXT("USpout2MediaOutputFactoryDisplayName", "Spout2 Media Output");
}

uint32 USpout2MediaOutputFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Media;
}

bool USpout2MediaOutputFactory::ShouldShowInNewMenu() const
{
	return Super::ShouldShowInNewMenu();
}

UObject* USpout2MediaOutputFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<USpout2MediaOutput>(InParent, InClass, InName, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE