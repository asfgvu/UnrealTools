// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshTools.generated.h"

/**
 * 
 */
UCLASS()
class TOOLS_API UMeshTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

    /**
     * G�n�re 3 LODs simplifi�s (0.75, 0.5, 0.25) sur le mesh donn�.
     * Tous les LODs utiliseront le m�me ScreenSize donn�.
     */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Mesh Tools")
    static void GenerateLODsForMesh(UStaticMesh* Mesh, float LODScreenSize = 0.5f);
	
};
