// GrassGenerator.h - Specialized Grass and Ground Cover Generator
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "LandscapeGrassType.h"
#include "GrassGenerator.generated.h"

/**
 * Grass density mode
 */
UENUM(BlueprintType)
enum class EGrassDensityMode : uint8
{
	/** Uniform density across area */
	Uniform UMETA(DisplayName = "Uniform"),

	/** Noise-based density variation */
	NoiseVariation UMETA(DisplayName = "Noise Variation"),

	/** Texture-based density mask */
	TextureMask UMETA(DisplayName = "Texture Mask"),

	/** Biome-based density */
	BiomeBased UMETA(DisplayName = "Biome Based")
};

/**
 * Wind animation settings
 */
USTRUCT(BlueprintType)
struct FGrassWindSettings
{
	GENERATED_BODY()

	/** Enable wind animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	bool bEnableWind = true;

	/** Wind strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindStrength = 0.5f;

	/** Wind speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0"))
	float WindSpeed = 1.0f;

	/** Wind direction (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float WindDirection = 0.0f;

	/** Wind turbulence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindTurbulence = 0.3f;

	/** Wind gust frequency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0"))
	float GustFrequency = 0.5f;

	/** Wind material parameter name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	FName WindParameterName = TEXT("WindStrength");

	FGrassWindSettings()
	{
	}
};

/**
 * Grass type configuration
 */
USTRUCT(BlueprintType)
struct FGrassTypeConfig
{
	GENERATED_BODY()

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName GrassName = NAME_None;

	/** Static mesh for grass */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* GrassMesh = nullptr;

	/** Alternative meshes for variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TArray<UStaticMesh*> VariationMeshes;

	/** Instances per square meter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float Density = 100.0f;

	/** Density jitter (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DensityJitter = 0.5f;

	/** Minimum scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.1"))
	float MinScale = 0.8f;

	/** Maximum scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (ClampMin = "0.1"))
	float MaxScale = 1.2f;

	/** Scale variance per axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale")
	FVector ScaleVariance = FVector(0.1f, 0.1f, 0.2f);

	/** Random rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bRandomRotation = true;

	/** Align to ground normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bAlignToGround = true;

	/** Ground alignment strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AlignmentStrength = 0.5f;

	/** Enable GPU instancing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableGPUInstancing = true;

	/** Max instances per component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100"))
	int32 MaxInstancesPerComponent = 10000;

	/** Culling distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000.0"))
	float CullDistance = 10000.0f;

	/** Start culling fade distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "500.0"))
	float StartCullFadeDistance = 8000.0f;

	/** Cast shadows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bCastShadow = false;

	/** Receive decals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	bool bReceiveDecals = false;

	/** Override material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	UMaterialInterface* OverrideMaterial = nullptr;

	/** Wind settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
	FGrassWindSettings WindSettings;

	/** Spawn weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
	float SpawnWeight = 1.0f;

	/** Slope restriction (max degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSlope = 60.0f;

	/** Biome restrictions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Restrictions")
	TArray<FName> AllowedBiomes;

	FGrassTypeConfig()
	{
	}
};

/**
 * Grass generation statistics
 */
USTRUCT(BlueprintType)
struct FGrassGenerationStats
{
	GENERATED_BODY()

	/** Total instances generated */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalInstances = 0;

	/** Number of components created */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 ComponentCount = 0;

	/** Area covered (square meters) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float AreaCovered = 0.0f;

	/** Generation time (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float GenerationTime = 0.0f;

	/** Memory usage (MB) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float MemoryUsageMB = 0.0f;

	FGrassGenerationStats()
	{
	}
};

/**
 * Grass Generator Actor
 * Specialized system for generating dense grass and ground cover with GPU instancing
 */
UCLASS(Blueprintable, ClassGroup = "ProceduralMap")
class PROCEDURALMAP_API AGrassGenerator : public AActor
{
	GENERATED_BODY()

public:
	AGrassGenerator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ============================================
	// Configuration
	// ============================================

	/** Grass types to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass")
	TArray<FGrassTypeConfig> GrassTypes;

	/** Landscape grass type (for integration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass")
	ULandscapeGrassType* LandscapeGrassType;

	/** Density mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
	EGrassDensityMode DensityMode = EGrassDensityMode::NoiseVariation;

	/** Base density multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float BaseDensityMultiplier = 1.0f;

	/** Density noise scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0.0001"))
	float DensityNoiseScale = 0.01f;

	/** Density texture mask (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
	UTexture2D* DensityMask = nullptr;

	/** Density mask channel (R=0, G=1, B=2, A=3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density", meta = (ClampMin = "0", ClampMax = "3"))
	int32 DensityMaskChannel = 0;

	/** Generate on begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bGenerateOnBeginPlay = false;

	/** Auto-update in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	bool bAutoUpdate = false;

	/** Generation radius from actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "100.0"))
	float GenerationRadius = 10000.0f;

	/** Grid cell size for batching (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "100.0"))
	float GridCellSize = 1000.0f;

	/** Random seed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 RandomSeed = 54321;

	/** Enable streaming (generate grass in chunks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	bool bEnableStreaming = false;

	/** Streaming distance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "1000.0"))
	float StreamingDistance = 15000.0f;

	/** Chunk size for streaming (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "1000.0"))
	float ChunkSize = 5000.0f;

	/** Enable GPU instancing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableGPUInstancing = true;

	/** Max instances per component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100"))
	int32 MaxInstancesPerComponent = 10000;

	/** Enable async generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableAsyncGeneration = false;

	/** Debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugDraw = false;

	/** Show statistics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowStats = false;

	// ============================================
	// Methods
	// ============================================

	/**
	 * Generate grass in radius around actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Grass")
	FGrassGenerationStats GenerateGrass(bool bClearExisting = true);

	/**
	 * Generate grass in specific area
	 */
	UFUNCTION(BlueprintCallable, Category = "Grass")
	FGrassGenerationStats GenerateGrassInBounds(const FBox& Bounds, bool bClearExisting = true);

	/**
	 * Generate grass around location
	 */
	UFUNCTION(BlueprintCallable, Category = "Grass")
	FGrassGenerationStats GenerateGrassAtLocation(const FVector& Location, float Radius, bool bClearExisting = true);

	/**
	 * Clear all generated grass
	 */
	UFUNCTION(BlueprintCallable, Category = "Grass")
	void ClearAllGrass();

	/**
	 * Update grass streaming based on view location
	 */
	UFUNCTION(BlueprintCallable, Category = "Grass")
	void UpdateStreaming(const FVector& ViewLocation);

	/**
	 * Get density at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Grass")
	float GetDensityAtLocation(const FVector& Location, int32 GrassTypeIndex) const;

	/**
	 * Update wind parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Wind")
	void UpdateWindParameters(const FGrassWindSettings& WindSettings);

	/**
	 * Set wind direction globally
	 */
	UFUNCTION(BlueprintCallable, Category = "Wind")
	void SetWindDirection(float Degrees);

	/**
	 * Set wind strength globally
	 */
	UFUNCTION(BlueprintCallable, Category = "Wind")
	void SetWindStrength(float Strength);

	/**
	 * Get generation statistics
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	FGrassGenerationStats GetGenerationStats() const { return GenerationStats; }

	/**
	 * Get total instance count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	int32 GetTotalInstanceCount() const;

	/**
	 * Integrate with landscape grass system
	 */
	UFUNCTION(BlueprintCallable, Category = "Integration")
	void IntegrateWithLandscape(class ALandscape* Landscape);

protected:
	// ============================================
	// Internal Methods
	// ============================================

	/** Create HISM components for grass types */
	void CreateGrassComponents();

	/** Get or create grass component */
	UHierarchicalInstancedStaticMeshComponent* GetOrCreateGrassComponent(int32 GrassTypeIndex, int32 ComponentIndex = 0);

	/** Generate grass in grid cell */
	void GenerateGrassInCell(const FVector2D& CellCenter, int32 GrassTypeIndex);

	/** Calculate density with noise/texture */
	float CalculateDensityModifier(const FVector& Location, int32 GrassTypeIndex) const;

	/** Get noise value for density */
	float GetDensityNoise(const FVector& Location) const;

	/** Sample density from texture */
	float SampleDensityTexture(const FVector& Location) const;

	/** Generate grass instance transform */
	FTransform GenerateGrassTransform(const FVector& Location, const FVector& Normal, int32 GrassTypeIndex);

	/** Add grass instance */
	void AddGrassInstance(int32 GrassTypeIndex, const FTransform& Transform);

	/** Update material parameters for wind */
	void UpdateMaterialWindParameters(UHierarchicalInstancedStaticMeshComponent* Component, const FGrassWindSettings& WindSettings);

	/** Check if location should have grass */
	bool ShouldSpawnGrass(const FVector& Location, const FVector& Normal, int32 GrassTypeIndex) const;

	/** Draw debug visualization */
	void DrawDebugVisualization() const;

protected:
	// ============================================
	// Internal Data
	// ============================================

	/** HISM components for grass types */
	UPROPERTY()
	TMap<int32, TArray<UHierarchicalInstancedStaticMeshComponent*>> GrassComponents;

	/** Random stream */
	FRandomStream RandomStream;

	/** Generation statistics */
	UPROPERTY()
	FGrassGenerationStats GenerationStats;

	/** Active streaming chunks */
	TSet<FIntVector> ActiveChunks;

	/** Last view location for streaming */
	FVector LastViewLocation = FVector::ZeroVector;

	/** Wind time accumulator */
	float WindTimeAccumulator = 0.0f;

	/** Current wind vector */
	FVector CurrentWindVector = FVector::ZeroVector;
};
