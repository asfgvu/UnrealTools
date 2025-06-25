// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshTools.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMesh.h"
#include <IMeshReductionManagerModule.h>
#include "Modules/ModuleManager.h"
#include "MeshUtilitiesCommon.h"
#include <Developer/MeshReductionInterface/Private/MeshReductionManagerModule.h>

void UMeshTools::GenerateLODsForMesh(UStaticMesh* Mesh, float LODScreenSize)
{
    if (!Mesh || LODScreenSize <= 0.0f || LODScreenSize >= 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid mesh or screen size"));
        return;
    }

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

    // Nettoyer les LODs existants (ne garder que LOD0)
    int32 ExistingLODCount = Mesh->GetNumSourceModels();
    if (ExistingLODCount > 1)
    {
        Mesh->SetNumSourceModels(1);
    }

    // Désactiver le screen size auto
    Mesh->bAutoComputeLODScreenSize = false;

    // Liste des pourcentages de réduction
    TArray<float> ReductionPercents = { 0.75f, 0.5f, 0.25f };

    for (float Reduction : ReductionPercents)
    {
        int32 NewLODIndex = Mesh->GetNumSourceModels();
        FStaticMeshSourceModel& LODModel = Mesh->AddSourceModel();

        // Paramètres de réduction
        LODModel.ReductionSettings.PercentTriangles = Reduction;
        LODModel.ReductionSettings.bRecalculateNormals = false;
        LODModel.ReductionSettings.bRecalculateNormals = false;

        // Paramètres de build
        LODModel.BuildSettings.bRecomputeNormals = false;
        LODModel.BuildSettings.bRecomputeTangents = false;

        // Paramètre d’affichage
        LODModel.ScreenSize.Default = LODScreenSize;

        UE_LOG(LogTemp, Log, TEXT("LOD %d ajouté avec %.0f%% triangles et ScreenSize %.2f"), NewLODIndex, Reduction * 100.f, LODScreenSize);
    }

    // Reconstruction du mesh
    Mesh->Build(false);
    Mesh->MarkPackageDirty();
    Mesh->Modify();

    // Réassigner les ScreenSize manuellement pour s'assurer qu'ils soient éditables
    for (int32 i = 1; i < Mesh->GetNumSourceModels(); ++i)
    {
        if (ReductionPercents.IsValidIndex(i)) {
            Mesh->GetSourceModel(i).ScreenSize.Default = ReductionPercents[i];
        }
    }

    Mesh->PostEditChange();

    UE_LOG(LogTemp, Log, TEXT("LODs générés avec succès pour le mesh : %s"), *Mesh->GetName());
}
