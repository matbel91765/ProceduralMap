// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

using UnrealBuildTool;

public class ProceduralMapEditor : ModuleRules
{
	public ProceduralMapEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"ProceduralMap",
			"UnrealEd",
			"EditorFramework",
			"Slate",
			"SlateCore",
			"PropertyEditor",
			"EditorStyle",
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

		// Editor-only module
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"EditorSubsystem",
				"ToolWidgets"
			});
		}
	}
}
