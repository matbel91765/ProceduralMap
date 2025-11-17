// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "LODManager.h"
#include "Engine/World.h"

// Initialize static member
TMap<UWorld*, ULODManager*> ULODManager::ManagerInstances;

ULODManager::ULODManager()
	: BaseLODDistanceMultiplier(2.0f)
	, HysteresisFactor(1.3f)
	, MinUpdateInterval(0.1f)
	, MaxSplitsPerFrame(10)
	, MaxMergesPerFrame(10)
	, bEnableFrustumCulling(true)
	, bEnableDistanceCulling(true)
	, MaxRenderDistance(50000.0f)
	, bEnableStatistics(true)
	, bShowDebugVisualization(false)
	, FrameStartTime(0.0)
{
	// Initialize debug colors for different LOD levels
	LODDebugColors.Empty();
	LODDebugColors.Add(FColor::Red);      // LOD 0 - Highest detail
	LODDebugColors.Add(FColor::Orange);   // LOD 1
	LODDebugColors.Add(FColor::Yellow);   // LOD 2
	LODDebugColors.Add(FColor::Green);    // LOD 3
	LODDebugColors.Add(FColor::Cyan);     // LOD 4
	LODDebugColors.Add(FColor::Blue);     // LOD 5
	LODDebugColors.Add(FColor::Magenta);  // LOD 6
	LODDebugColors.Add(FColor::White);    // LOD 7+

	InitializeDefaultBiomes();
}

float ULODManager::CalculateSplitDistance(int32 LODLevel, float NodeSize, FName BiomeName) const
{
	// Get biome-specific multiplier
	float BiomeMultiplier = BaseLODDistanceMultiplier;

	if (BiomeName != NAME_None)
	{
		FBiomeLODConfig Config = GetBiomeConfig(BiomeName);
		BiomeMultiplier *= Config.DistanceMultiplier;
	}

	// Distance = NodeSize * Multiplier * (2^LODLevel)
	// Higher LOD = further away before splitting
	float Distance = NodeSize * BiomeMultiplier * FMath::Pow(2.0f, static_cast<float>(LODLevel));

	return Distance;
}

float ULODManager::CalculateMergeDistance(int32 LODLevel, float NodeSize, FName BiomeName) const
{
	// Merge distance is split distance multiplied by hysteresis factor
	// This prevents rapid split/merge flickering
	float SplitDist = CalculateSplitDistance(LODLevel, NodeSize, BiomeName);
	return SplitDist * HysteresisFactor;
}

FBiomeLODConfig ULODManager::GetBiomeConfig(FName BiomeName) const
{
	// Search for biome configuration
	for (const FBiomeLODConfig& Config : BiomeConfigs)
	{
		if (Config.BiomeName == BiomeName)
		{
			return Config;
		}
	}

	// Return default configuration
	return FBiomeLODConfig();
}

bool ULODManager::ShouldSplitNode(float CameraDistance, int32 LODLevel, float NodeSize,
								  float LastUpdateTime, float CurrentTime, FName BiomeName) const
{
	// Check minimum update interval to prevent thrashing
	if ((CurrentTime - LastUpdateTime) < MinUpdateInterval)
	{
		return false;
	}

	// Calculate split threshold
	float SplitDistance = CalculateSplitDistance(LODLevel, NodeSize, BiomeName);

	// Split if camera is closer than threshold
	return CameraDistance < SplitDistance;
}

bool ULODManager::ShouldMergeNode(float CameraDistance, int32 LODLevel, float NodeSize,
								  float LastUpdateTime, float CurrentTime, FName BiomeName) const
{
	// Check minimum update interval
	if ((CurrentTime - LastUpdateTime) < MinUpdateInterval)
	{
		return false;
	}

	// Calculate merge threshold (with hysteresis)
	float MergeDistance = CalculateMergeDistance(LODLevel, NodeSize, BiomeName);

	// Merge if camera is further than threshold
	return CameraDistance > MergeDistance;
}

int32 ULODManager::GetMeshResolutionForLOD(int32 LODLevel, int32 BaseMeshResolution, FName BiomeName) const
{
	// Get biome config
	int32 BiomeResolution = BaseMeshResolution;

	if (BiomeName != NAME_None)
	{
		FBiomeLODConfig Config = GetBiomeConfig(BiomeName);
		if (Config.MeshResolution > 0)
		{
			BiomeResolution = Config.MeshResolution;
		}
	}

	// Reduce resolution for higher LOD levels
	// LOD 0: Full resolution
	// LOD 1: Half resolution
	// LOD 2: Quarter resolution, etc.
	int32 Resolution = FMath::Max(4, BiomeResolution >> LODLevel);

	// Ensure power of 2 for better performance
	Resolution = FMath::RoundUpToPowerOfTwo(Resolution);

	return Resolution;
}

void ULODManager::BeginFrame()
{
	if (!bEnableStatistics)
	{
		return;
	}

	// Reset frame statistics
	CurrentStatistics.SplitsThisFrame = 0;
	CurrentStatistics.MergesThisFrame = 0;
	CurrentStatistics.MeshUpdatesThisFrame = 0;

	// Record start time
	FrameStartTime = FPlatformTime::Seconds();
}

void ULODManager::EndFrame(float DeltaTime)
{
	if (!bEnableStatistics)
	{
		return;
	}

	// Calculate frame time
	double FrameEndTime = FPlatformTime::Seconds();
	CurrentStatistics.UpdateTimeMS = static_cast<float>((FrameEndTime - FrameStartTime) * 1000.0);
}

void ULODManager::RecordSplit()
{
	if (bEnableStatistics)
	{
		CurrentStatistics.SplitsThisFrame++;
	}
}

void ULODManager::RecordMerge()
{
	if (bEnableStatistics)
	{
		CurrentStatistics.MergesThisFrame++;
	}
}

void ULODManager::RecordMeshUpdate()
{
	if (bEnableStatistics)
	{
		CurrentStatistics.MeshUpdatesThisFrame++;
	}
}

ULODManager* ULODManager::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	// Check if instance exists for this world
	if (ULODManager** FoundManager = ManagerInstances.Find(World))
	{
		return *FoundManager;
	}

	// Create new instance
	ULODManager* NewManager = NewObject<ULODManager>(GetTransientPackage(), ULODManager::StaticClass());
	if (NewManager)
	{
		NewManager->AddToRoot(); // Prevent garbage collection
		ManagerInstances.Add(World, NewManager);
	}

	return NewManager;
}

void ULODManager::InitializeDefaultBiomes()
{
	BiomeConfigs.Empty();

	// Default biome
	FBiomeLODConfig DefaultBiome;
	DefaultBiome.BiomeName = FName("Default");
	DefaultBiome.DistanceMultiplier = 1.0f;
	DefaultBiome.MaxLODLevel = 6;
	DefaultBiome.MeshResolution = 32;
	DefaultBiome.HeightScale = 1.0f;
	DefaultBiome.bEnableDetailFeatures = true;
	BiomeConfigs.Add(DefaultBiome);

	// Mountains - higher detail, higher terrain
	FBiomeLODConfig MountainBiome;
	MountainBiome.BiomeName = FName("Mountains");
	MountainBiome.DistanceMultiplier = 0.8f; // More aggressive LOD (more detail)
	MountainBiome.MaxLODLevel = 7;
	MountainBiome.MeshResolution = 64;
	MountainBiome.HeightScale = 2.0f;
	MountainBiome.bEnableDetailFeatures = true;
	BiomeConfigs.Add(MountainBiome);

	// Plains - lower detail acceptable
	FBiomeLODConfig PlainsBiome;
	PlainsBiome.BiomeName = FName("Plains");
	PlainsBiome.DistanceMultiplier = 1.5f; // Less aggressive LOD
	PlainsBiome.MaxLODLevel = 5;
	PlainsBiome.MeshResolution = 24;
	PlainsBiome.HeightScale = 0.3f;
	PlainsBiome.bEnableDetailFeatures = true;
	BiomeConfigs.Add(PlainsBiome);

	// Forest - medium detail
	FBiomeLODConfig ForestBiome;
	ForestBiome.BiomeName = FName("Forest");
	ForestBiome.DistanceMultiplier = 1.0f;
	ForestBiome.MaxLODLevel = 6;
	ForestBiome.MeshResolution = 32;
	ForestBiome.HeightScale = 0.8f;
	ForestBiome.bEnableDetailFeatures = true;
	BiomeConfigs.Add(ForestBiome);

	// Desert - can use lower detail
	FBiomeLODConfig DesertBiome;
	DesertBiome.BiomeName = FName("Desert");
	DesertBiome.DistanceMultiplier = 1.8f; // Very relaxed LOD
	DesertBiome.MaxLODLevel = 5;
	DesertBiome.MeshResolution = 20;
	DesertBiome.HeightScale = 0.4f;
	DesertBiome.bEnableDetailFeatures = false; // No foliage
	BiomeConfigs.Add(DesertBiome);

	// Ocean - minimal detail needed
	FBiomeLODConfig OceanBiome;
	OceanBiome.BiomeName = FName("Ocean");
	OceanBiome.DistanceMultiplier = 2.0f; // Very relaxed LOD
	OceanBiome.MaxLODLevel = 4;
	OceanBiome.MeshResolution = 16;
	OceanBiome.HeightScale = 0.1f;
	OceanBiome.bEnableDetailFeatures = false;
	BiomeConfigs.Add(OceanBiome);
}

FColor ULODManager::GetDebugColorForLOD(int32 LODLevel) const
{
	if (LODLevel >= 0 && LODLevel < LODDebugColors.Num())
	{
		return LODDebugColors[LODLevel];
	}

	// Default to white for very high LOD levels
	return FColor::White;
}
