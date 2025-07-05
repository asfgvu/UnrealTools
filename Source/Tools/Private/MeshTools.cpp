// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshTools.h"
#include "../../../../../../../../Program Files/Epic Games/UE_5.4/Engine/Plugins/Editor/EditorScriptingUtilities/Source/EditorScriptingUtilities/Public/EditorAssetLibrary.h"

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

