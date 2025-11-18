// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

using UnrealBuildTool;

public class ProceduralMap : ModuleRules
{
	public ProceduralMap(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"ProceduralMeshComponent",
			"RenderCore",
			"Renderer",
			"RHI",
			"Projects",
			"Landscape",
			"Foliage",
			"Niagara",
			"Water",
			"ImageWrapper",
			"ImageWriteQueue"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});

		// PCG Framework (UE5) - Optional
		// Uncomment if you have UE 5.4+ with PCG enabled
		// if (Target.Version.MajorVersion >= 5)
		// {
		// 	PublicDependencyModuleNames.Add("PCG");
		// }

		// Enable multi-threading
		bEnableExceptions = false;
		bUseRTTI = false;

		// Optimizations
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
