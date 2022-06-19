// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class DataWiseEditor : ModuleRules
	{
        public DataWiseEditor(ReadOnlyTargetRules Target) : base(Target)
		{
		    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
                    "Projects",
                    "UnrealEd",
                    "AssetTools",
                    "AssetRegistry",
                    "BlueprintGraph",
                    "KismetWidgets",
                    "KismetCompiler",
                    "Kismet",
                    "GraphEditor",
                    "DataWise"
                }
			);

		    string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..\\..\\ThirdParty"));
		    PrivateIncludePaths.Add(Path.Combine(ThirdPartyPath, "Includes"));
        }
	}
}
