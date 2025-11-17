// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProceduralMapTarget : TargetRules
{
	public ProceduralMapTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("ProceduralMap");
	}
}
