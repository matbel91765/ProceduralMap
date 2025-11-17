// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "TerrainGenerationTask.h"
#include "BiomeConfig.h"
#include "TerrainGenerator_EditorUtility.generated.h"

class UQuadTreeTerrain;
class UTerrainPreset;

/**
 * Preset information structure
 */
USTRUCT(BlueprintType)
struct FTerrainPresetInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Preset")
	FString PresetName;

	UPROPERTY(BlueprintReadWrite, Category = "Preset")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "Preset")
	UTerrainPreset* Preset;

	FTerrainPresetInfo()
		: PresetName(TEXT(""))
		, Description(TEXT(""))
		, Preset(nullptr)
	{}
};

/**
 * UTerrainGenerator_EditorUtility
 * Complete editor utility widget for terrain generation
 *
 * Features:
 * - Interactive sliders for all generation parameters
 * - Buttons: Generate, Clear, Export, Import
 * - Integrated preview window
 * - Preset management (Save/Load/Delete)
 * - Seed randomization
 * - Real-time parameter updates
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAPEDITOR_API UTerrainGenerator_EditorUtility : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UTerrainGenerator_EditorUtility();

	// ========================================================================
	// Generation Parameters (exposed to Blueprint/UMG)
	// ========================================================================

	/** Current terrain generation parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FTerrainGenerationParams GenerationParams;

	/** Current biome type for generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	EBiomeType CurrentBiomeType;

	/** Enable erosion simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bEnableErosion;

	/** Erosion iterations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "0", ClampMax = "10000", EditCondition = "bEnableErosion"))
	int32 ErosionIterations;

	/** Terrain size multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float TerrainSizeMultiplier;

	// ========================================================================
	// Preview Settings
	// ========================================================================

	/** Show preview window */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	bool bShowPreview;

	/** Auto-update preview on parameter change */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	bool bAutoUpdatePreview;

	/** Preview resolution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview", meta = (ClampMin = "64", ClampMax = "1024"))
	int32 PreviewResolution;

	// ========================================================================
	// Preset Management
	// ========================================================================

	/** Available presets */
	UPROPERTY(BlueprintReadOnly, Category = "Presets")
	TArray<FTerrainPresetInfo> AvailablePresets;

	/** Currently selected preset */
	UPROPERTY(BlueprintReadWrite, Category = "Presets")
	UTerrainPreset* CurrentPreset;

	/** Current preset name (for saving new presets) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
	FString CurrentPresetName;

	// ========================================================================
	// Blueprint Callable Functions
	// ========================================================================

	/**
	 * Generate terrain with current parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void GenerateTerrain();

	/**
	 * Clear current terrain
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void ClearTerrain();

	/**
	 * Regenerate terrain (clear + generate)
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void RegenerateTerrain();

	/**
	 * Export heightmap to file
	 * @param FilePath - Path to export file
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void ExportHeightmap(const FString& FilePath);

	/**
	 * Import heightmap from file
	 * @param FilePath - Path to import file
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void ImportHeightmap(const FString& FilePath);

	/**
	 * Randomize seed
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void RandomizeSeed();

	/**
	 * Update preview with current parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void UpdatePreview();

	/**
	 * Save current parameters as preset
	 * @param PresetName - Name for the new preset
	 * @param Description - Description of the preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	bool SavePreset(const FString& PresetName, const FString& Description);

	/**
	 * Load preset by name
	 * @param PresetName - Name of preset to load
	 */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	bool LoadPreset(const FString& PresetName);

	/**
	 * Load preset from asset
	 * @param Preset - Preset asset to load
	 */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	void LoadPresetFromAsset(UTerrainPreset* Preset);

	/**
	 * Delete preset by name
	 * @param PresetName - Name of preset to delete
	 */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	bool DeletePreset(const FString& PresetName);

	/**
	 * Refresh available presets list
	 */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	void RefreshPresetsList();

	/**
	 * Get preset names for dropdown
	 */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	TArray<FString> GetPresetNames() const;

	/**
	 * Apply default parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void ApplyDefaultParameters();

	/**
	 * Apply quick preset - Plains
	 */
	UFUNCTION(BlueprintCallable, Category = "Quick Presets")
	void ApplyPlainsPreset();

	/**
	 * Apply quick preset - Mountains
	 */
	UFUNCTION(BlueprintCallable, Category = "Quick Presets")
	void ApplyMountainsPreset();

	/**
	 * Apply quick preset - Desert
	 */
	UFUNCTION(BlueprintCallable, Category = "Quick Presets")
	void ApplyDesertPreset();

	/**
	 * Apply quick preset - Islands
	 */
	UFUNCTION(BlueprintCallable, Category = "Quick Presets")
	void ApplyIslandsPreset();

	/**
	 * Get current terrain actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	UQuadTreeTerrain* GetCurrentTerrainActor();

	/**
	 * Get generation progress (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	float GetGenerationProgress() const;

	/**
	 * Is generation in progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	bool IsGenerating() const;

	/**
	 * Cancel current generation
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
	void CancelGeneration();

	// ========================================================================
	// Blueprint Implementable Events
	// ========================================================================

	/**
	 * Called when generation starts
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnGenerationStarted();

	/**
	 * Called when generation completes
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnGenerationCompleted();

	/**
	 * Called when generation fails
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnGenerationFailed(const FString& ErrorMessage);

	/**
	 * Called when preset is loaded
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPresetLoaded(const FString& PresetName);

	/**
	 * Called when preset is saved
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPresetSaved(const FString& PresetName);

	/**
	 * Called when parameters change
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnParametersChanged();

protected:
	/** Initialize widget */
	virtual void NativeConstruct() override;

	/** Tick function */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** Current terrain actor */
	UPROPERTY()
	UQuadTreeTerrain* CurrentTerrainActor;

	/** Is generation in progress */
	bool bIsGenerating;

	/** Generation progress */
	float GenerationProgress;

	/** Last preview update time */
	double LastPreviewUpdateTime;

	/** Find or create terrain actor in level */
	UQuadTreeTerrain* FindOrCreateTerrainActor();

	/** Load all available presets */
	void LoadAvailablePresets();

	/** Create default presets if they don't exist */
	void CreateDefaultPresets();

	/** Validate parameters before generation */
	bool ValidateParameters() const;

	/** Log generation info */
	void LogGenerationInfo() const;
};
