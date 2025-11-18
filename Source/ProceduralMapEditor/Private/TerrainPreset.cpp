// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "TerrainPreset.h"
#include "QuadTreeTerrain.h"

UTerrainPreset::UTerrainPreset()
{
	// Initialize metadata
	PresetName = TEXT("Untitled Preset");
	Description = TEXT("A terrain generation preset");
	Author = TEXT("");
	Version = 1;

	// Initialize generation parameters
	GenerationParams = FTerrainGenerationParams();
	BiomeType = EBiomeType::Plains;
	TerrainSize = 50000.0f;
	MaxLODLevels = 6;
	MeshResolution = 64;

	// Initialize erosion settings
	bEnableErosion = true;
	ErosionIterations = 1000;
	ErosionStrength = 1.0f;
	bEnableThermalErosion = false;
	ThermalErosionIterations = 500;

	// Initialize biome settings
	bEnableBiomeBlending = true;
	BiomeBlendDistance = 100.0f;

	// Initialize foliage settings
	bEnableFoliage = true;
	FoliageDensityMultiplier = 1.0f;

	// Initialize water settings
	bEnableWater = true;
	WaterLevel = 0.0f;
	bEnableRivers = false;
	NumRivers = 0;

	// Initialize advanced settings
	bUseGPUAcceleration = true;
	bEnableAsyncGeneration = true;
	CustomMaterial = nullptr;
	PreviewThumbnail = nullptr;
}

void UTerrainPreset::ApplyToTerrain(UQuadTreeTerrain* Terrain)
{
	if (!Terrain)
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainPreset: Cannot apply to null terrain"));
		return;
	}

	// Apply generation parameters
	Terrain->GenerationParams = GenerationParams;
	Terrain->TerrainSize = TerrainSize;
	Terrain->MaxLODLevel = MaxLODLevels;
	Terrain->MeshResolution = MeshResolution;
	Terrain->BiomeName = FName(*PresetName);

	// Apply material if specified
	if (CustomMaterial)
	{
		Terrain->TerrainMaterial = CustomMaterial;
	}

	// Apply async settings
	Terrain->bEnableAsyncGeneration = bEnableAsyncGeneration;

	UE_LOG(LogTemp, Log, TEXT("TerrainPreset: Applied preset '%s' to terrain"), *PresetName);
}

UTerrainPreset* UTerrainPreset::DuplicatePreset()
{
	UTerrainPreset* NewPreset = DuplicateObject<UTerrainPreset>(this, GetTransientPackage());

	if (NewPreset)
	{
		NewPreset->PresetName = PresetName + TEXT(" (Copy)");
		NewPreset->Version = 1;
	}

	return NewPreset;
}

bool UTerrainPreset::ValidatePreset(FString& OutErrorMessage) const
{
	// Validate generation parameters
	if (GenerationParams.Frequency <= 0.0f)
	{
		OutErrorMessage = TEXT("Frequency must be greater than 0");
		return false;
	}

	if (GenerationParams.Octaves < 1 || GenerationParams.Octaves > 16)
	{
		OutErrorMessage = TEXT("Octaves must be between 1 and 16");
		return false;
	}

	if (TerrainSize < 1000.0f || TerrainSize > 1000000.0f)
	{
		OutErrorMessage = TEXT("Terrain size must be between 1000 and 1000000");
		return false;
	}

	if (MaxLODLevels < 1 || MaxLODLevels > 10)
	{
		OutErrorMessage = TEXT("Max LOD levels must be between 1 and 10");
		return false;
	}

	if (MeshResolution < 8 || MeshResolution > 256)
	{
		OutErrorMessage = TEXT("Mesh resolution must be between 8 and 256");
		return false;
	}

	if (bEnableErosion && ErosionIterations > 100000)
	{
		OutErrorMessage = TEXT("Erosion iterations should not exceed 100000 for performance");
		return false;
	}

	OutErrorMessage = TEXT("");
	return true;
}

FString UTerrainPreset::GetPresetSummary() const
{
	return FString::Printf(
		TEXT("Preset: %s\n")
		TEXT("Biome: %s\n")
		TEXT("Size: %.0f units\n")
		TEXT("Seed: %d\n")
		TEXT("Octaves: %d\n")
		TEXT("Erosion: %s (%d iterations)\n")
		TEXT("Water: %s\n")
		TEXT("Foliage: %s"),
		*PresetName,
		*UEnum::GetValueAsString(BiomeType),
		TerrainSize,
		GenerationParams.Seed,
		GenerationParams.Octaves,
		bEnableErosion ? TEXT("Enabled") : TEXT("Disabled"),
		ErosionIterations,
		bEnableWater ? TEXT("Enabled") : TEXT("Disabled"),
		bEnableFoliage ? TEXT("Enabled") : TEXT("Disabled")
	);
}

#if WITH_EDITOR
void UTerrainPreset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Validate changes
	FString ErrorMessage;
	if (!ValidatePreset(ErrorMessage))
	{
		UE_LOG(LogTemp, Warning, TEXT("TerrainPreset: Validation warning - %s"), *ErrorMessage);
	}
}
#endif
