// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HydraulicErosion.h"
#include "ThermalErosion.h"
#include "ErosionComputeShader.h"
#include "ErosionExample.generated.h"

/**
 * AErosionExample - Example actor demonstrating erosion usage
 *
 * This actor shows how to use the hydraulic and thermal erosion systems
 * to create realistic terrain. It can be placed in the level and configured
 * via the Details panel.
 *
 * Usage:
 * 1. Place this actor in your level
 * 2. Configure erosion parameters in Details panel
 * 3. Call ApplyErosion() from Blueprint or C++
 * 4. The actor will apply erosion to its heightmap and generate a mesh
 */
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALMAP_API AErosionExample : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Constructor
	 */
	AErosionExample();

	/**
	 * Called every frame
	 */
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// Main API
	// ========================================

	/**
	 * Apply erosion to the heightmap
	 * This will run hydraulic erosion followed by thermal erosion
	 * @param bUseGPU - If true, uses GPU compute shader for hydraulic erosion
	 * @param bAsync - If true, runs erosion asynchronously
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	void ApplyErosion(bool bUseGPU = false, bool bAsync = false);

	/**
	 * Apply only hydraulic erosion
	 * @param bUseGPU - If true, uses GPU compute shader
	 * @param bAsync - If true, runs erosion asynchronously
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	void ApplyHydraulicErosion(bool bUseGPU = false, bool bAsync = false);

	/**
	 * Apply only thermal erosion
	 * @param bAsync - If true, runs erosion asynchronously
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	void ApplyThermalErosion(bool bAsync = false);

	/**
	 * Generate a simple test heightmap (for demonstration)
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	void GenerateTestHeightmap();

	/**
	 * Check if erosion is complete
	 */
	UFUNCTION(BlueprintPure, Category = "Erosion")
	bool IsErosionComplete() const;

	/**
	 * Reset heightmap to original state
	 */
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	void ResetHeightmap();

	// ========================================
	// Configuration
	// ========================================

	/** Size of the heightmap (square: MapSize x MapSize) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Setup", meta = (ClampMin = "32", ClampMax = "2048"))
	int32 MapSize;

	/** Hydraulic erosion system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Erosion|Hydraulic")
	UHydraulicErosion* HydraulicErosion;

	/** Thermal erosion system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Erosion|Thermal")
	UThermalErosion* ThermalErosion;

	/** Enable thermal erosion after hydraulic erosion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Setup")
	bool bEnableThermalErosion;

	/** Auto-apply erosion on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Setup")
	bool bAutoApplyOnBeginPlay;

	/** Preview the erosion in editor (when parameters change) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Setup")
	bool bPreviewInEditor;

protected:
	/**
	 * Called when the game starts
	 */
	virtual void BeginPlay() override;

	/**
	 * Called after construction or when properties change in editor
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
	/** Current heightmap data */
	TArray<float> HeightMap;

	/** Original heightmap (for reset) */
	TArray<float> OriginalHeightMap;

	/** Is erosion currently running? */
	bool bIsErosionRunning;
};
