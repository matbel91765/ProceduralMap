// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProceduralMapEditorTarget : TargetRules
{
	public ProceduralMapEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("ProceduralMap");
		ExtraModuleNames.Add("ProceduralMapEditor");
	}
}
