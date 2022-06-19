// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class DataWise : ModuleRules
	{
        public DataWise(ReadOnlyTargetRules Target) : base(Target)
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
                    "SlateCore",
                    "Slate"
                }
			);

            string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..\\..\\ThirdParty"));

            string LibPath = "";
            PrivateIncludePaths.Add(Path.Combine(ThirdPartyPath, "Includes"));

            if (Target.Platform == UnrealTargetPlatform.Win64)
		    {
		        LibPath = Path.Combine(ThirdPartyPath, "Bin\\windows_x64");

                RuntimeDependencies.Add(Path.Combine(LibPath, "core.dll"));
		        PublicAdditionalLibraries.Add(Path.Combine(LibPath, "core.lib"));

		        PublicDelayLoadDLLs.Add("core.dll");
		    }

		    PublicLibraryPaths.Add(LibPath);
		    
		}
	}
}
