// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PhysicsEngine/BodySetup.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "Rendering/PositionVertexBuffer.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMesh.h"
#include <IMeshReductionManagerModule.h>
#include "Modules/ModuleManager.h"
#include "MeshUtilitiesCommon.h"
#include <Developer/MeshReductionInterface/Private/MeshReductionManagerModule.h>
#include <PhysicsEngine/ConvexElem.h>
#include "BodySetupCore.h"
#include "AssetToolsModule.h"
#include "MeshUtilities.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshOperations.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "PhysicsEngine/BodySetup.h"
#include "MeshTools.generated.h"

class UBodySetup;

UCLASS()
class TOOLS_API UMeshTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

    /**
     * Ajoute un LOD sur l'objets selectionner
     * LODsValues : X -> Pourcentage de triangle
     * LODsValues : Y -> Screen Size
     */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Mesh Tools")
    static void GenerateLODsForMesh(UStaticMesh* Mesh, int LODIndex, FVector2D LODsValues);

    /**
     * Clear tous les LODs de l'objet selectionne
     */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Mesh Tools")
    static void ClearLODs(UStaticMesh* Mesh);

    /** Genere une collision simplifiee sur le StaticMesh donne */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Mesh Tools")
    static void GenerateSimpleCollision(UStaticMesh* Mesh);

    /**
    * Replaces materials on a batch of static meshes.
    *
    * If MaterialToReplace is nullptr, replaces **all** materials on the given meshes with NewMaterial.
    * Otherwise, only replaces occurrences of MaterialToReplace.
    *
    * @param Meshes           Array of Static Mesh assets to process.
    * @param MaterialToReplace Material to be replaced. If nullptr, all materials are replaced.
    * @param NewMaterial       Material to assign in place of replaced materials. Must not be nullptr.
    * @return Number of meshes successfully modified and saved.
    */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Materials")
    static int32 ReplaceMaterialBatch(const TArray<UObject*>& Objects, const FString& MaterialToReplaceName, const FString& NewMaterialName);

    /** Returns the names of all UMaterial and UMaterialInstanceConstant assets in the project */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Materials")
    static TArray<FString> GetAllMaterialAssetNames(const FString& NameFilter);

    /** Returns all material asset data (for advanced use) */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Materials")
    static TArray<FAssetData> GetAllMaterialAssets();
};
