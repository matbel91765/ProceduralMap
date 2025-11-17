// ProceduralFoliageSpawner.cpp - Professional Procedural Foliage Spawning System
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#include "ProceduralFoliageSpawner.h"
#include "FoliageDistribution.h"
#include "VegetationLayer.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AProceduralFoliageSpawner::AProceduralFoliageSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Set root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	// Initialize default distribution algorithm
	DistributionAlgorithm = CreateDefaultSubobject<UFoliageDistribution>(TEXT("DistributionAlgorithm"));
}

void AProceduralFoliageSpawner::BeginPlay()
{
	Super::BeginPlay();

	// Initialize random stream
	RandomStream.Initialize(RandomSeed);

	if (bSpawnOnBeginPlay)
	{
		SpawnFoliageOnTerrain(true);
	}
}

void AProceduralFoliageSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#if WITH_EDITOR
void AProceduralFoliageSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bAutoUpdate && PropertyChangedEvent.Property != nullptr)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName != GET_MEMBER_NAME_CHECKED(AProceduralFoliageSpawner, bDebugDraw) &&
			PropertyName != GET_MEMBER_NAME_CHECKED(AProceduralFoliageSpawner, bAutoUpdate))
		{
			// Regenerate foliage
			SpawnFoliageOnTerrain(true);
		}
	}
}
#endif

FFoliageSpawnResult AProceduralFoliageSpawner::SpawnFoliageOnTerrain(bool bClearExisting)
{
	FVector Center = GetActorLocation();
	return SpawnFoliageInRadius(Center, SpawnRadius, bClearExisting);
}

FFoliageSpawnResult AProceduralFoliageSpawner::SpawnFoliageInRadius(FVector Center, float Radius, bool bClearExisting)
{
	// Create bounds from radius
	FBox Bounds(Center - FVector(Radius), Center + FVector(Radius));
	return SpawnFoliageInBounds(Bounds, bClearExisting);
}

FFoliageSpawnResult AProceduralFoliageSpawner::SpawnFoliageInBounds(FBox Bounds, bool bClearExisting)
{
	double StartTime = FPlatformTime::Seconds();

	FFoliageSpawnResult Result;
	Result.InstancesSpawned = 0;
	Result.SpawnAttempts = 0;

	if (bClearExisting)
	{
		ClearAllFoliage();
	}

	// Create HISM components if needed
	CreateHISMComponents();

	// Load foliage types from vegetation layer if available
	TArray<FFoliageTypeConfig> ActiveFoliageTypes = FoliageTypes;
	if (VegetationLayer && VegetationLayer->VegetationTypes.Num() > 0)
	{
		// Convert vegetation types to foliage types
		ActiveFoliageTypes.Empty();
		for (const FVegetationTypeConfig& VegType : VegetationLayer->VegetationTypes)
		{
			FFoliageTypeConfig FoliageType;
			FoliageType.StaticMesh = VegType.StaticMesh;
			FoliageType.Density = VegType.BaseDensity * VegetationLayer->LayerDensityMultiplier;
			FoliageType.MinScale = VegType.MinScale;
			FoliageType.MaxScale = VegType.MaxScale;
			FoliageType.bRandomRotation = VegType.bRandomRotation;
			FoliageType.bAlignToNormal = VegType.bAlignToNormal;
			FoliageType.MaxAlignmentAngle = VegType.MaxAlignmentAngle;
			FoliageType.BiomeMask = VegType.AllowedBiomes;
			FoliageType.MinHeight = VegType.MinHeight;
			FoliageType.MaxHeight = VegType.MaxHeight;
			FoliageType.MinSlope = VegType.MinSlope;
			FoliageType.MaxSlope = VegType.MaxSlope;
			FoliageType.SpawnWeight = VegType.SpawnWeight;
			ActiveFoliageTypes.Add(FoliageType);
		}
	}

	if (ActiveFoliageTypes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProceduralFoliageSpawner: No foliage types configured"));
		return Result;
	}

	// Spawn each foliage type
	for (int32 TypeIndex = 0; TypeIndex < ActiveFoliageTypes.Num(); ++TypeIndex)
	{
		const FFoliageTypeConfig& FoliageType = ActiveFoliageTypes[TypeIndex];

		if (!FoliageType.StaticMesh)
		{
			continue;
		}

		// Generate spawn points using distribution algorithm
		TArray<FSpawnPoint> SpawnPoints;
		if (DistributionAlgorithm)
		{
			DistributionAlgorithm->GenerateSpawnPoints(Bounds, FoliageType.Density, RandomStream, SpawnPoints);
		}
		else
		{
			// Fallback to simple random distribution
			int32 NumInstances = FMath::RoundToInt(FoliageType.Density * Bounds.GetSize().X * Bounds.GetSize().Y / 10000.0f);
			for (int32 i = 0; i < NumInstances; ++i)
			{
				FVector RandomLocation = FVector(
					RandomStream.FRandRange(Bounds.Min.X, Bounds.Max.X),
					RandomStream.FRandRange(Bounds.Min.Y, Bounds.Max.Y),
					RandomStream.FRandRange(Bounds.Min.Z, Bounds.Max.Z)
				);
				SpawnPoints.Add(FSpawnPoint(RandomLocation, FVector::UpVector, TypeIndex));
			}
		}

		// Process each spawn point
		for (FSpawnPoint& SpawnPoint : SpawnPoints)
		{
			Result.SpawnAttempts++;

			// Trace to ground
			FVector HitLocation, HitNormal;
			if (!TraceGround(SpawnPoint.Location, HitLocation, HitNormal))
			{
				if (bDebugDraw)
				{
					DebugDrawSpawnPoint(SpawnPoint.Location, HitNormal, false);
				}
				continue;
			}

			// Check spawn conditions
			if (!CheckSpawnConditions(HitLocation, HitNormal, FoliageType))
			{
				if (bDebugDraw)
				{
					DebugDrawSpawnPoint(HitLocation, HitNormal, false);
				}
				continue;
			}

			// Apply density calculation
			float DensityMultiplier = CalculateFoliageDensity(HitLocation, FoliageType);
			if (RandomStream.FRand() > DensityMultiplier)
			{
				continue;
			}

			// Generate transform
			FVector Scale = GenerateRandomScale(FoliageType);
			FRotator Rotation = GenerateRotation(HitNormal, FoliageType);
			FTransform Transform(Rotation, HitLocation, Scale);

			// Add instance
			AddFoliageInstance(TypeIndex, Transform);
			Result.InstancesSpawned++;

			if (bDebugDraw)
			{
				DebugDrawSpawnPoint(HitLocation, HitNormal, true);
			}
		}
	}

	// Update culling distances
	UpdateCullingDistances();

	// Calculate statistics
	double EndTime = FPlatformTime::Seconds();
	Result.SpawnTime = static_cast<float>(EndTime - StartTime);
	Result.SuccessRate = Result.SpawnAttempts > 0 ?
		static_cast<float>(Result.InstancesSpawned) / static_cast<float>(Result.SpawnAttempts) : 0.0f;

	LastSpawnResult = Result;

	UE_LOG(LogTemp, Log, TEXT("ProceduralFoliageSpawner: Spawned %d instances (%.1f%% success) in %.2fs"),
		Result.InstancesSpawned, Result.SuccessRate * 100.0f, Result.SpawnTime);

	return Result;
}

void AProceduralFoliageSpawner::ClearAllFoliage()
{
	for (auto& Pair : HISMComponents)
	{
		for (UHierarchicalInstancedStaticMeshComponent* Component : Pair.Value)
		{
			if (Component)
			{
				Component->ClearInstances();
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ProceduralFoliageSpawner: Cleared all foliage instances"));
}

float AProceduralFoliageSpawner::CalculateFoliageDensity_Implementation(const FVector& Location, const FFoliageTypeConfig& FoliageType) const
{
	// Base density multiplier (can be overridden in Blueprint)
	return 1.0f;
}

bool AProceduralFoliageSpawner::CheckSpawnConditions_Implementation(const FVector& Location, const FVector& Normal, const FFoliageTypeConfig& FoliageType) const
{
	// Check height constraints
	if (!CheckHeightConstraints(Location, FoliageType))
	{
		return false;
	}

	// Check slope constraints
	if (!CheckSlopeConstraints(Normal, FoliageType))
	{
		return false;
	}

	// Check biome constraints
	if (!CheckBiomeConstraints(Location, FoliageType))
	{
		return false;
	}

	// Check water distance constraints
	if (!CheckWaterDistanceConstraints(Location, FoliageType))
	{
		return false;
	}

	return true;
}

int32 AProceduralFoliageSpawner::GetTotalInstanceCount() const
{
	int32 TotalCount = 0;
	for (const auto& Pair : HISMComponents)
	{
		for (UHierarchicalInstancedStaticMeshComponent* Component : Pair.Value)
		{
			if (Component)
			{
				TotalCount += Component->GetInstanceCount();
			}
		}
	}
	return TotalCount;
}

int32 AProceduralFoliageSpawner::GetInstanceCountForType(int32 FoliageTypeIndex) const
{
	int32 Count = 0;
	const TArray<UHierarchicalInstancedStaticMeshComponent*>* Components = HISMComponents.Find(FoliageTypeIndex);
	if (Components)
	{
		for (UHierarchicalInstancedStaticMeshComponent* Component : *Components)
		{
			if (Component)
			{
				Count += Component->GetInstanceCount();
			}
		}
	}
	return Count;
}

void AProceduralFoliageSpawner::UpdateCullingDistances()
{
	for (auto& Pair : HISMComponents)
	{
		int32 TypeIndex = Pair.Key;
		const FFoliageTypeConfig& FoliageType = FoliageTypes.IsValidIndex(TypeIndex) ? FoliageTypes[TypeIndex] : FFoliageTypeConfig();

		for (UHierarchicalInstancedStaticMeshComponent* Component : Pair.Value)
		{
			if (Component)
			{
				float CullDist = FoliageType.CullDistance > 0.0f ? FoliageType.CullDistance : DefaultCullDistance;
				Component->SetCullDistances(0, CullDist);
			}
		}
	}
}

void AProceduralFoliageSpawner::CreateHISMComponents()
{
	// Components are created on-demand in GetOrCreateHISMComponent
}

UHierarchicalInstancedStaticMeshComponent* AProceduralFoliageSpawner::GetOrCreateHISMComponent(int32 FoliageTypeIndex, int32 ComponentIndex)
{
	TArray<UHierarchicalInstancedStaticMeshComponent*>& Components = HISMComponents.FindOrAdd(FoliageTypeIndex);

	// Ensure array is large enough
	while (Components.Num() <= ComponentIndex)
	{
		FString ComponentName = FString::Printf(TEXT("FoliageHISM_%d_%d"), FoliageTypeIndex, Components.Num());
		UHierarchicalInstancedStaticMeshComponent* NewComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			this,
			*ComponentName
		);

		if (NewComponent)
		{
			NewComponent->SetupAttachment(GetRootComponent());
			NewComponent->RegisterComponent();

			// Configure component
			if (FoliageTypes.IsValidIndex(FoliageTypeIndex))
			{
				const FFoliageTypeConfig& FoliageType = FoliageTypes[FoliageTypeIndex];
				NewComponent->SetStaticMesh(FoliageType.StaticMesh);
				NewComponent->SetCollisionEnabled(FoliageType.bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
				NewComponent->SetCastShadow(FoliageType.bCastShadow);

				float CullDist = FoliageType.CullDistance > 0.0f ? FoliageType.CullDistance : DefaultCullDistance;
				NewComponent->SetCullDistances(0, CullDist);
			}

			Components.Add(NewComponent);
		}
	}

	return Components.IsValidIndex(ComponentIndex) ? Components[ComponentIndex] : nullptr;
}

bool AProceduralFoliageSpawner::TraceGround(const FVector& Location, FVector& OutHitLocation, FVector& OutHitNormal) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector TraceStart = FVector(Location.X, Location.Y, Location.Z + 1000.0f);
	FVector TraceEnd = FVector(Location.X, Location.Y, Location.Z - 1000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		OutHitLocation = HitResult.Location;
		OutHitNormal = HitResult.Normal;
		return true;
	}

	return false;
}

bool AProceduralFoliageSpawner::CheckHeightConstraints(const FVector& Location, const FFoliageTypeConfig& FoliageType) const
{
	return Location.Z >= FoliageType.MinHeight && Location.Z <= FoliageType.MaxHeight;
}

bool AProceduralFoliageSpawner::CheckSlopeConstraints(const FVector& Normal, const FFoliageTypeConfig& FoliageType) const
{
	float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
	return SlopeAngle >= FoliageType.MinSlope && SlopeAngle <= FoliageType.MaxSlope;
}

bool AProceduralFoliageSpawner::CheckBiomeConstraints(const FVector& Location, const FFoliageTypeConfig& FoliageType) const
{
	// If no biome mask, allow all
	if (FoliageType.BiomeMask.Num() == 0)
	{
		return true;
	}

	// TODO: Implement biome checking with BiomeGenerator
	return true;
}

bool AProceduralFoliageSpawner::CheckWaterDistanceConstraints(const FVector& Location, const FFoliageTypeConfig& FoliageType) const
{
	if (FoliageType.MinDistanceFromWater <= 0.0f && FoliageType.MaxDistanceFromWater <= 0.0f)
	{
		return true;
	}

	// TODO: Implement water distance checking
	return true;
}

FVector AProceduralFoliageSpawner::GenerateRandomScale(const FFoliageTypeConfig& FoliageType) const
{
	float Scale = RandomStream.FRandRange(FoliageType.MinScale, FoliageType.MaxScale);
	return FVector(Scale);
}

FRotator AProceduralFoliageSpawner::GenerateRotation(const FVector& Normal, const FFoliageTypeConfig& FoliageType) const
{
	FRotator Rotation = FRotator::ZeroRotator;

	// Random Z rotation
	if (FoliageType.bRandomRotation)
	{
		Rotation.Yaw = RandomStream.FRandRange(0.0f, 360.0f);
	}

	// Align to normal
	if (FoliageType.bAlignToNormal)
	{
		FVector AlignedZ = Normal;
		FVector AlignedX = FVector::CrossProduct(FVector::RightVector, AlignedZ).GetSafeNormal();
		if (AlignedX.IsNearlyZero())
		{
			AlignedX = FVector::CrossProduct(FVector::ForwardVector, AlignedZ).GetSafeNormal();
		}
		FVector AlignedY = FVector::CrossProduct(AlignedZ, AlignedX);

		FMatrix AlignMatrix = FMatrix(AlignedX, AlignedY, AlignedZ, FVector::ZeroVector);
		FRotator AlignRotation = AlignMatrix.Rotator();

		// Blend between aligned and upright based on max alignment angle
		float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
		float AlignmentBlend = FMath::Clamp(SlopeAngle / FoliageType.MaxAlignmentAngle, 0.0f, 1.0f);

		Rotation.Pitch = FMath::Lerp(0.0f, AlignRotation.Pitch, AlignmentBlend);
		Rotation.Roll = FMath::Lerp(0.0f, AlignRotation.Roll, AlignmentBlend);
	}

	return Rotation;
}

void AProceduralFoliageSpawner::AddFoliageInstance(int32 FoliageTypeIndex, const FTransform& Transform)
{
	// Find component with space
	int32 ComponentIndex = 0;
	UHierarchicalInstancedStaticMeshComponent* Component = GetOrCreateHISMComponent(FoliageTypeIndex, ComponentIndex);

	while (Component && Component->GetInstanceCount() >= MaxInstancesPerComponent)
	{
		ComponentIndex++;
		Component = GetOrCreateHISMComponent(FoliageTypeIndex, ComponentIndex);
	}

	if (Component)
	{
		Component->AddInstance(Transform);
	}
}

void AProceduralFoliageSpawner::DebugDrawSpawnPoint(const FVector& Location, const FVector& Normal, bool bSuccess) const
{
	if (!bDebugDraw)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FColor Color = bSuccess ? FColor::Green : FColor::Red;
	DrawDebugSphere(World, Location, 50.0f, 8, Color, false, DebugDrawDuration);
	DrawDebugLine(World, Location, Location + Normal * 100.0f, FColor::Blue, false, DebugDrawDuration);
}
