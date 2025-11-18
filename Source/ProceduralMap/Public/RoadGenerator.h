// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"
#include "BiomeConfig.h"
#include "RoadGenerator.generated.h"

class ALandscape;
class UPCGComponent;

/**
 * Road path node for A* pathfinding
 */
USTRUCT(BlueprintType)
struct FRoadPathNode
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

	FRoadPathNode()
		: Position(FVector::ZeroVector)
		, GCost(0.0f)
		, HCost(0.0f)
		, ParentIndex(-1)
	{}

	float GetFCost() const { return GCost + HCost; }
};

/**
 * Point of interest for road network
 */
USTRUCT(BlueprintType)
struct FPointOfInterest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority;

	FPointOfInterest()
		: Location(FVector::ZeroVector)
		, Name(TEXT("POI"))
		, Priority(1)
	{}
};

/**
 * Road intersection data
 */
USTRUCT(BlueprintType)
struct FRoadIntersection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Location;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> ConnectedRoadIndices;

	UPROPERTY(BlueprintReadWrite)
	int32 IntersectionType; // 0 = T-junction, 1 = Cross, 2 = Y-junction

	FRoadIntersection()
		: Location(FVector::ZeroVector)
		, IntersectionType(0)
	{}
};

/**
 * Road segment between two points
 */
USTRUCT(BlueprintType)
struct FRoadSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector StartPoint;

	UPROPERTY(BlueprintReadWrite)
	FVector EndPoint;

	UPROPERTY(BlueprintReadWrite)
	USplineComponent* SegmentSpline;

	UPROPERTY(BlueprintReadWrite)
	float RoadWidth;

	FRoadSegment()
		: StartPoint(FVector::ZeroVector)
		, EndPoint(FVector::ZeroVector)
		, SegmentSpline(nullptr)
		, RoadWidth(400.0f)
	{}
};

/**
 * Main road generator actor
 * Generates procedural roads between points of interest using A* pathfinding
 */
UCLASS(Blueprintable)
class PROCEDURALMAP_API ARoadGenerator : public AActor
{
	GENERATED_BODY()

public:
	ARoadGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== Components ==========

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	/** Procedural mesh for road surface */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProceduralMeshComponent* RoadMesh;

	/** PCG component for road decoration (UE5) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPCGComponent* PCGComponent;

	// ========== Road Parameters ==========

	/** Width of the road in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Parameters", meta = (ClampMin = "200.0", ClampMax = "2000.0"))
	float RoadWidth;

	/** Thickness of the road mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Parameters", meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float RoadThickness;

	/** Height offset above terrain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Parameters", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float HeightOffset;

	/** Number of segments per spline section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Parameters", meta = (ClampMin = "1", ClampMax = "20"))
	int32 SegmentsPerSection;

	/** Enable banking on curves */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Parameters")
	bool bEnableBanking;

	/** Maximum banking angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Parameters", meta = (EditCondition = "bEnableBanking", ClampMin = "0.0", ClampMax = "45.0"))
	float MaxBankingAngle;

	// ========== Pathfinding Settings ==========

	/** Points of interest to connect with roads */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Pathfinding")
	TArray<FPointOfInterest> PointsOfInterest;

	/** Step size for A* pathfinding in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Pathfinding", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float PathfindingStepSize;

	/** Maximum slope for roads in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Pathfinding", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float MaxSlope;

	/** Penalty multiplier for steep slopes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Pathfinding", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float SlopePenalty;

	/** Cost multiplier based on biome type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Pathfinding")
	TMap<EBiomeType, float> BiomeCostMultipliers;

	/** Prefer existing roads (lower cost when near existing paths) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Pathfinding")
	bool bPreferExistingRoads;

	/** Distance threshold for road merging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Pathfinding", meta = (EditCondition = "bPreferExistingRoads"))
	float RoadMergeDistance;

	// ========== Terrain Integration ==========

	/** Reference to landscape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Terrain")
	ALandscape* TargetLandscape;

	/** Flatten terrain under road */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Terrain")
	bool bFlattenTerrain;

	/** Flattening radius around road center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Terrain", meta = (EditCondition = "bFlattenTerrain"))
	float FlatteningRadius;

	/** Smoothness of terrain transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Terrain", meta = (EditCondition = "bFlattenTerrain"))
	float TerrainSmoothness;

	// ========== Road Network ==========

	/** Generated road segments */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road|Network")
	TArray<FRoadSegment> RoadSegments;

	/** Road intersections */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road|Network")
	TArray<FRoadIntersection> Intersections;

	/** Spline components for each road segment */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road|Network")
	TArray<USplineComponent*> RoadSplines;

	// ========== Materials ==========

	/** Road surface material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Materials")
	UMaterialInterface* RoadMaterial;

	/** Intersection material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Materials")
	UMaterialInterface* IntersectionMaterial;

	// ========== PCG Integration ==========

	/** Enable PCG decoration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|PCG")
	bool bEnablePCGDecoration;

	/** PCG graph for road decoration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|PCG", meta = (EditCondition = "bEnablePCGDecoration"))
	TSoftObjectPtr<class UPCGGraphInterface> PCGGraph;

	// ========== Generation Methods ==========

	/**
	 * Generate complete road network connecting all POIs
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	void GenerateRoadNetwork();

	/**
	 * Generate single road between two points
	 * @param StartPoint Starting position
	 * @param EndPoint Ending position
	 * @return Road segment index
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	int32 GenerateRoadSegment(const FVector& StartPoint, const FVector& EndPoint);

	/**
	 * Find optimal path using A* with terrain-aware cost function
	 * @param Start Starting position
	 * @param End Ending position
	 * @return Array of path points
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Pathfinding")
	TArray<FVector> FindOptimalPath(const FVector& Start, const FVector& End);

	/**
	 * Calculate movement cost between two points
	 * @param From Current position
	 * @param To Target position
	 * @return Movement cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Pathfinding")
	float CalculateMovementCost(const FVector& From, const FVector& To);

	/**
	 * Generate road mesh along spline
	 * @param RoadSpline Spline to follow
	 * @param SegmentIndex Segment index for mesh generation
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	void GenerateRoadMesh(USplineComponent* RoadSpline, int32 SegmentIndex);

	/**
	 * Detect and create intersections
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	void DetectIntersections();

	/**
	 * Create intersection mesh at location
	 * @param Location Intersection location
	 * @param IntersectionType Type of intersection
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	void CreateIntersectionMesh(const FVector& Location, int32 IntersectionType);

	/**
	 * Flatten terrain along road path
	 * @param RoadSpline Spline to flatten along
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	void FlattenTerrainAlongRoad(USplineComponent* RoadSpline);

	/**
	 * Clear all generated roads
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Generation")
	void ClearRoads();

	/**
	 * Get terrain height at position
	 * @param WorldPosition World position
	 * @return Terrain height
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Utilities")
	float GetTerrainHeight(const FVector& WorldPosition) const;

	/**
	 * Get terrain slope at position
	 * @param WorldPosition World position
	 * @return Slope in degrees
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Utilities")
	float GetTerrainSlope(const FVector& WorldPosition) const;

	/**
	 * Get biome type at position
	 * @param WorldPosition World position
	 * @return Biome type
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Utilities")
	EBiomeType GetBiomeAtPosition(const FVector& WorldPosition) const;

	/**
	 * Check if two road segments intersect
	 * @param Segment1 First segment
	 * @param Segment2 Second segment
	 * @param OutIntersection Intersection location if found
	 * @return True if segments intersect
	 */
	UFUNCTION(BlueprintCallable, Category = "Road|Utilities")
	bool CheckSegmentIntersection(const FRoadSegment& Segment1, const FRoadSegment& Segment2, FVector& OutIntersection);

private:
	/**
	 * A* pathfinding implementation with terrain costs
	 */
	TArray<FVector> FindPathAStar(const FVector& Start, const FVector& End);

	/**
	 * Get neighbor positions for pathfinding
	 */
	TArray<FVector> GetNeighborPositions(const FVector& Position) const;

	/**
	 * Calculate heuristic for A*
	 */
	float CalculateHeuristic(const FVector& From, const FVector& To) const;

	/**
	 * Check if position is valid for road placement
	 */
	bool IsValidRoadPosition(const FVector& Position) const;

	/**
	 * Smooth road path
	 */
	TArray<FVector> SmoothRoadPath(const TArray<FVector>& Path, int32 Iterations = 3) const;

	/**
	 * Calculate banking angle for curve
	 */
	float CalculateBankingAngle(USplineComponent* Spline, float Distance) const;

	/**
	 * Generate mesh section between two points
	 */
	void GenerateMeshSection(const FVector& Start, const FVector& End, float Width, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs);

	/**
	 * Merge nearby roads
	 */
	void MergeNearbyRoads();

	/**
	 * Find nearest point on existing roads
	 */
	FVector FindNearestRoadPoint(const FVector& Position, float& OutDistance) const;

	/**
	 * Calculate cost multiplier based on biome
	 */
	float GetBiomeCostMultiplier(EBiomeType BiomeType) const;

	// Cache for performance
	TMap<FIntPoint, float> HeightCache;
	TMap<FIntPoint, EBiomeType> BiomeCache;
	bool bNetworkGenerated;
};
