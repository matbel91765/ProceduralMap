// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProceduralMapEditor, Log, All);

/**
 * FProceduralMapEditorModule - Editor module for ProceduralMap
 * Handles editor-only functionality and custom editor modes
 */
class FProceduralMapEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register custom editor modes and tools */
	void RegisterEditorModes();

	/** Unregister custom editor modes and tools */
	void UnregisterEditorModes();

	/** Register custom asset types */
	void RegisterAssetTypes();

	/** Unregister custom asset types */
	void UnregisterAssetTypes();
};
