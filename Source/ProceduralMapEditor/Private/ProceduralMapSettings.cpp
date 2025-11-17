// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ProceduralMapSettings.h"

#define LOCTEXT_NAMESPACE "ProceduralMapSettings"

UProceduralMapSettings::UProceduralMapSettings()
{
	// Initialize default generation parameters
	DefaultGenerationParams = FTerrainGenerationParams();
	DefaultBiomeBlendDistance = 100.0f;
	DefaultErosionIterations = 1000;
	bEnableErosionByDefault = true;
	bEnableBiomeBlendingByDefault = true;

	// Initialize performance settings
	PerformancePreset = EPerformancePreset::High;
	PerformanceSettings = FPerformanceSettings();

	// Initialize asset paths
	AssetPaths = FAssetPathsConfig();

	// Initialize editor preferences
	EditorPreferences = FEditorPreferences();

	// Initialize advanced settings
	bEnableExperimentalFeatures = false;
	bEnableVerboseLogging = false;
	MemoryBudgetMB = 2048;
	bEnableProfiling = false;
}

UProceduralMapSettings* UProceduralMapSettings::Get()
{
	return GetMutableDefault<UProceduralMapSettings>();
}

void UProceduralMapSettings::ApplyPerformancePreset(EPerformancePreset Preset)
{
	PerformancePreset = Preset;

	switch (Preset)
	{
	case EPerformancePreset::Low:
		ApplyLowPreset();
		break;
	case EPerformancePreset::Medium:
		ApplyMediumPreset();
		break;
	case EPerformancePreset::High:
		ApplyHighPreset();
		break;
	case EPerformancePreset::Ultra:
		ApplyUltraPreset();
		break;
	case EPerformancePreset::Custom:
		// Keep current settings
		break;
	default:
		break;
	}

	SaveConfig();

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapSettings: Applied performance preset: %d"), (int32)Preset);
}

FPerformanceSettings UProceduralMapSettings::GetPerformanceSettings() const
{
	return PerformanceSettings;
}

void UProceduralMapSettings::ResetToDefaults()
{
	// Reset to default values
	DefaultGenerationParams = FTerrainGenerationParams();
	DefaultBiomeBlendDistance = 100.0f;
	DefaultErosionIterations = 1000;
	bEnableErosionByDefault = true;
	bEnableBiomeBlendingByDefault = true;

	PerformancePreset = EPerformancePreset::High;
	ApplyHighPreset();

	AssetPaths = FAssetPathsConfig();
	EditorPreferences = FEditorPreferences();

	bEnableExperimentalFeatures = false;
	bEnableVerboseLogging = false;
	MemoryBudgetMB = 2048;
	bEnableProfiling = false;

	SaveConfig();

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapSettings: Reset to defaults"));
}

FName UProceduralMapSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FText UProceduralMapSettings::GetSectionText() const
{
	return LOCTEXT("ProceduralMapSettingsSection", "Procedural Map");
}

#if WITH_EDITOR
void UProceduralMapSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// Handle performance preset changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UProceduralMapSettings, PerformancePreset))
	{
		if (PerformancePreset != EPerformancePreset::Custom)
		{
			ApplyPerformancePreset(PerformancePreset);
		}
	}

	// Handle custom performance settings changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UProceduralMapSettings, PerformanceSettings))
	{
		PerformancePreset = EPerformancePreset::Custom;
	}

	SaveConfig();
}
#endif

void UProceduralMapSettings::ApplyLowPreset()
{
	PerformanceSettings.MaxTerrainSize = 20000.0f;
	PerformanceSettings.MaxLODLevels = 3;
	PerformanceSettings.DefaultMeshResolution = 32;
	PerformanceSettings.bEnableMultiThreading = true;
	PerformanceSettings.MaxWorkerThreads = 2;
	PerformanceSettings.bEnableGPUAcceleration = false;
	PerformanceSettings.ChunkGenerationBudgetMS = 8.0f;
	PerformanceSettings.bEnableAsyncMeshUpdates = false;

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapSettings: Applied Low preset"));
}

void UProceduralMapSettings::ApplyMediumPreset()
{
	PerformanceSettings.MaxTerrainSize = 50000.0f;
	PerformanceSettings.MaxLODLevels = 4;
	PerformanceSettings.DefaultMeshResolution = 48;
	PerformanceSettings.bEnableMultiThreading = true;
	PerformanceSettings.MaxWorkerThreads = 4;
	PerformanceSettings.bEnableGPUAcceleration = true;
	PerformanceSettings.ChunkGenerationBudgetMS = 12.0f;
	PerformanceSettings.bEnableAsyncMeshUpdates = true;

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapSettings: Applied Medium preset"));
}

void UProceduralMapSettings::ApplyHighPreset()
{
	PerformanceSettings.MaxTerrainSize = 100000.0f;
	PerformanceSettings.MaxLODLevels = 6;
	PerformanceSettings.DefaultMeshResolution = 64;
	PerformanceSettings.bEnableMultiThreading = true;
	PerformanceSettings.MaxWorkerThreads = 8;
	PerformanceSettings.bEnableGPUAcceleration = true;
	PerformanceSettings.ChunkGenerationBudgetMS = 16.0f;
	PerformanceSettings.bEnableAsyncMeshUpdates = true;

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapSettings: Applied High preset"));
}

void UProceduralMapSettings::ApplyUltraPreset()
{
	PerformanceSettings.MaxTerrainSize = 200000.0f;
	PerformanceSettings.MaxLODLevels = 8;
	PerformanceSettings.DefaultMeshResolution = 128;
	PerformanceSettings.bEnableMultiThreading = true;
	PerformanceSettings.MaxWorkerThreads = 16;
	PerformanceSettings.bEnableGPUAcceleration = true;
	PerformanceSettings.ChunkGenerationBudgetMS = 33.0f;
	PerformanceSettings.bEnableAsyncMeshUpdates = true;

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapSettings: Applied Ultra preset"));
}

#undef LOCTEXT_NAMESPACE
