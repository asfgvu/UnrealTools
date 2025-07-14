// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetRegistry/AssetData.h"
#include "AutoCleanupTool.generated.h"

/**
 * 
 */
UCLASS()
class TOOLS_API UAutoCleanupTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    /**
    * Identifies unreferenced assets inside the /Game directory, excluding specific folders.
    *
    * This version returns the results directly as an array and is designed for use in both
    * Blueprint and C++ asset cleanup workflows. It safely filters out any assets located
    * within user-specified directories (e.g., /Game/Tools, /Game/Developer) and checks
    * whether each asset is actually used in the project.
    *
    * @param ExcludedFolders Array of folder paths to exclude (must begin with /Game).
    * @return Array of FAssetData for all unused, user-facing assets.
    */
    static TArray<FAssetData> FindUnusedAssets(const TArray<FString>& ExcludedFolders);

    /**
     * Checks if a given asset is used (referenced by other assets).
     * @param AssetData - Asset to check.
     * @return true if the asset has references, false otherwise.
     */
    static bool IsAssetUsed(const FAssetData& AssetData);

    /**
    * Moves all unused assets into the specified folder and returns a log summary.
    * Create the destination folder if it does not already exist
    * @param NewFolderPath - Destination path (e.g., "/Game/_UnusedAssets")
    * @return Log message indicating result of the move operation.
    */
    UFUNCTION(BlueprintCallable, Category = "AutoCleanup")
    static FString MoveUnusedAssetsToFolder(const FString& NewFolderPath, const TArray<FString>& ExcludedFolders);

    /**
    * Returns an array of asset names that are currently unused in the project.
    */
    UFUNCTION(BlueprintCallable, Category = "AutoCleanup")
    static TArray<FString> GetUnusedAssetNames(const TArray<FString>& ExcludedFolders);
};
