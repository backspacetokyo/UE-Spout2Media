// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Spout2MediaOutputFactory.generated.h"

UCLASS(hidecategories=Object)
class SPOUT2MEDIAEDITOR_API USpout2MediaOutputFactory
	: public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;

	virtual bool ShouldShowInNewMenu() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
									  UObject* Context, FFeedbackContext* Warn) override;

};
