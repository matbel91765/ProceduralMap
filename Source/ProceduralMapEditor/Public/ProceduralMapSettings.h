// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TerrainGenerationTask.h"
#include "BiomeConfig.h"
#include "ProceduralMapSettings.generated.h"

/**
 * Performance preset enumeration
 */
UENUM(BlueprintType)
enum class EPerformancePreset : uint8
{
	Low			UMETA(DisplayName = "Low (Mobile)"),
	Medium		UMETA(DisplayName = "Medium (Console)"),
	High		UMETA(DisplayName = "High (PC)"),
	Ultra		UMETA(DisplayName = "Ultra (High-End PC)"),
	Custom		UMETA(DisplayName = "Custom")
};

/**
 * Performance settings for terrain generation
 */
USTRUCT(BlueprintType)
struct FPerformanceSettings
{
	GENERATED_BODY()

	/** Maximum terrain size (in Unreal units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000", ClampMax = "1000000"))
	float MaxTerrainSize;

	/** Maximum LOD levels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxLODLevels;

	/** Default mesh resolution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "8", ClampMax = "256"))
	int32 DefaultMeshResolution;

	/** Enable multi-threading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableMultiThreading;

	/** Maximum worker threads */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "64", EditCondition = "bEnableMultiThreading"))
	int32 MaxWorkerThreads;

	/** Enable GPU acceleration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableGPUAcceleration;

	/** Chunk generation budget (milliseconds per frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float ChunkGenerationBudgetMS;

	/** Enable async mesh updates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableAsyncMeshUpdates;

	FPerformanceSettings()
		: MaxTerrainSize(100000.0f)
		, MaxLODLevels(6)
		, DefaultMeshResolution(64)
		, bEnableMultiThreading(true)
		, MaxWorkerThreads(4)
		, bEnableGPUAcceleration(true)
		, ChunkGenerationBudgetMS(16.0f)
		, bEnableAsyncMeshUpdates(true)
	{}
};

/**
 * Editor preferences for procedural map tools
 */
USTRUCT(BlueprintType)
struct FEditorPreferences
{
	GENERATED_BODY()

	/** Auto-save presets after modifications */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor")
	bool bAutoSavePresets;

	/** Show real-time preview */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor")
	bool bShowRealtimePreview;

	/** Preview update interval (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float PreviewUpdateInterval;

	/** Show debug information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor")
	bool bShowDebugInfo;

	/** Default preview resolution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor", meta = (ClampMin = "64", ClampMax = "1024"))
	int32 DefaultPreviewResolution;

	/** Enable undo/redo for terrain painting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor")
	bool bEnableUndoRedo;

	/** Maximum undo history size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor", meta = (ClampMin = "1", ClampMax = "100", EditCondition = "bEnableUndoRedo"))
	int32 MaxUndoHistorySize;

	FEditorPreferences()
		: bAutoSavePresets(true)
		, bShowRealtimePreview(true)
		, PreviewUpdateInterval(0.1f)
		, bShowDebugInfo(false)
		, DefaultPreviewResolution(256)
		, bEnableUndoRedo(true)
		, MaxUndoHistorySize(20)
	{}
};

/**
 * Asset paths configuration
 */
USTRUCT(BlueprintType)
struct FAssetPathsConfig
{
	GENERATED_BODY()

	/** Default path for terrain presets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paths", meta = (ContentDir))
	FDirectoryPath PresetsPath;

	/** Default path for biome configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paths", meta = (ContentDir))
	FDirectoryPath BiomeConfigsPath;

	/** Default path for terrain materials */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paths", meta = (ContentDir))
	FDirectoryPath MaterialsPath;

	/** Default path for foliage assets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paths", meta = (ContentDir))
	FDirectoryPath FoliagePath;

	/** Default path for exported heightmaps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paths", meta = (ContentDir))
	FDirectoryPath ExportPath;

	FAssetPathsConfig()
	{
		PresetsPath.Path = TEXT("/Game/ProceduralMap/Presets");
		BiomeConfigsPath.Path = TEXT("/Game/ProceduralMap/Biomes");
		MaterialsPath.Path = TEXT("/Game/ProceduralMap/Materials");
		FoliagePath.Path = TEXT("/Game/ProceduralMap/Foliage");
		ExportPath.Path = TEXT("/Game/ProceduralMap/Exports");
	}
};

/**
 * UProceduralMapSettings
 * Global project settings for ProceduralMap system
 * Accessible via Project Settings > Plugins > Procedural Map
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Procedural Map"))
class PROCEDURALMAPEDITOR_API UProceduralMapSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProceduralMapSettings();

	// ========================================================================
	// Default Generation Parameters
	// ========================================================================

	/** Default terrain generation parameters */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Generation|Defaults")
	FTerrainGenerationParams DefaultGenerationParams;

	/** Default biome blend distance */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Generation|Defaults", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float DefaultBiomeBlendDistance;

	/** Default erosion iterations */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Generation|Defaults", meta = (ClampMin = "0", ClampMax = "10000"))
	int32 DefaultErosionIterations;

	/** Enable erosion by default */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Generation|Defaults")
	bool bEnableErosionByDefault;

	/** Enable biome blending by default */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Generation|Defaults")
	bool bEnableBiomeBlendingByDefault;

	// ========================================================================
	// Performance Settings
	// ========================================================================

	/** Performance preset */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Performance")
	EPerformancePreset PerformancePreset;

	/** Performance settings */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (EditCondition = "PerformancePreset == EPerformancePreset::Custom"))
	FPerformanceSettings PerformanceSettings;

	// ========================================================================
	// Asset Paths
	// ========================================================================

	/** Asset paths configuration */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Paths")
	FAssetPathsConfig AssetPaths;

	// ========================================================================
	// Editor Preferences
	// ========================================================================

	/** Editor preferences */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Editor")
	FEditorPreferences EditorPreferences;

	// ========================================================================
	// Advanced Settings
	// ========================================================================

	/** Enable experimental features */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bEnableExperimentalFeatures;

	/** Enable verbose logging */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bEnableVerboseLogging;

	/** Memory budget for terrain generation (MB) */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "100", ClampMax = "10000"))
	int32 MemoryBudgetMB;

	/** Enable profiling */
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bEnableProfiling;

	// ========================================================================
	// Public API
	// ========================================================================

	/** Get singleton instance */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static UProceduralMapSettings* Get();

	/** Apply performance preset */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyPerformancePreset(EPerformancePreset Preset);

	/** Get current performance settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	FPerformanceSettings GetPerformanceSettings() const;

	/** Reset to default settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetToDefaults();

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** Apply low performance preset */
	void ApplyLowPreset();

	/** Apply medium performance preset */
	void ApplyMediumPreset();

	/** Apply high performance preset */
	void ApplyHighPreset();

	/** Apply ultra performance preset */
	void ApplyUltraPreset();
};
