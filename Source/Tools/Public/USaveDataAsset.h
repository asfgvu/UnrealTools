// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "USaveDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class TOOLS_API UUSaveDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    // Editable array of strings accessible in Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    TArray<FString> StringArray;
};
