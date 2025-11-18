// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ProceduralMap.h"
#include "Modules/ModuleManager.h"

// Optional includes for advanced features
// Uncomment these when needed
// #include "Interfaces/IPluginManager.h"
// #include "Misc/Paths.h"
// #include "ShaderCore.h"

DEFINE_LOG_CATEGORY(LogProceduralMap);

void FProceduralMapModule::StartupModule()
{
	UE_LOG(LogProceduralMap, Log, TEXT("ProceduralMap module started"));

	// Register shader directory (optional, for GPU compute shaders)
	// Uncomment when you need compute shader support
	/*
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/ProceduralMap"), ShaderDirectory);
	UE_LOG(LogProceduralMap, Log, TEXT("Shader directory registered: %s"), *ShaderDirectory);
	*/
}

void FProceduralMapModule::ShutdownModule()
{
	UE_LOG(LogProceduralMap, Log, TEXT("ProceduralMap module shutdown"));
}

IMPLEMENT_MODULE(FProceduralMapModule, ProceduralMap);
