// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Factories/Factory.h"
#include "ToolEditorSubsytem.generated.h"

/**
 * Editor subsystem to handle automatic texture import organization.
 * Automatically places imported textures into a named folder under /Game/Textures/.
 */
UCLASS()
class TOOLS_API UToolEditorSubsytem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
    // Subsystem lifecycle overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    // Callback for post asset import events
    void OnAssetPostImport(UFactory* InFactory, UObject* InObject);

    // Internal utility to move texture asset to its dedicated folder
    void OrganizeImportedTexture(UTexture2D* ImportedTexture);
	
};
