// FoliageCulling.cpp - Foliage Culling and Optimization System
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#include "FoliageCulling.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UFoliageCulling::UFoliageCulling()
{
	CullingStrategy = ECullingStrategy::Combined;
	bEnableDistanceCulling = true;
	DefaultCullDistance = 20000.0f;
	CullDistanceFadeRange = 1000.0f;
	bEnableFrustumCulling = true;
	FrustumCullingMargin = 500.0f;
	bEnableOcclusionHints = false;
	OcclusionQueryDistance = 10000.0f;
	bEnableDynamicLOD = false;
	TargetFrameTime = 16.67f;
	UpdateFrequency = 0.1f;
	bEnableSpatialPartitioning = true;
	PartitionCellSize = 10000.0f;
	MaxInstancesPerFrame = 1000;
	bEnableStats = false;
	bDrawDebug = false;
	LastUpdateTime = 0.0f;
	AccumulatedDeltaTime = 0.0f;

	FrameTimeHistory.Reserve(MaxFrameTimeHistory);
}

void UFoliageCulling::UpdateCulling(
	UHierarchicalInstancedStaticMeshComponent* HISMComponent,
	const FVector& ViewLocation,
	const FRotator& ViewRotation,
	float DeltaTime
)
{
	if (!HISMComponent || !HISMComponent->GetStaticMesh())
	{
		return;
	}

	AccumulatedDeltaTime += DeltaTime;

	// Check if we should update based on frequency
	if (AccumulatedDeltaTime < UpdateFrequency)
	{
		return;
	}

	float UpdateDeltaTime = AccumulatedDeltaTime;
	AccumulatedDeltaTime = 0.0f;

	double StartTime = FPlatformTime::Seconds();

	// Reset stats
	if (bEnableStats)
	{
		CullingStats.Reset();
		CullingStats.TotalInstances = HISMComponent->GetInstanceCount();
	}

	// Build spatial partition if needed
	if (bEnableSpatialPartitioning && !SpatialPartitions.Contains(HISMComponent))
	{
		BuildSpatialPartition(HISMComponent);
	}

	// Update distance culling
	if (bEnableDistanceCulling && (CullingStrategy == ECullingStrategy::Distance || CullingStrategy == ECullingStrategy::Combined || CullingStrategy == ECullingStrategy::Advanced))
	{
		UpdateDistanceCulling(HISMComponent, ViewLocation);
	}

	// Update frustum culling
	if (bEnableFrustumCulling && (CullingStrategy == ECullingStrategy::Frustum || CullingStrategy == ECullingStrategy::Combined || CullingStrategy == ECullingStrategy::Advanced))
	{
		UpdateFrustumCulling(HISMComponent, ViewLocation, ViewRotation);
	}

	// Update LOD levels
	UpdateLODLevels(HISMComponent, ViewLocation);

	// Draw debug visualization
	if (bDrawDebug)
	{
		DrawDebugCulling(HISMComponent, ViewLocation);
	}

	// Update stats
	if (bEnableStats)
	{
		double EndTime = FPlatformTime::Seconds();
		CullingStats.LastUpdateTime = static_cast<float>(EndTime - StartTime);
		LastUpdateTime = static_cast<float>(EndTime);
	}

	// Track frame time for dynamic LOD
	if (bEnableDynamicLOD)
	{
		FrameTimeHistory.Add(UpdateDeltaTime * 1000.0f); // Convert to ms
		if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
		{
			FrameTimeHistory.RemoveAt(0);
		}

		// Adjust LOD bias based on performance
		float AverageFrameTime = 0.0f;
		for (float FrameTime : FrameTimeHistory)
		{
			AverageFrameTime += FrameTime;
		}
		AverageFrameTime /= FrameTimeHistory.Num();

		float OptimalBias = CalculateOptimalLODBias(AverageFrameTime);
		LODSettings.LODBias = OptimalBias;
	}
}

void UFoliageCulling::UpdateCullingBatch(
	const TArray<UHierarchicalInstancedStaticMeshComponent*>& HISMComponents,
	const FVector& ViewLocation,
	const FRotator& ViewRotation,
	float DeltaTime
)
{
	for (UHierarchicalInstancedStaticMeshComponent* Component : HISMComponents)
	{
		UpdateCulling(Component, ViewLocation, ViewRotation, DeltaTime);
	}
}

void UFoliageCulling::SetCullDistance(UHierarchicalInstancedStaticMeshComponent* HISMComponent, float Distance)
{
	if (!HISMComponent)
	{
		return;
	}

	CustomCullDistances.Add(HISMComponent, Distance);
	HISMComponent->SetCullDistances(0, Distance);
}

void UFoliageCulling::SetLODDistances(
	UHierarchicalInstancedStaticMeshComponent* HISMComponent,
	const FFoliageLODSettings& InLODSettings
)
{
	if (!HISMComponent)
	{
		return;
	}

	CustomLODSettings.Add(HISMComponent, InLODSettings);
}

float UFoliageCulling::CalculateOptimalLODBias(float CurrentFrameTime)
{
	float Bias = 0.0f;

	if (CurrentFrameTime > TargetFrameTime * 1.2f)
	{
		// Performance is bad, increase LOD bias (lower quality)
		Bias = FMath::Clamp((CurrentFrameTime - TargetFrameTime) / TargetFrameTime, 0.0f, 3.0f);
	}
	else if (CurrentFrameTime < TargetFrameTime * 0.8f)
	{
		// Performance is good, decrease LOD bias (higher quality)
		Bias = FMath::Clamp((CurrentFrameTime - TargetFrameTime) / TargetFrameTime, -3.0f, 0.0f);
	}

	return Bias;
}

void UFoliageCulling::ResetStats()
{
	CullingStats.Reset();
	FrameTimeHistory.Empty();
}

bool UFoliageCulling::ShouldCullByDistance(const FVector& InstanceLocation, const FVector& ViewLocation, float CullDistance) const
{
	float DistanceSq = FVector::DistSquared(InstanceLocation, ViewLocation);
	float CullDistSq = CullDistance * CullDistance;

	return DistanceSq > CullDistSq;
}

bool UFoliageCulling::ShouldCullByFrustum(const FVector& InstanceLocation, const FVector& ViewLocation, const FRotator& ViewRotation) const
{
	// Simple frustum check using view direction and FOV
	FVector ViewDir = ViewRotation.Vector();
	FVector ToInstance = (InstanceLocation - ViewLocation).GetSafeNormal();

	float DotProduct = FVector::DotProduct(ViewDir, ToInstance);

	// Assume 90 degree FOV, so cull if dot product < -0.2 (behind and outside extended frustum)
	return DotProduct < -0.2f;
}

int32 UFoliageCulling::GetLODLevel(const FVector& InstanceLocation, const FVector& ViewLocation) const
{
	float Distance = FVector::Dist(InstanceLocation, ViewLocation);
	return LODSettings.GetLODForDistance(Distance);
}

void UFoliageCulling::SetCullingEnabled(bool bEnabled)
{
	bEnableDistanceCulling = bEnabled;
	bEnableFrustumCulling = bEnabled;
}

int32 UFoliageCulling::GetTotalInstanceCount() const
{
	return CullingStats.TotalInstances;
}

int32 UFoliageCulling::GetVisibleInstanceCount() const
{
	return CullingStats.VisibleInstances;
}

void UFoliageCulling::UpdateDistanceCulling(
	UHierarchicalInstancedStaticMeshComponent* HISMComponent,
	const FVector& ViewLocation
)
{
	// Get cull distance for this component
	float CullDistance = DefaultCullDistance;
	if (CustomCullDistances.Contains(HISMComponent))
	{
		CullDistance = CustomCullDistances[HISMComponent];
	}

	// Note: HISM components handle distance culling automatically via SetCullDistances
	// This is more for statistics tracking

	int32 InstanceCount = HISMComponent->GetInstanceCount();
	float CullDistSq = CullDistance * CullDistance;

	for (int32 i = 0; i < InstanceCount && i < MaxInstancesPerFrame; ++i)
	{
		FTransform InstanceTransform;
		if (HISMComponent->GetInstanceTransform(i, InstanceTransform, true))
		{
			float DistSq = FVector::DistSquared(InstanceTransform.GetLocation(), ViewLocation);

			if (DistSq > CullDistSq)
			{
				if (bEnableStats)
				{
					CullingStats.DistanceCulled++;
				}
			}
			else
			{
				if (bEnableStats)
				{
					CullingStats.VisibleInstances++;
				}
			}
		}
	}
}

void UFoliageCulling::UpdateFrustumCulling(
	UHierarchicalInstancedStaticMeshComponent* HISMComponent,
	const FVector& ViewLocation,
	const FRotator& ViewRotation
)
{
	// HISM components handle frustum culling automatically
	// This is primarily for statistics

	FVector ViewDir = ViewRotation.Vector();
	int32 InstanceCount = HISMComponent->GetInstanceCount();

	for (int32 i = 0; i < InstanceCount && i < MaxInstancesPerFrame; ++i)
	{
		FTransform InstanceTransform;
		if (HISMComponent->GetInstanceTransform(i, InstanceTransform, true))
		{
			if (ShouldCullByFrustum(InstanceTransform.GetLocation(), ViewLocation, ViewRotation))
			{
				if (bEnableStats)
				{
					CullingStats.FrustumCulled++;
				}
			}
		}
	}
}

void UFoliageCulling::UpdateLODLevels(
	UHierarchicalInstancedStaticMeshComponent* HISMComponent,
	const FVector& ViewLocation
)
{
	// Get LOD settings for this component
	FFoliageLODSettings ActiveLODSettings = LODSettings;
	if (CustomLODSettings.Contains(HISMComponent))
	{
		ActiveLODSettings = CustomLODSettings[HISMComponent];
	}

	// Track LOD distribution for stats
	if (bEnableStats)
	{
		int32 InstanceCount = HISMComponent->GetInstanceCount();

		for (int32 i = 0; i < InstanceCount && i < MaxInstancesPerFrame; ++i)
		{
			FTransform InstanceTransform;
			if (HISMComponent->GetInstanceTransform(i, InstanceTransform, true))
			{
				int32 LODLevel = ActiveLODSettings.GetLODForDistance(
					FVector::Dist(InstanceTransform.GetLocation(), ViewLocation)
				);

				switch (LODLevel)
				{
				case 0: CullingStats.LOD0Count++; break;
				case 1: CullingStats.LOD1Count++; break;
				case 2: CullingStats.LOD2Count++; break;
				case 3: CullingStats.LOD3Count++; break;
				}
			}
		}
	}
}

void UFoliageCulling::BuildSpatialPartition(UHierarchicalInstancedStaticMeshComponent* HISMComponent)
{
	FSpatialPartition& Partition = SpatialPartitions.FindOrAdd(HISMComponent);
	Partition.CellSize = PartitionCellSize;
	Partition.Clear();

	int32 InstanceCount = HISMComponent->GetInstanceCount();

	for (int32 i = 0; i < InstanceCount; ++i)
	{
		FTransform InstanceTransform;
		if (HISMComponent->GetInstanceTransform(i, InstanceTransform, true))
		{
			FIntVector CellIndex = Partition.GetCellIndex(InstanceTransform.GetLocation());
			Partition.InstanceGrid.FindOrAdd(CellIndex).Add(i);
		}
	}
}

void UFoliageCulling::DrawDebugCulling(
	UHierarchicalInstancedStaticMeshComponent* HISMComponent,
	const FVector& ViewLocation
)
{
	if (!HISMComponent || !HISMComponent->GetWorld())
	{
		return;
	}

	UWorld* World = HISMComponent->GetWorld();

	// Draw view location
	DrawDebugSphere(World, ViewLocation, 100.0f, 12, FColor::Yellow, false, 0.1f);

	// Draw cull distance
	float CullDistance = DefaultCullDistance;
	if (CustomCullDistances.Contains(HISMComponent))
	{
		CullDistance = CustomCullDistances[HISMComponent];
	}

	DrawDebugCircle(World, ViewLocation, CullDistance, 64, FColor::Red, false, 0.1f, 0, 10.0f,
		FVector::ForwardVector, FVector::RightVector, false);

	// Draw LOD rings
	FFoliageLODSettings ActiveLODSettings = LODSettings;
	if (CustomLODSettings.Contains(HISMComponent))
	{
		ActiveLODSettings = CustomLODSettings[HISMComponent];
	}

	DrawDebugCircle(World, ViewLocation, ActiveLODSettings.LOD1Distance, 32, FColor::Green, false, 0.1f);
	DrawDebugCircle(World, ViewLocation, ActiveLODSettings.LOD2Distance, 32, FColor::Blue, false, 0.1f);
	DrawDebugCircle(World, ViewLocation, ActiveLODSettings.LOD3Distance, 32, FColor::Cyan, false, 0.1f);
}
