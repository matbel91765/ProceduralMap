// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ProceduralMapEditorModule.h"
#include "ProceduralMapEditorMode.h"
#include "ProceduralMapAssetActions.h"
#include "TerrainPresetFactory.h"
#include "BiomeSettingsCustomization.h"
#include "NoiseParametersCustomization.h"
#include "EditorModeRegistry.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "ToolMenus.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "FProceduralMapEditorModule"

void FProceduralMapEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Module Starting..."));

	// Register asset type actions
	RegisterAssetTypeActions();

	// Register detail customizations
	RegisterDetailCustomizations();

	// Register editor modes
	RegisterEditorModes();

	// Register menu extensions
	RegisterMenuExtensions();

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Module Started Successfully"));
}

void FProceduralMapEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Module Shutting Down..."));

	// Unregister in reverse order
	UnregisterMenuExtensions();
	UnregisterEditorModes();
	UnregisterDetailCustomizations();
	UnregisterAssetTypeActions();

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Module Shutdown Complete"));
}

void FProceduralMapEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register ProceduralMap asset actions
	TSharedPtr<IAssetTypeActions> ProceduralMapActions = MakeShareable(new FProceduralMapAssetActions());
	AssetTools.RegisterAssetTypeActions(ProceduralMapActions.ToSharedRef());
	RegisteredAssetTypeActions.Add(ProceduralMapActions);

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Registered %d asset type actions"), RegisteredAssetTypeActions.Num());
}

void FProceduralMapEditorModule::UnregisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}
	RegisteredAssetTypeActions.Empty();
}

void FProceduralMapEditorModule::RegisterDetailCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Register BiomeSettings customization
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"BiomeSettings",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBiomeSettingsCustomization::MakeInstance)
	);

	// Register NoiseParameters customization
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"TerrainGenerationParams",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FNoiseParametersCustomization::MakeInstance)
	);

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Registered detail customizations"));
}

void FProceduralMapEditorModule::UnregisterDetailCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("BiomeSettings");
		PropertyModule.UnregisterCustomPropertyTypeLayout("TerrainGenerationParams");
	}
}

void FProceduralMapEditorModule::RegisterEditorModes()
{
	// Register the ProceduralMap editor mode
	FEditorModeRegistry::Get().RegisterMode<FProceduralMapEditorMode>(
		FProceduralMapEditorMode::EM_ProceduralMapEditorModeId,
		LOCTEXT("ProceduralMapEditorModeName", "Procedural Terrain"),
		FSlateIcon(),
		true
	);

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Registered editor mode"));
}

void FProceduralMapEditorModule::UnregisterEditorModes()
{
	FEditorModeRegistry::Get().UnregisterMode(FProceduralMapEditorMode::EM_ProceduralMapEditorModeId);
}

void FProceduralMapEditorModule::RegisterMenuExtensions()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		// Extend the content browser context menu
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu");
		if (Menu)
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry("ProceduralMapActions", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				UContentBrowserAssetContextMenuContext* Context = InSection.FindContext<UContentBrowserAssetContextMenuContext>();
				if (Context)
				{
					// Add custom menu entries here
					// This will be populated by asset actions
				}
			}));
		}
	}));

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditor: Registered menu extensions"));
}

void FProceduralMapEditorModule::UnregisterMenuExtensions()
{
	if (UObjectInitialized())
	{
		UToolMenus::UnregisterOwner(this);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FProceduralMapEditorModule, ProceduralMapEditor)
