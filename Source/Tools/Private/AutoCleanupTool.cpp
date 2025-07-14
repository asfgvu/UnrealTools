// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoCleanupTool.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "Misc/PackageName.h"
#include "AssetToolsModule.h"

TArray<FAssetData> UAutoCleanupTool::FindUnusedAssets(const TArray<FString>& ExcludedFolders)
{
    TArray<FAssetData> UnusedAssets;

    // Prepare a recursive filter scoped to the /Game directory, which contains project content
    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.PackagePaths.Add(FName("/Game"));

    // Retrieve all assets in /Game using the Asset Registry module
    TArray<FAssetData> AllAssets;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssets(Filter, AllAssets);

    // Iterate through each asset to determine if it is unused and not in excluded folders
    for (const FAssetData& Asset : AllAssets)
    {
        const FString AssetPath = Asset.PackagePath.ToString();

        // Skip assets located in any excluded folder to avoid touching important tools/dev content
        bool bIsExcluded = false;
        for (const FString& Excluded : ExcludedFolders)
        {
            if (AssetPath.StartsWith(Excluded))
            {
                bIsExcluded = true;
                break;
            }
        }
        if (bIsExcluded)
        {
            continue;
        }

        // Confirm asset physically exists on disk and check if it is referenced elsewhere in the project
        const FString ObjectPath = Asset.PackageName.ToString();
        if (UEditorAssetLibrary::DoesAssetExist(ObjectPath) && !IsAssetUsed(Asset))
        {
            // Asset is unreferenced and safe to consider unused; add to output list
            UnusedAssets.Add(Asset);
        }
    }

    return UnusedAssets;
}

bool UAutoCleanupTool::IsAssetUsed(const FAssetData& AssetData)
{
    // Load AssetRegistry for reference checks
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    TArray<FName> Referencers;
    // Get assets that reference this one
    AssetRegistryModule.Get().GetReferencers(AssetData.PackageName, Referencers);

    // If the list is non-empty, the asset is used
    return Referencers.Num() > 0;
}

FString UAutoCleanupTool::MoveUnusedAssetsToFolder(const FString& NewFolderPath, const TArray<FString>& ExcludedFolders)
{
    FString Log;

    // Validate that the destination folder is within the /Game directory
    if (!NewFolderPath.StartsWith("/Game"))
    {
        return TEXT("Invalid folder path. It must start with '/Game'.");
    }

    // Create the destination folder if it doesn't already exist
    if (!UEditorAssetLibrary::DoesDirectoryExist(NewFolderPath))
    {
        if (!UEditorAssetLibrary::MakeDirectory(NewFolderPath))
        {
            return FString::Printf(TEXT("Failed to create folder: %s"), *NewFolderPath);
        }

        Log += FString::Printf(TEXT("Created folder: %s\n"), *NewFolderPath);
    }
    else
    {
        Log += FString::Printf(TEXT("Using existing folder: %s\n"), *NewFolderPath);
    }

    // Find all unused assets, excluding specified folders
    TArray<FAssetData> UnusedAssets = UAutoCleanupTool::FindUnusedAssets(ExcludedFolders);

    int32 SuccessCount = 0;
    int32 FailCount = 0;
    int64 TotalMovedBytes = 0;

    // Iterate through each unused asset and attempt to move it
    for (const FAssetData& Asset : UnusedAssets)
    {
        FString OldPath = Asset.ObjectPath.ToString();
        FString PackageName = Asset.PackageName.ToString();
        FString AssetName = Asset.AssetName.ToString();
        FString NewObjectPath = NewFolderPath / AssetName;

        // Try to get the absolute file path of the .uasset
        FString AssetFilePath;
        if (FPackageName::TryConvertLongPackageNameToFilename(PackageName, AssetFilePath, TEXT(".uasset")))
        {
            int64 FileSize = IFileManager::Get().FileSize(*AssetFilePath);
            if (FileSize > 0)
            {
                TotalMovedBytes += FileSize;
            }
        }

        // Attempt to move the asset
        bool bSuccess = UEditorAssetLibrary::RenameAsset(OldPath, NewObjectPath);

        if (bSuccess)
        {
            ++SuccessCount;
        }
        else
        {
            ++FailCount;
            Log += FString::Printf(TEXT("Failed to move asset: %s\n"), *OldPath);
        }
    }

    // Convert total byte size to megabytes
    float SizeSavedMB = TotalMovedBytes / (1024.0f * 1024.0f);

    // Summary log
    Log += FString::Printf(TEXT("Moved %d unused assets to %s\n"), SuccessCount, *NewFolderPath);
    Log += FString::Printf(TEXT("Estimated disk space involved: %.2f MB\n"), SizeSavedMB);

    if (FailCount > 0)
    {
        Log += FString::Printf(TEXT("Failed to move %d assets.\n"), FailCount);
    }

    return Log;
}

TArray<FString> UAutoCleanupTool::GetUnusedAssetNames(const TArray<FString>& ExcludedFolders)
{
    TArray<FAssetData> UnusedAssets = FindUnusedAssets(ExcludedFolders);

    TArray<FString> UnusedNames;
    for (const FAssetData& Asset : UnusedAssets)
    {
        UnusedNames.Add(Asset.AssetName.ToString());
    }

    return UnusedNames;
}


