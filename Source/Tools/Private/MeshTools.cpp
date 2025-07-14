// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshTools.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"  // Pour sauvegarder les assets
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "EngineUtils.h"

void UMeshTools::GenerateLODsForMesh(UStaticMesh* Mesh, int LODIndex, FVector2D LODsValues)
{

    // Chargement du système de réduction de mesh
    IMeshReduction* MeshReduction = nullptr;

    if (FModuleManager::Get().IsModuleLoaded("MeshReductionInterface"))
    {
        MeshReduction = FModuleManager::GetModuleChecked<FMeshReductionManagerModule>("MeshReductionInterface").GetStaticMeshReductionInterface();
    }
    else
    {
        MeshReduction = FModuleManager::LoadModuleChecked<FMeshReductionManagerModule>("MeshReductionInterface").GetStaticMeshReductionInterface();
    }

    // Désactiver le screen size auto
    Mesh->bAutoComputeLODScreenSize = false;

    int32 NewLODIndex = Mesh->GetNumSourceModels();
    FStaticMeshSourceModel& LODModel = Mesh->AddSourceModel();

    // Paramètres de réduction
    LODModel.ReductionSettings.PercentTriangles = LODsValues.X;
    LODModel.ReductionSettings.bRecalculateNormals = false;
    LODModel.ReductionSettings.bRecalculateNormals = false;

    // Paramètres de build
    LODModel.BuildSettings.bRecomputeNormals = false;
    LODModel.BuildSettings.bRecomputeTangents = false;

    // Paramètre d’affichage
    LODModel.ScreenSize.Default = LODsValues.Y;

    // Reconstruction du mesh
    Mesh->Build(false);
    Mesh->MarkPackageDirty();
    Mesh->Modify();

    // Réassigner les ScreenSize manuellement pour s'assurer qu'ils soient éditables
    Mesh->GetSourceModel(LODIndex).ScreenSize.Default = LODsValues.Y;

    Mesh->PostEditChange();

    UE_LOG(LogTemp, Log, TEXT("LODs générés avec succès pour le mesh : %s"), *Mesh->GetName());
}

void UMeshTools::ClearLODs(UStaticMesh* Mesh)
{
    // Nettoyer les LODs existants (ne garder que LOD0)
    int32 ExistingLODCount = Mesh->GetNumSourceModels();
    if (ExistingLODCount > 1)
    {
        Mesh->SetNumSourceModels(1);
    }

    // Reconstruit et sauvegarde les changements
    Mesh->Build(false);
    Mesh->MarkPackageDirty();
    Mesh->Modify();
    Mesh->PostEditChange();
}

void UMeshTools::GenerateSimpleCollision(UStaticMesh* Mesh)
{
    if (!Mesh) return;

    // S'assure que le BodySetup existe
    if (!Mesh->GetBodySetup())
    {
        Mesh->CreateBodySetup();
    }

    UBodySetup* BodySetup = Mesh->GetBodySetup();
    BodySetup->RemoveSimpleCollision(); // Nettoie les anciennes collisions

    // Crée une collision convexe à partir du mesh LOD 0
    TArray<FKAggregateGeom> AggregateGeoms;
    FKAggregateGeom& AggGeom = BodySetup->AggGeom;

    // Génère une forme convexe (enveloppe simple)
    AggGeom.ConvexElems.Add(FKConvexElem());

    FKConvexElem& Convex = AggGeom.ConvexElems.Last();
    FStaticMeshRenderData* RenderData = Mesh->GetRenderData();
    if (!RenderData || RenderData->LODResources.Num() == 0) return;

    const FStaticMeshLODResources& LOD = RenderData->LODResources[Mesh->GetNumSourceModels() - 1];
    const FPositionVertexBuffer& VertexBuffer = LOD.VertexBuffers.PositionVertexBuffer;

    TArray<FVector> ConvexVerts;

    int32 VertexCount = VertexBuffer.GetNumVertices();
    for (int32 i = 0; i < VertexCount; ++i)
    {
        FVector Position = FVector(VertexBuffer.VertexPosition(i));
        ConvexVerts.Add(Position);
    }

    Convex.VertexData = ConvexVerts;
    Convex.UpdateElemBox();

    // Marque le mesh comme modifié
    BodySetup->InvalidatePhysicsData();
    BodySetup->CreatePhysicsMeshes();
    Mesh->MarkPackageDirty();
    Mesh->Build(false);
}

int32 UMeshTools::ReplaceMaterialBatch(const TArray<UObject*>& Objects, const FString& MaterialToReplaceName, const FString& NewMaterialName)
{
    // Load the Asset Registry to search for materials
    FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    // Construct the full object path for the new material
    FString NewMaterialObjectPath = FString::Printf(TEXT("/Game/%s.%s"), *NewMaterialName, *NewMaterialName);
    FAssetData NewMaterialData = AssetRegistry.Get().GetAssetByObjectPath(FName(*NewMaterialObjectPath));
    UMaterialInterface* NewMaterial = Cast<UMaterialInterface>(NewMaterialData.GetAsset());

    if (!NewMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("ReplaceMaterialBatch: Failed to find NewMaterial '%s'. Aborting."), *NewMaterialObjectPath);
        return 0;
    }

    // Optionally resolve MaterialToReplace (can be empty)
    UMaterialInterface* MaterialToReplace = nullptr;
    if (!MaterialToReplaceName.IsEmpty())
    {
        FString OldMaterialObjectPath = FString::Printf(TEXT("/Game/%s.%s"), *MaterialToReplaceName, *MaterialToReplaceName);
        FAssetData OldMaterialData = AssetRegistry.Get().GetAssetByObjectPath(FName(*OldMaterialObjectPath));
        MaterialToReplace = Cast<UMaterialInterface>(OldMaterialData.GetAsset());

        if (!MaterialToReplace)
        {
            UE_LOG(LogTemp, Warning, TEXT("ReplaceMaterialBatch: Could not find MaterialToReplace '%s'. Continuing with global replacement."), *OldMaterialObjectPath);
        }
    }

    int32 ModifiedCount = 0;

    for (UObject* Obj : Objects)
    {
        UStaticMesh* Mesh = Cast<UStaticMesh>(Obj);
        if (!Mesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("ReplaceMaterialBatch: Skipping non-UStaticMesh object."));
            continue;
        }

        bool bModified = false;
        TArray<FStaticMaterial>& StaticMaterials = Mesh->GetStaticMaterials();

        // Replace materials on the static mesh asset
        for (FStaticMaterial& StaticMat : StaticMaterials)
        {
            if (!MaterialToReplace || StaticMat.MaterialInterface == MaterialToReplace)
            {
                if (StaticMat.MaterialInterface != NewMaterial)
                {
                    StaticMat.MaterialInterface = NewMaterial;
                    bModified = true;
                }
            }
        }

        if (bModified)
        {
            // Mark asset dirty and save
            Mesh->MarkPackageDirty();

            const FString AssetPath = Mesh->GetPathName();
            const bool bSaved = UEditorAssetLibrary::SaveAsset(AssetPath);

            if (!bSaved)
            {
                UE_LOG(LogTemp, Warning, TEXT("ReplaceMaterialBatch: Failed to save modified mesh: %s"), *AssetPath);
            }

            // Update materials on all actors in the scene using this mesh
            UWorld* World = GEditor->GetEditorWorldContext().World();
            if (World)
            {
                for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
                {
                    AStaticMeshActor* Actor = *It;
                    if (!Actor || !Actor->GetStaticMeshComponent())
                        continue;

                    if (Actor->GetStaticMeshComponent()->GetStaticMesh() == Mesh)
                    {
                        int32 MatCount = Actor->GetStaticMeshComponent()->GetNumMaterials();
                        for (int32 i = 0; i < MatCount; ++i)
                        {
                            UMaterialInterface* CurrentMat = Actor->GetStaticMeshComponent()->GetMaterial(i);
                            if (!MaterialToReplace || CurrentMat == MaterialToReplace)
                            {
                                if (CurrentMat != NewMaterial)
                                {
                                    Actor->GetStaticMeshComponent()->SetMaterial(i, NewMaterial);
                                }
                            }
                        }

                        Actor->Modify(); // Allow undo
                        // Force the render state to update so material changes appear immediately
                        Actor->GetStaticMeshComponent()->MarkRenderStateDirty();
                    }
                }
            }

            ModifiedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ReplaceMaterialBatch: Modified %d meshes."), ModifiedCount);
    return ModifiedCount;
}

TArray<FAssetData> UMeshTools::GetAllMaterialAssets()
{
    TArray<FAssetData> OutAssets;

    // Load the Asset Registry module (used to query assets in the editor)
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    // Define the filter to search for materials only within the /Game directory
    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.PackagePaths.Add("/Game");

    // Use ClassPaths instead of deprecated ClassNames
    // "/Script/Engine.Material" = UMaterial
    // "/Script/Engine.MaterialInstanceConstant" = UMaterialInstanceConstant
    Filter.ClassPaths.Add(FTopLevelAssetPath("/Script/Engine", "Material"));
    Filter.ClassPaths.Add(FTopLevelAssetPath("/Script/Engine", "MaterialInstanceConstant"));

    // Retrieve assets matching the filter
    AssetRegistryModule.Get().GetAssets(Filter, OutAssets);

    return OutAssets;
}

TArray<FString> UMeshTools::GetAllMaterialAssetNames(const FString& NameFilter)
{
    TArray<FString> MaterialNames;

    // Get all material and material instance assets
    TArray<FAssetData> MaterialAssets = GetAllMaterialAssets();

    for (const FAssetData& Asset : MaterialAssets)
    {
        const FString AssetName = Asset.AssetName.ToString();

        // Apply filter if specified
        if (NameFilter.IsEmpty() || AssetName.Contains(NameFilter, ESearchCase::IgnoreCase))
        {
            MaterialNames.Add(AssetName);
        }
    }

    return MaterialNames;
}
