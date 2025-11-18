// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ProceduralMapEditor.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogProceduralMapEditor);

void FProceduralMapEditorModule::StartupModule()
{
	UE_LOG(LogProceduralMapEditor, Log, TEXT("ProceduralMapEditor module starting up"));

	// Register editor modes and asset types
	RegisterEditorModes();
	RegisterAssetTypes();

	UE_LOG(LogProceduralMapEditor, Log, TEXT("ProceduralMapEditor module started successfully"));
}

void FProceduralMapEditorModule::ShutdownModule()
{
	UE_LOG(LogProceduralMapEditor, Log, TEXT("ProceduralMapEditor module shutting down"));

	// Unregister editor modes and asset types
	UnregisterEditorModes();
	UnregisterAssetTypes();

	UE_LOG(LogProceduralMapEditor, Log, TEXT("ProceduralMapEditor module shut down"));
}

void FProceduralMapEditorModule::RegisterEditorModes()
{
	// TODO: Register custom editor modes when they are implemented
	// Example:
	// FEditorModeRegistry::Get().RegisterMode<FProceduralMapEditorMode>(
	//     FProceduralMapEditorMode::EM_ProceduralMapEditorModeId,
	//     LOCTEXT("ProceduralMapEditorModeName", "Procedural Map"),
	//     FSlateIcon(),
	//     true);
}

void FProceduralMapEditorModule::UnregisterEditorModes()
{
	// TODO: Unregister custom editor modes
	// Example:
	// FEditorModeRegistry::Get().UnregisterMode(FProceduralMapEditorMode::EM_ProceduralMapEditorModeId);
}

void FProceduralMapEditorModule::RegisterAssetTypes()
{
	// TODO: Register custom asset types and actions
	// Example:
	// IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	// AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_TerrainPreset));
}

void FProceduralMapEditorModule::UnregisterAssetTypes()
{
	// TODO: Unregister custom asset types
}

IMPLEMENT_MODULE(FProceduralMapEditorModule, ProceduralMapEditor);
