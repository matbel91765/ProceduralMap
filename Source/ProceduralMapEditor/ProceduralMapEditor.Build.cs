// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

using UnrealBuildTool;

public class ProceduralMapEditor : ModuleRules
{
	public ProceduralMapEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Essential dependencies only
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"ProceduralMap"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"InputCore",
			"PropertyEditor",
			"EditorStyle"
		});

		// Optional advanced dependencies
		// Uncomment these as needed once the project compiles successfully
		/*
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"EditorFramework",
			"EditorWidgets",
			"AssetTools",
			"ContentBrowser",
			"WorkspaceMenuStructure",
			"Projects",
			"ToolMenus",
			"DeveloperSettings",
			"Blutility",
			"UMGEditor",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"RenderCore",
			"RHI",
			"Renderer",
			"ProceduralMeshComponent",
			"Landscape",
			"LandscapeEditor",
			"AdvancedPreviewScene",
			"ApplicationCore",
			"DetailCustomizations",
			"GraphEditor",
			"KismetWidgets",
			"AppFramework",
			"MainFrame"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"EditorSubsystem",
				"ToolWidgets"
			});
		}
		*/
	}
}
