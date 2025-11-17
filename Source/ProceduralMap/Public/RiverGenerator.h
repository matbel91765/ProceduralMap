// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "WaterBodyRiverActor.h"
#include "RiverGenerator.generated.h"

class ALandscape;
class AWaterBodyRiver;

/**
 * River path node for A* pathfinding
 */
USTRUCT(BlueprintType)
struct FRiverPathNode
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Position;

	UPROPERTY(BlueprintReadWrite)
	float GCost;

	UPROPERTY(BlueprintReadWrite)
	float HCost;

	UPROPERTY(BlueprintReadWrite)
	int32 ParentIndex;

	FRiverPathNode()
		: Position(FVector::ZeroVector)
		, GCost(0.0f)
		, HCost(0.0f)
		, ParentIndex(-1)
	{}

	float GetFCost() const { return GCost + HCost; }
};

/**
 * Tributary configuration
 */
USTRUCT(BlueprintType)
struct FTributaryConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SourceLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WidthMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlowSpeedMultiplier;

	FTributaryConfig()
		: SourceLocation(FVector::ZeroVector)
		, WidthMultiplier(0.5f)
		, FlowSpeedMultiplier(0.7f)
	{}
};

/**
 * Main river generator actor
 * Generates procedural rivers using splines with A* pathfinding and landscape modification
 */
UCLASS(Blueprintable)
class PROCEDURALMAP_API ARiverGenerator : public AActor
{
	GENERATED_BODY()

public:
	ARiverGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== Components ==========

	/** Main spline component for the river path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* RiverSpline;

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	// ========== River Parameters ==========

	/** Width of the river in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float Width;

	/** Depth to carve into landscape in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float Depth;

	/** Flow speed of water */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float FlowSpeed;

	/** Curvature amount for the river path (0 = straight, 1 = very curvy) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Curvature;

	/** Width variation along the river (randomness) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Parameters", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WidthVariation;

	// ========== Generation Settings ==========

	/** Starting point of the river */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
	FVector SourceLocation;

	/** Ending point of the river */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
	FVector DestinationLocation;

	/** Whether to use automatic destination (find lowest point) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
	bool bAutoFindDestination;

	/** Search radius for finding lowest point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation", meta = (EditCondition = "bAutoFindDestination"))
	float DestinationSearchRadius;

	/** Step size for pathfinding in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float PathfindingStepSize;

	/** Prefer downhill paths (higher = stronger preference) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float DownhillBias;

	// ========== Curl Noise Settings ==========

	/** Enable curl noise distortion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Noise")
	bool bApplyCurlNoise;

	/** Frequency of curl noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Noise", meta = (EditCondition = "bApplyCurlNoise"))
	float NoiseFrequency;

	/** Amplitude of curl noise displacement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Noise", meta = (EditCondition = "bApplyCurlNoise"))
	float NoiseAmplitude;

	/** Seed for noise generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Noise")
	int32 NoiseSeed;

	// ========== Landscape Modification ==========

	/** Reference to landscape to modify */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Landscape")
	ALandscape* TargetLandscape;

	/** Enable landscape carving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Landscape")
	bool bModifyLandscape;

	/** Smoothing radius around the river */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Landscape", meta = (EditCondition = "bModifyLandscape"))
	float SmoothingRadius;

	/** Falloff curve for landscape modification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Landscape", meta = (EditCondition = "bModifyLandscape"))
	float FalloffPower;

	// ========== Water System Integration ==========

	/** Enable UE5 Water System integration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Water")
	bool bCreateWaterBody;

	/** Water body class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Water", meta = (EditCondition = "bCreateWaterBody"))
	TSubclassOf<AWaterBodyRiver> WaterBodyClass;

	/** Reference to spawned water body */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River|Water")
	AWaterBodyRiver* WaterBodyRiver;

	// ========== Tributaries ==========

	/** Tributary configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Tributaries")
	TArray<FTributaryConfig> Tributaries;

	/** Generated tributary actors */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River|Tributaries")
	TArray<ARiverGenerator*> TributaryActors;

	// ========== Generation Methods ==========

	/**
	 * Generate the complete river system
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	void GenerateRiver();

	/**
	 * Find lowest path from source to destination using A* or gradient descent
	 * @param Start Starting position
	 * @param End Ending position
	 * @return Array of path points
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	TArray<FVector> FindLowestPath(const FVector& Start, const FVector& End);

	/**
	 * Generate river spline from path points
	 * @param PathPoints Array of positions along the river
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	void GenerateRiverSpline(const TArray<FVector>& PathPoints);

	/**
	 * Apply curl noise distortion to the spline
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	void ApplyCurlNoise();

	/**
	 * Modify landscape to carve river bed
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	void ModifyLandscape();

	/**
	 * Create UE5 Water Body actor
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	void CreateWaterBody();

	/**
	 * Generate all tributaries
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	void GenerateTributaries();

	/**
	 * Clear generated river data
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Generation")
	void ClearRiver();

	/**
	 * Get terrain height at world position
	 * @param WorldPosition World position to query
	 * @return Height at position
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Utilities")
	float GetTerrainHeight(const FVector& WorldPosition) const;

	/**
	 * Get terrain normal at world position
	 * @param WorldPosition World position to query
	 * @return Normal vector at position
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Utilities")
	FVector GetTerrainNormal(const FVector& WorldPosition) const;

	/**
	 * Calculate slope between two points
	 * @param Point1 First point
	 * @param Point2 Second point
	 * @return Slope in degrees
	 */
	UFUNCTION(BlueprintCallable, Category = "River|Utilities")
	float CalculateSlope(const FVector& Point1, const FVector& Point2) const;

private:
	/**
	 * A* pathfinding implementation
	 */
	TArray<FVector> FindPathAStar(const FVector& Start, const FVector& End);

	/**
	 * Gradient descent pathfinding (simpler, faster)
	 */
	TArray<FVector> FindPathGradientDescent(const FVector& Start, const FVector& End);

	/**
	 * Get neighbors for A* pathfinding
	 */
	TArray<FVector> GetNeighbors(const FVector& Position) const;

	/**
	 * Calculate heuristic for A* (Euclidean distance + elevation bias)
	 */
	float CalculateHeuristic(const FVector& From, const FVector& To) const;

	/**
	 * Check if position is valid for river placement
	 */
	bool IsValidRiverPosition(const FVector& Position) const;

	/**
	 * Smooth path using moving average
	 */
	TArray<FVector> SmoothPath(const TArray<FVector>& Path, int32 WindowSize = 3) const;

	/**
	 * Calculate curl noise at position
	 */
	FVector2D CalculateCurlNoise(const FVector& Position) const;

	/**
	 * Calculate noise derivative for curl
	 */
	float NoiseDerivative(float X, float Y, bool bXDerivative) const;

	/**
	 * Get 2D noise value
	 */
	float GetNoise2D(float X, float Y) const;

	/**
	 * Find lowest point within radius
	 */
	FVector FindLowestPoint(const FVector& Center, float Radius) const;

	/**
	 * Modify landscape heightmap at position
	 */
	void ModifyLandscapeAtPoint(const FVector& Position, float Radius, float TargetHeight);

	/**
	 * Sample existing spline points for tributary connection
	 */
	FVector FindNearestSplinePoint(const FVector& Position) const;

	// Cache for performance
	TArray<FVector> CachedPathPoints;
	bool bRiverGenerated;
};
