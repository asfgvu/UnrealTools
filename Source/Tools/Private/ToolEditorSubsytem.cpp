// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolEditorSubsytem.h"
#include "Editor.h"
#include "EditorFramework/AssetImportData.h"
#include "Factories/TextureFactory.h"
#include "Engine/Texture2D.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "EditorAssetLibrary.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"
#include "Logging/LogMacros.h"

#define TEXTURE_ROOT_FOLDER TEXT("/Game/Textures")

void UToolEditorSubsytem::Initialize(FSubsystemCollectionBase& Collection)
{
    FEditorDelegates::OnAssetPostImport.AddUObject(this, &UToolEditorSubsytem::OnAssetPostImport);
}

void UToolEditorSubsytem::Deinitialize()
{
    FEditorDelegates::OnAssetPostImport.RemoveAll(this);
}

void UToolEditorSubsytem::OnAssetPostImport(UFactory* InFactory, UObject* InObject)
{
    // Only process Texture2D assets
    if (UTexture2D* ImportedTexture = Cast<UTexture2D>(InObject))
    {
        OrganizeImportedTexture(ImportedTexture);
    }
}

void UToolEditorSubsytem::OrganizeImportedTexture(UTexture2D* ImportedTexture)
{
    if (!ImportedTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("OrganizeImportedTexture: Invalid or null texture."));
        return;
    }

    // Get the name of the texture asset (e.g., "T_Wood_Albedo")
    const FString TextureName = ImportedTexture->GetName();

    // Remove known suffixes (_Albedo, _Roughness, etc.) if present
    FString BaseName;
    if (TextureName.Split(TEXT("_"), nullptr, &BaseName))
    {
        BaseName = TextureName.Left(TextureName.Find(TEXT("_")));
    }
    else
    {
        BaseName = TextureName; // fallback if no underscore
    }

    // Construct the full path of the destination folder
    const FString TargetFolderPath = FString::Printf(TEXT("%s/%s"), TEXTURE_ROOT_FOLDER, *BaseName);

    // Create the folder if it doesn't already exist
    if (!UEditorAssetLibrary::DoesDirectoryExist(TargetFolderPath))
    {
        const bool bCreated = UEditorAssetLibrary::MakeDirectory(TargetFolderPath);
        if (bCreated)
        {
            UE_LOG(LogTemp, Log, TEXT("Created folder: %s"), *TargetFolderPath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create folder: %s"), *TargetFolderPath);
            return;
        }
    }

    // Build full old and new object paths
    const FString OldPath = ImportedTexture->GetPathName();
    const FString NewObjectPath = TargetFolderPath + TEXT("/") + TextureName;

    // Move the asset to the new folder
    if (UEditorAssetLibrary::RenameAsset(OldPath, NewObjectPath))
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully moved texture '%s' to '%s'"), *TextureName, *TargetFolderPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to move texture '%s' to '%s'"), *TextureName, *TargetFolderPath);
    }
}