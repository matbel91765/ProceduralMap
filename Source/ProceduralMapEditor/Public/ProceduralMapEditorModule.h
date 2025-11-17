// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Toolkits/AssetEditorToolkit.h"

class FProceduralMapEditorMode;
class IAssetTypeActions;

/**
 * ProceduralMap Editor Module
 * Provides professional editor tools with real-time preview for procedural terrain generation
 */
class FProceduralMapEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Get the singleton instance of this module
	 */
	static FProceduralMapEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FProceduralMapEditorModule>("ProceduralMapEditor");
	}

	/**
	 * Check if the module is loaded
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ProceduralMapEditor");
	}

private:
	/** Register custom asset types */
	void RegisterAssetTypeActions();

	/** Unregister custom asset types */
	void UnregisterAssetTypeActions();

	/** Register detail customizations */
	void RegisterDetailCustomizations();

	/** Unregister detail customizations */
	void UnregisterDetailCustomizations();

	/** Register editor modes */
	void RegisterEditorModes();

	/** Unregister editor modes */
	void UnregisterEditorModes();

	/** Register menu extensions */
	void RegisterMenuExtensions();

	/** Unregister menu extensions */
	void UnregisterMenuExtensions();

	/** Registered asset type actions */
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** Editor mode ID */
	FEditorModeID ProceduralMapEditorModeID;
};
