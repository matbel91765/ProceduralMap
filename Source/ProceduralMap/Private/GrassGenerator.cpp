// GrassGenerator.cpp - Specialized Grass and Ground Cover Generator
// Copyright (c) 2025 ProceduralMap. All Rights Reserved.

#include "GrassGenerator.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Landscape.h"

AGrassGenerator::AGrassGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Set root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	// Default settings
	DensityMode = EGrassDensityMode::NoiseVariation;
	BaseDensityMultiplier = 1.0f;
	DensityNoiseScale = 0.01f;
	bGenerateOnBeginPlay = false;
	bAutoUpdate = false;
	GenerationRadius = 10000.0f;
	GridCellSize = 1000.0f;
	RandomSeed = 54321;
	bEnableStreaming = false;
	StreamingDistance = 15000.0f;
	ChunkSize = 5000.0f;
	bEnableGPUInstancing = true;
	MaxInstancesPerComponent = 10000;
	bEnableAsyncGeneration = false;
	bDebugDraw = false;
	bShowStats = false;
	WindTimeAccumulator = 0.0f;
	CurrentWindVector = FVector::ZeroVector;
}

void AGrassGenerator::BeginPlay()
{
	Super::BeginPlay();

	// Initialize random stream
	RandomStream.Initialize(RandomSeed);

	if (bGenerateOnBeginPlay)
	{
		GenerateGrass(true);
	}
}

void AGrassGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update wind animation
	WindTimeAccumulator += DeltaTime;

	for (const auto& Pair : GrassComponents)
	{
		const TArray<UHierarchicalInstancedStaticMeshComponent*>& Components = Pair.Value;
		int32 TypeIndex = Pair.Key;

		if (GrassTypes.IsValidIndex(TypeIndex))
		{
			const FGrassTypeConfig& GrassType = GrassTypes[TypeIndex];

			if (GrassType.WindSettings.bEnableWind)
			{
				// Calculate wind vector
				float WindAngle = FMath::DegreesToRadians(GrassType.WindSettings.WindDirection);
				FVector BaseWind = FVector(
					FMath::Cos(WindAngle),
					FMath::Sin(WindAngle),
					0.0f
				) * GrassType.WindSettings.WindStrength;

				// Add turbulence
				float Turbulence = FMath::Sin(WindTimeAccumulator * GrassType.WindSettings.GustFrequency) *
					GrassType.WindSettings.WindTurbulence;
				CurrentWindVector = BaseWind * (1.0f + Turbulence);

				// Update material parameters
				for (UHierarchicalInstancedStaticMeshComponent* Component : Components)
				{
					if (Component)
					{
						UpdateMaterialWindParameters(Component, GrassType.WindSettings);
					}
				}
			}
		}
	}

	// Update streaming if enabled
	if (bEnableStreaming)
	{
		// Get player camera location
		APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		if (CameraManager)
		{
			UpdateStreaming(CameraManager->GetCameraLocation());
		}
	}

	// Draw debug visualization
	if (bDebugDraw)
	{
		DrawDebugVisualization();
	}
}

void AGrassGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllGrass();
	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void AGrassGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bAutoUpdate && PropertyChangedEvent.Property != nullptr)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName != GET_MEMBER_NAME_CHECKED(AGrassGenerator, bDebugDraw) &&
			PropertyName != GET_MEMBER_NAME_CHECKED(AGrassGenerator, bAutoUpdate))
		{
			// Regenerate grass
			GenerateGrass(true);
		}
	}
}
#endif

FGrassGenerationStats AGrassGenerator::GenerateGrass(bool bClearExisting)
{
	FVector Center = GetActorLocation();
	return GenerateGrassAtLocation(Center, GenerationRadius, bClearExisting);
}

FGrassGenerationStats AGrassGenerator::GenerateGrassInBounds(const FBox& Bounds, bool bClearExisting)
{
	double StartTime = FPlatformTime::Seconds();

	if (bClearExisting)
	{
		ClearAllGrass();
	}

	GenerationStats = FGrassGenerationStats();
	GenerationStats.AreaCovered = (Bounds.GetSize().X * Bounds.GetSize().Y) / 10000.0f; // Square meters

	// Create grass components
	CreateGrassComponents();

	if (GrassTypes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrassGenerator: No grass types configured"));
		return GenerationStats;
	}

	// Calculate grid
	int32 GridX = FMath::CeilToInt(Bounds.GetSize().X / GridCellSize);
	int32 GridY = FMath::CeilToInt(Bounds.GetSize().Y / GridCellSize);

	// Generate grass in each cell
	for (int32 x = 0; x < GridX; ++x)
	{
		for (int32 y = 0; y < GridY; ++y)
		{
			FVector2D CellCenter = FVector2D(
				Bounds.Min.X + (x + 0.5f) * GridCellSize,
				Bounds.Min.Y + (y + 0.5f) * GridCellSize
			);

			// Generate grass for each type in this cell
			for (int32 TypeIndex = 0; TypeIndex < GrassTypes.Num(); ++TypeIndex)
			{
				GenerateGrassInCell(CellCenter, TypeIndex);
			}
		}
	}

	// Calculate generation time
	double EndTime = FPlatformTime::Seconds();
	GenerationStats.GenerationTime = static_cast<float>(EndTime - StartTime);

	UE_LOG(LogTemp, Log, TEXT("GrassGenerator: Generated %d instances in %.2fs (%.1f sq.m)"),
		GenerationStats.TotalInstances, GenerationStats.GenerationTime, GenerationStats.AreaCovered);

	return GenerationStats;
}

FGrassGenerationStats AGrassGenerator::GenerateGrassAtLocation(const FVector& Location, float Radius, bool bClearExisting)
{
	FBox Bounds(Location - FVector(Radius), Location + FVector(Radius));
	return GenerateGrassInBounds(Bounds, bClearExisting);
}

void AGrassGenerator::ClearAllGrass()
{
	for (auto& Pair : GrassComponents)
	{
		for (UHierarchicalInstancedStaticMeshComponent* Component : Pair.Value)
		{
			if (Component)
			{
				Component->ClearInstances();
				Component->DestroyComponent();
			}
		}
	}

	GrassComponents.Empty();
	ActiveChunks.Empty();
	GenerationStats = FGrassGenerationStats();

	UE_LOG(LogTemp, Log, TEXT("GrassGenerator: Cleared all grass instances"));
}

void AGrassGenerator::UpdateStreaming(const FVector& ViewLocation)
{
	if (!bEnableStreaming)
	{
		return;
	}

	LastViewLocation = ViewLocation;

	// Calculate which chunks should be active
	TSet<FIntVector> NewActiveChunks;

	int32 ChunkRadius = FMath::CeilToInt(StreamingDistance / ChunkSize);

	FIntVector ViewChunk = FIntVector(
		FMath::FloorToInt(ViewLocation.X / ChunkSize),
		FMath::FloorToInt(ViewLocation.Y / ChunkSize),
		0
	);

	for (int32 x = -ChunkRadius; x <= ChunkRadius; ++x)
	{
		for (int32 y = -ChunkRadius; y <= ChunkRadius; ++y)
		{
			FIntVector ChunkIndex = ViewChunk + FIntVector(x, y, 0);
			NewActiveChunks.Add(ChunkIndex);
		}
	}

	// Remove old chunks
	for (const FIntVector& OldChunk : ActiveChunks)
	{
		if (!NewActiveChunks.Contains(OldChunk))
		{
			// Chunk is no longer needed - could remove instances here
		}
	}

	// Add new chunks
	for (const FIntVector& NewChunk : NewActiveChunks)
	{
		if (!ActiveChunks.Contains(NewChunk))
		{
			// Generate grass in new chunk
			FVector2D ChunkCenter = FVector2D(
				NewChunk.X * ChunkSize + ChunkSize * 0.5f,
				NewChunk.Y * ChunkSize + ChunkSize * 0.5f
			);

			for (int32 TypeIndex = 0; TypeIndex < GrassTypes.Num(); ++TypeIndex)
			{
				GenerateGrassInCell(ChunkCenter, TypeIndex);
			}
		}
	}

	ActiveChunks = NewActiveChunks;
}

float AGrassGenerator::GetDensityAtLocation(const FVector& Location, int32 GrassTypeIndex) const
{
	if (!GrassTypes.IsValidIndex(GrassTypeIndex))
	{
		return 0.0f;
	}

	const FGrassTypeConfig& GrassType = GrassTypes[GrassTypeIndex];
	float BaseDensity = GrassType.Density * BaseDensityMultiplier;

	// Apply density modifier based on mode
	float Modifier = CalculateDensityModifier(Location, GrassTypeIndex);

	return BaseDensity * Modifier;
}

void AGrassGenerator::UpdateWindParameters(const FGrassWindSettings& WindSettings)
{
	for (auto& Pair : GrassComponents)
	{
		for (UHierarchicalInstancedStaticMeshComponent* Component : Pair.Value)
		{
			if (Component)
			{
				UpdateMaterialWindParameters(Component, WindSettings);
			}
		}
	}
}

void AGrassGenerator::SetWindDirection(float Degrees)
{
	for (FGrassTypeConfig& GrassType : GrassTypes)
	{
		GrassType.WindSettings.WindDirection = Degrees;
	}
}

void AGrassGenerator::SetWindStrength(float Strength)
{
	for (FGrassTypeConfig& GrassType : GrassTypes)
	{
		GrassType.WindSettings.WindStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
	}
}

int32 AGrassGenerator::GetTotalInstanceCount() const
{
	return GenerationStats.TotalInstances;
}

void AGrassGenerator::IntegrateWithLandscape(ALandscape* Landscape)
{
	if (!Landscape)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrassGenerator: Invalid landscape for integration"));
		return;
	}

	// Integration with landscape grass system
	// This would involve setting up landscape layers and grass types
	UE_LOG(LogTemp, Log, TEXT("GrassGenerator: Integrated with landscape"));
}

void AGrassGenerator::CreateGrassComponents()
{
	// Components are created on-demand in GetOrCreateGrassComponent
}

UHierarchicalInstancedStaticMeshComponent* AGrassGenerator::GetOrCreateGrassComponent(int32 GrassTypeIndex, int32 ComponentIndex)
{
	TArray<UHierarchicalInstancedStaticMeshComponent*>& Components = GrassComponents.FindOrAdd(GrassTypeIndex);

	// Ensure array is large enough
	while (Components.Num() <= ComponentIndex)
	{
		FString ComponentName = FString::Printf(TEXT("GrassHISM_%d_%d"), GrassTypeIndex, Components.Num());
		UHierarchicalInstancedStaticMeshComponent* NewComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			this,
			*ComponentName
		);

		if (NewComponent)
		{
			NewComponent->SetupAttachment(GetRootComponent());
			NewComponent->RegisterComponent();

			// Configure component
			if (GrassTypes.IsValidIndex(GrassTypeIndex))
			{
				const FGrassTypeConfig& GrassType = GrassTypes[GrassTypeIndex];
				NewComponent->SetStaticMesh(GrassType.GrassMesh);
				NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				NewComponent->SetCastShadow(GrassType.bCastShadow);
				NewComponent->SetReceivesDecals(GrassType.bReceiveDecals);
				NewComponent->SetCullDistances(GrassType.StartCullFadeDistance, GrassType.CullDistance);

				// Apply override material
				if (GrassType.OverrideMaterial)
				{
					NewComponent->SetMaterial(0, GrassType.OverrideMaterial);
				}
			}

			Components.Add(NewComponent);
			GenerationStats.ComponentCount++;
		}
	}

	return Components.IsValidIndex(ComponentIndex) ? Components[ComponentIndex] : nullptr;
}

void AGrassGenerator::GenerateGrassInCell(const FVector2D& CellCenter, int32 GrassTypeIndex)
{
	if (!GrassTypes.IsValidIndex(GrassTypeIndex))
	{
		return;
	}

	const FGrassTypeConfig& GrassType = GrassTypes[GrassTypeIndex];

	if (!GrassType.GrassMesh)
	{
		return;
	}

	// Calculate number of instances for this cell
	float CellArea = (GridCellSize * GridCellSize) / 10000.0f; // Square meters
	float EffectiveDensity = GetDensityAtLocation(FVector(CellCenter, 0.0f), GrassTypeIndex);
	int32 NumInstances = FMath::RoundToInt(CellArea * EffectiveDensity);

	// Apply density jitter
	if (GrassType.DensityJitter > 0.0f)
	{
		float Jitter = RandomStream.FRandRange(-GrassType.DensityJitter, GrassType.DensityJitter);
		NumInstances = FMath::Max(0, NumInstances + FMath::RoundToInt(NumInstances * Jitter));
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Generate instances
	for (int32 i = 0; i < NumInstances; ++i)
	{
		// Random position within cell
		FVector RandomOffset = FVector(
			RandomStream.FRandRange(-GridCellSize * 0.5f, GridCellSize * 0.5f),
			RandomStream.FRandRange(-GridCellSize * 0.5f, GridCellSize * 0.5f),
			0.0f
		);

		FVector SpawnLocation = FVector(CellCenter, 0.0f) + RandomOffset;

		// Trace to ground
		FVector TraceStart = SpawnLocation + FVector(0.0f, 0.0f, 1000.0f);
		FVector TraceEnd = SpawnLocation - FVector(0.0f, 0.0f, 1000.0f);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			// Check if we should spawn grass here
			if (!ShouldSpawnGrass(HitResult.Location, HitResult.Normal, GrassTypeIndex))
			{
				continue;
			}

			// Generate transform
			FTransform GrassTransform = GenerateGrassTransform(HitResult.Location, HitResult.Normal, GrassTypeIndex);

			// Add instance
			AddGrassInstance(GrassTypeIndex, GrassTransform);
			GenerationStats.TotalInstances++;
		}
	}
}

float AGrassGenerator::CalculateDensityModifier(const FVector& Location, int32 GrassTypeIndex) const
{
	float Modifier = 1.0f;

	switch (DensityMode)
	{
	case EGrassDensityMode::Uniform:
		Modifier = 1.0f;
		break;

	case EGrassDensityMode::NoiseVariation:
		Modifier = GetDensityNoise(Location);
		break;

	case EGrassDensityMode::TextureMask:
		if (DensityMask)
		{
			Modifier = SampleDensityTexture(Location);
		}
		break;

	case EGrassDensityMode::BiomeBased:
		// Would integrate with biome system
		Modifier = 1.0f;
		break;
	}

	return FMath::Clamp(Modifier, 0.0f, 2.0f);
}

float AGrassGenerator::GetDensityNoise(const FVector& Location) const
{
	// Simple Perlin-like noise
	float X = Location.X * DensityNoiseScale;
	float Y = Location.Y * DensityNoiseScale;

	float Noise = FMath::PerlinNoise2D(FVector2D(X, Y));
	Noise = (Noise + 1.0f) * 0.5f; // Normalize to 0-1

	return Noise;
}

float AGrassGenerator::SampleDensityTexture(const FVector& Location) const
{
	// Placeholder for texture sampling
	// Would need proper UV mapping and texture reading
	return 1.0f;
}

FTransform AGrassGenerator::GenerateGrassTransform(const FVector& Location, const FVector& Normal, int32 GrassTypeIndex)
{
	if (!GrassTypes.IsValidIndex(GrassTypeIndex))
	{
		return FTransform::Identity;
	}

	const FGrassTypeConfig& GrassType = GrassTypes[GrassTypeIndex];

	// Generate scale
	FVector Scale = FVector(
		RandomStream.FRandRange(GrassType.MinScale, GrassType.MaxScale),
		RandomStream.FRandRange(GrassType.MinScale, GrassType.MaxScale),
		RandomStream.FRandRange(GrassType.MinScale, GrassType.MaxScale)
	);

	// Apply scale variance
	Scale.X *= (1.0f + RandomStream.FRandRange(-GrassType.ScaleVariance.X, GrassType.ScaleVariance.X));
	Scale.Y *= (1.0f + RandomStream.FRandRange(-GrassType.ScaleVariance.Y, GrassType.ScaleVariance.Y));
	Scale.Z *= (1.0f + RandomStream.FRandRange(-GrassType.ScaleVariance.Z, GrassType.ScaleVariance.Z));

	// Generate rotation
	FRotator Rotation = FRotator::ZeroRotator;

	if (GrassType.bRandomRotation)
	{
		Rotation.Yaw = RandomStream.FRandRange(0.0f, 360.0f);
	}

	// Align to ground normal
	if (GrassType.bAlignToGround && GrassType.AlignmentStrength > 0.0f)
	{
		FVector AlignedZ = FMath::Lerp(FVector::UpVector, Normal, GrassType.AlignmentStrength);
		FVector AlignedX = FVector::CrossProduct(FVector::RightVector, AlignedZ).GetSafeNormal();
		if (AlignedX.IsNearlyZero())
		{
			AlignedX = FVector::CrossProduct(FVector::ForwardVector, AlignedZ).GetSafeNormal();
		}
		FVector AlignedY = FVector::CrossProduct(AlignedZ, AlignedX);

		FMatrix AlignMatrix = FMatrix(AlignedX, AlignedY, AlignedZ, FVector::ZeroVector);
		FRotator AlignRotation = AlignMatrix.Rotator();

		Rotation.Pitch = AlignRotation.Pitch;
		Rotation.Roll = AlignRotation.Roll;
	}

	return FTransform(Rotation, Location, Scale);
}

void AGrassGenerator::AddGrassInstance(int32 GrassTypeIndex, const FTransform& Transform)
{
	// Find component with space
	int32 ComponentIndex = 0;
	UHierarchicalInstancedStaticMeshComponent* Component = GetOrCreateGrassComponent(GrassTypeIndex, ComponentIndex);

	while (Component && Component->GetInstanceCount() >= MaxInstancesPerComponent)
	{
		ComponentIndex++;
		Component = GetOrCreateGrassComponent(GrassTypeIndex, ComponentIndex);
	}

	if (Component)
	{
		Component->AddInstance(Transform);
	}
}

void AGrassGenerator::UpdateMaterialWindParameters(UHierarchicalInstancedStaticMeshComponent* Component, const FGrassWindSettings& WindSettings)
{
	if (!Component)
	{
		return;
	}

	// Create dynamic material instance if needed
	for (int32 i = 0; i < Component->GetNumMaterials(); ++i)
	{
		UMaterialInterface* Material = Component->GetMaterial(i);
		if (Material)
		{
			UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);
			if (!DynamicMaterial)
			{
				DynamicMaterial = Component->CreateDynamicMaterialInstance(i, Material);
			}

			if (DynamicMaterial)
			{
				// Set wind parameters
				DynamicMaterial->SetScalarParameterValue(WindSettings.WindParameterName, WindSettings.WindStrength);
				DynamicMaterial->SetVectorParameterValue(TEXT("WindVector"), CurrentWindVector);
				DynamicMaterial->SetScalarParameterValue(TEXT("WindSpeed"), WindSettings.WindSpeed);
			}
		}
	}
}

bool AGrassGenerator::ShouldSpawnGrass(const FVector& Location, const FVector& Normal, int32 GrassTypeIndex) const
{
	if (!GrassTypes.IsValidIndex(GrassTypeIndex))
	{
		return false;
	}

	const FGrassTypeConfig& GrassType = GrassTypes[GrassTypeIndex];

	// Check slope
	float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
	if (SlopeAngle > GrassType.MaxSlope)
	{
		return false;
	}

	// Check biome restrictions
	if (GrassType.AllowedBiomes.Num() > 0)
	{
		// Would integrate with biome system
	}

	return true;
}

void AGrassGenerator::DrawDebugVisualization() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector Center = GetActorLocation();

	// Draw generation radius
	DrawDebugCircle(World, Center, GenerationRadius, 64, FColor::Green, false, -1.0f, 0, 10.0f,
		FVector::ForwardVector, FVector::RightVector, false);

	// Draw streaming distance if enabled
	if (bEnableStreaming)
	{
		DrawDebugCircle(World, LastViewLocation, StreamingDistance, 64, FColor::Blue, false, -1.0f, 0, 10.0f,
			FVector::ForwardVector, FVector::RightVector, false);

		// Draw active chunks
		for (const FIntVector& Chunk : ActiveChunks)
		{
			FVector ChunkCenter = FVector(
				Chunk.X * ChunkSize + ChunkSize * 0.5f,
				Chunk.Y * ChunkSize + ChunkSize * 0.5f,
				Center.Z
			);

			DrawDebugBox(World, ChunkCenter, FVector(ChunkSize * 0.5f), FColor::Yellow, false, -1.0f, 0, 5.0f);
		}
	}

	// Draw stats
	if (bShowStats)
	{
		FString StatsText = FString::Printf(
			TEXT("Grass Instances: %d\nComponents: %d\nArea: %.1f sq.m\nGen Time: %.2fs"),
			GenerationStats.TotalInstances,
			GenerationStats.ComponentCount,
			GenerationStats.AreaCovered,
			GenerationStats.GenerationTime
		);

		DrawDebugString(World, Center + FVector(0.0f, 0.0f, 200.0f), StatsText, nullptr, FColor::White, 0.0f, true);
	}
}
