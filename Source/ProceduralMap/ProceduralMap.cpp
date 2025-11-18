// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ProceduralMap.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

DEFINE_LOG_CATEGORY(LogProceduralMap);

void FProceduralMapModule::StartupModule()
{
	// Register shader directory
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/ProceduralMap"), ShaderDirectory);

	UE_LOG(LogProceduralMap, Log, TEXT("ProceduralMap module started - Shader directory registered: %s"), *ShaderDirectory);
}

void FProceduralMapModule::ShutdownModule()
{
	UE_LOG(LogProceduralMap, Log, TEXT("ProceduralMap module shutdown"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FProceduralMapModule, ProceduralMap, "ProceduralMap");
