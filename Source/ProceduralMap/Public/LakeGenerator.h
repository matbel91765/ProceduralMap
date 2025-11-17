// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "WaterBodyLakeActor.h"
#include "LakeGenerator.generated.h"

class ALandscape;
class AWaterBodyLake;

/**
 * Lake shape type
 */
UENUM(BlueprintType)
enum class ELakeShape : uint8
{
	Circular UMETA(DisplayName = "Circular"),
	Organic UMETA(DisplayName = "Organic"),
	Irregular UMETA(DisplayName = "Irregular"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Low area detection result
 */
USTRUCT(BlueprintType)
struct FLowAreaData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Center;

	UPROPERTY(BlueprintReadWrite)
	float AverageHeight;

	UPROPERTY(BlueprintReadWrite)
	float Radius;

	UPROPERTY(BlueprintReadWrite)
	int32 PointCount;

	FLowAreaData()
		: Center(FVector::ZeroVector)
		, AverageHeight(0.0f)
		, Radius(0.0f)
		, PointCount(0)
	{}
};

/**
 * Island configuration
 */
USTRUCT(BlueprintType)
struct FIslandConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height;

	FIslandConfig()
		: Location(FVector::ZeroVector)
		, Radius(500.0f)
		, Height(100.0f)
	{}
};

/**
 * Main lake generator actor
 * Generates procedural lakes with closed-loop splines in low-lying terrain areas
 */
UCLASS(Blueprintable)
class PROCEDURALMAP_API ALakeGenerator : public AActor
{
	GENERATED_BODY()

public:
	ALakeGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== Components ==========

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	/** Lake boundary spline (closed loop) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* LakeSpline;

	// ========== Lake Parameters ==========

	/** Lake shape type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Parameters")
	ELakeShape LakeShape;

	/** Target radius for the lake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Parameters", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
	float LakeRadius;

	/** Depth of the lake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Parameters", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
	float LakeDepth;

	/** Water level height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Parameters")
	float WaterLevel;

	/** Number of spline points for lake boundary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Parameters", meta = (ClampMin = "8", ClampMax = "128"))
	int32 BoundaryPoints;

	/** Irregularity factor (0 = perfect circle, 1 = very irregular) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Parameters", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Irregularity;

	// ========== Generation Settings ==========

	/** Center location for the lake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Generation")
	FVector LakeCenter;

	/** Auto-detect low areas for lake placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Generation")
	bool bAutoDetectLowAreas;

	/** Search radius for low area detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Generation", meta = (EditCondition = "bAutoDetectLowAreas"))
	float SearchRadius;

	/** Minimum area size to be considered for lake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Generation", meta = (EditCondition = "bAutoDetectLowAreas"))
	float MinimumAreaSize;

	/** Maximum number of lakes to generate in auto mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Generation", meta = (EditCondition = "bAutoDetectLowAreas"))
	int32 MaxLakesCount;

	// ========== Noise Settings ==========

	/** Apply noise to lake boundary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Noise")
	bool bApplyNoise;

	/** Noise frequency for boundary variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Noise", meta = (EditCondition = "bApplyNoise"))
	float NoiseFrequency;

	/** Noise amplitude for boundary displacement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Noise", meta = (EditCondition = "bApplyNoise"))
	float NoiseAmplitude;

	/** Number of noise octaves */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Noise", meta = (EditCondition = "bApplyNoise"))
	int32 NoiseOctaves;

	/** Noise seed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Noise")
	int32 NoiseSeed;

	// ========== Terrain Integration ==========

	/** Reference to landscape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Terrain")
	ALandscape* TargetLandscape;

	/** Carve terrain for lake bed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Terrain")
	bool bCarveTerrainForLake;

	/** Smooth shoreline transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Terrain", meta = (EditCondition = "bCarveTerrainForLake"))
	float ShorelineSmoothness;

	/** Beach width around lake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Terrain")
	float BeachWidth;

	// ========== Water System Integration ==========

	/** Enable UE5 Water System integration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Water")
	bool bCreateWaterBody;

	/** Water body class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Water", meta = (EditCondition = "bCreateWaterBody"))
	TSubclassOf<AWaterBodyLake> WaterBodyClass;

	/** Reference to spawned water body */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lake|Water")
	AWaterBodyLake* WaterBodyLake;

	// ========== Islands ==========

	/** Island configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Islands")
	TArray<FIslandConfig> Islands;

	/** Auto-generate random islands */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Islands")
	bool bAutoGenerateIslands;

	/** Number of islands to auto-generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Islands", meta = (EditCondition = "bAutoGenerateIslands", ClampMin = "0", ClampMax = "10"))
	int32 IslandCount;

	/** Minimum distance from shore for islands */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Islands", meta = (EditCondition = "bAutoGenerateIslands"))
	float IslandMinDistanceFromShore;

	// ========== Custom Shape ==========

	/** Custom spline points for lake shape (used when LakeShape is Custom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Custom")
	TArray<FVector> CustomShapePoints;

	// ========== Generation Methods ==========

	/**
	 * Generate the lake
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void GenerateLake();

	/**
	 * Detect low areas in terrain for lake placement
	 * @param ScanCenter Center of scan area
	 * @param ScanRadius Radius to scan
	 * @return Array of detected low areas
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	TArray<FLowAreaData> DetectLowAreas(const FVector& ScanCenter, float ScanRadius);

	/**
	 * Generate circular lake shape
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void GenerateCircularLake();

	/**
	 * Generate organic lake shape (natural-looking)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void GenerateOrganicLake();

	/**
	 * Generate irregular lake shape (very natural)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void GenerateIrregularLake();

	/**
	 * Generate lake from custom points
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void GenerateCustomLake();

	/**
	 * Apply procedural noise to lake boundary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void ApplyNoiseToLakeBoundary();

	/**
	 * Carve terrain for lake bed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void CarveTerrainForLake();

	/**
	 * Create UE5 Water Body actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void CreateWaterBody();

	/**
	 * Generate islands within the lake
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void GenerateIslands();

	/**
	 * Clear generated lake
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Generation")
	void ClearLake();

	/**
	 * Get terrain height at position
	 * @param WorldPosition World position
	 * @return Terrain height
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Utilities")
	float GetTerrainHeight(const FVector& WorldPosition) const;

	/**
	 * Check if point is inside lake boundary
	 * @param WorldPosition Position to check
	 * @return True if inside lake
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Utilities")
	bool IsPointInsideLake(const FVector& WorldPosition) const;

	/**
	 * Get distance from point to nearest shore
	 * @param WorldPosition Position to check
	 * @return Distance to shore
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Utilities")
	float GetDistanceToShore(const FVector& WorldPosition) const;

	/**
	 * Calculate lake surface area
	 * @return Surface area in cm²
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Utilities")
	float CalculateLakeSurfaceArea() const;

	/**
	 * Calculate lake volume
	 * @return Volume in cm³
	 */
	UFUNCTION(BlueprintCallable, Category = "Lake|Utilities")
	float CalculateLakeVolume() const;

private:
	/**
	 * Generate spline points in circular pattern
	 */
	TArray<FVector> GenerateCircularPattern(const FVector& Center, float Radius, int32 NumPoints);

	/**
	 * Apply radial noise to points
	 */
	void ApplyRadialNoise(TArray<FVector>& Points, float Frequency, float Amplitude);

	/**
	 * Get noise value at angle
	 */
	float GetRadialNoise(float Angle, int32 Seed) const;

	/**
	 * Smooth lake boundary
	 */
	void SmoothLakeBoundary(int32 Iterations = 2);

	/**
	 * Check if area is suitable for lake
	 */
	bool IsSuitableForLake(const FVector& Center, float Radius) const;

	/**
	 * Calculate average height in area
	 */
	float CalculateAverageHeight(const FVector& Center, float Radius, int32 Samples = 20) const;

	/**
	 * Find local height minimum
	 */
	FVector FindLocalMinimum(const FVector& StartPosition, float SearchRadius) const;

	/**
	 * Modify terrain at position for lake bed
	 */
	void ModifyTerrainAtPoint(const FVector& Position, float Radius, float TargetHeight);

	/**
	 * Generate random island position
	 */
	FVector GenerateIslandPosition() const;

	/**
	 * Create island mesh/terrain
	 */
	void CreateIsland(const FIslandConfig& Config);

	/**
	 * Point-in-polygon test (2D)
	 */
	bool IsPointInPolygon(const FVector2D& Point, const TArray<FVector2D>& Polygon) const;

	// Cache for performance
	TArray<FVector> CachedBoundaryPoints;
	bool bLakeGenerated;
};
