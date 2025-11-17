// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "TerrainPresetFactory.h"
#include "TerrainPreset.h"
#include "AssetTypeCategories.h"

#define LOCTEXT_NAMESPACE "TerrainPresetFactory"

UTerrainPresetFactory::UTerrainPresetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTerrainPreset::StaticClass();
}

UObject* UTerrainPresetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UTerrainPreset* NewPreset = NewObject<UTerrainPreset>(InParent, InClass, InName, Flags);

	if (NewPreset)
	{
		NewPreset->PresetName = InName.ToString();
		UE_LOG(LogTemp, Log, TEXT("TerrainPresetFactory: Created new preset '%s'"), *InName.ToString());
	}

	return NewPreset;
}

bool UTerrainPresetFactory::ShouldShowInNewMenu() const
{
	return true;
}

FText UTerrainPresetFactory::GetDisplayName() const
{
	return LOCTEXT("TerrainPresetDisplayName", "Terrain Preset");
}

uint32 UTerrainPresetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

FText UTerrainPresetFactory::GetToolTip() const
{
	return LOCTEXT("TerrainPresetTooltip", "Create a new Terrain Generation Preset");
}

FString UTerrainPresetFactory::GetDefaultNewAssetName() const
{
	return TEXT("NewTerrainPreset");
}

#undef LOCTEXT_NAMESPACE
