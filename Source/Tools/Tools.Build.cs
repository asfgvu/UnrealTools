// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Tools : ModuleRules
{
	public Tools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
            "MeshReductionInterface",
			"StaticMeshDescription",
			"AssetRegistry",
			"AssetTools",
			"EditorSubsystem",
			"UnrealEd"
        });
    }
}
