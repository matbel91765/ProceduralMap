// Ultimate Procedural Map Generation System - Cave Decorator
// Copyright (C) 2025. All Rights Reserved.

#include "CaveDecorator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UCaveDecorator::UCaveDecorator()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default settings
	Seed = 12345;
	VoxelSize = 100.0f;
	WallHeight = 400.0f;

	// Speleothems
	StalactiteSpawnChance = 0.1f;
	StalagmiteSpawnChance = 0.1f;
	StalactiteScaleRange = FVector2D(0.8f, 1.5f);
	StalagmiteScaleRange = FVector2D(0.8f, 1.2f);

	// Rocks
	RockSpawnChance = 0.05f;
	RockScaleRange = FVector2D(0.5f, 1.5f);
	MinRockSpacing = 150.0f;

	// Lighting
	bSpawnLights = true;

	// Particles
	bSpawnParticles = false;

	// Crystals
	CrystalSpawnChance = 0.03f;
	CrystalScaleRange = FVector2D(0.8f, 1.2f);
	bCrystalsEmitLight = true;
	CrystalLightColor = FLinearColor(0.5f, 0.8f, 1.0f);
	CrystalLightIntensity = 2000.0f;
}

void UCaveDecorator::BeginPlay()
{
	Super::BeginPlay();
}

void UCaveDecorator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCaveDecorator::DecorateCave()
{
	if (!CellularAutomata)
	{
		UE_LOG(LogTemp, Error, TEXT("CaveDecorator: CellularAutomata is null!"));
		return;
	}

	// Initialize random stream
	RandomStream.Initialize(Seed);

	// Clear previous decorations
	ClearDecorations();

	// Place decorations in order
	PlaceSpeleothems();
	PlaceCrystals();
	PlaceProps();
	ScatterDebris();

	if (bSpawnLights)
	{
		PlaceLighting();
	}

	if (bSpawnParticles)
	{
		PlaceParticles();
	}

	UE_LOG(LogTemp, Log, TEXT("Cave decoration complete!"));
}

void UCaveDecorator::PlaceSpeleothems()
{
	if (!CellularAutomata || (!StalactiteMesh && !StalagmiteMesh))
	{
		return;
	}

	UInstancedStaticMeshComponent* StalactiteISMC = nullptr;
	UInstancedStaticMeshComponent* StalagmiteISMC = nullptr;

	if (StalactiteMesh)
	{
		StalactiteISMC = GetOrCreateInstancedMeshComponent(StalactiteMesh, SpeleotemMaterial);
	}

	if (StalagmiteMesh)
	{
		StalagmiteISMC = GetOrCreateInstancedMeshComponent(StalagmiteMesh, SpeleotemMaterial);
	}

	// Place in empty spaces adjacent to walls
	for (int32 X = 1; X < CellularAutomata->MapWidth - 1; X++)
	{
		for (int32 Y = 1; Y < CellularAutomata->MapHeight - 1; Y++)
		{
			if (CellularAutomata->GetTile2D(X, Y) == 0) // Empty space
			{
				// Check for ceiling (wall above)
				if (StalactiteISMC && IsCeilingSurface(X, Y))
				{
					if (RandomStream.FRand() < StalactiteSpawnChance)
					{
						FVector Position = GetWorldPosition(X, Y, 0);
						Position.Z += WallHeight / 2.0f;

						float Scale = RandomStream.FRandRange(StalactiteScaleRange.X, StalactiteScaleRange.Y);
						FRotator Rotation(0, RandomStream.FRandRange(0, 360), 0);

						FTransform Transform(Rotation, Position, FVector(Scale, Scale, Scale));
						StalactiteISMC->AddInstance(Transform);
					}
				}

				// Check for floor (wall below)
				if (StalagmiteISMC && IsFloorSurface(X, Y))
				{
					if (RandomStream.FRand() < StalagmiteSpawnChance)
					{
						FVector Position = GetWorldPosition(X, Y, 0);
						Position.Z -= WallHeight / 2.0f;

						float Scale = RandomStream.FRandRange(StalagmiteScaleRange.X, StalagmiteScaleRange.Y);
						FRotator Rotation(0, RandomStream.FRandRange(0, 360), 0);

						FTransform Transform(Rotation, Position, FVector(Scale, Scale, Scale));
						StalagmiteISMC->AddInstance(Transform);
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Placed speleothems"));
}

void UCaveDecorator::PlaceProps()
{
	if (!CellularAutomata || PropRules.Num() == 0)
	{
		return;
	}

	for (const FPropPlacementRule& Rule : PropRules)
	{
		if (!Rule.PropMesh)
		{
			continue;
		}

		UInstancedStaticMeshComponent* ISMC = GetOrCreateInstancedMeshComponent(Rule.PropMesh, Rule.PropMaterial);

		for (int32 X = 1; X < CellularAutomata->MapWidth - 1; X++)
		{
			for (int32 Y = 1; Y < CellularAutomata->MapHeight - 1; Y++)
			{
				if (CellularAutomata->GetTile2D(X, Y) == 0) // Empty space
				{
					bool bShouldSpawn = false;

					if (Rule.bFloorOnly && IsFloorSurface(X, Y))
					{
						bShouldSpawn = true;
					}
					else if (Rule.bCeilingOnly && IsCeilingSurface(X, Y))
					{
						bShouldSpawn = true;
					}
					else if (Rule.bWallsOnly && IsWallSurface(X, Y))
					{
						bShouldSpawn = true;
					}
					else if (!Rule.bFloorOnly && !Rule.bCeilingOnly && !Rule.bWallsOnly)
					{
						bShouldSpawn = IsFloorSurface(X, Y); // Default to floor
					}

					if (bShouldSpawn && RandomStream.FRand() < Rule.SpawnChance)
					{
						FVector Position = GetWorldPosition(X, Y, 0);

						if (Rule.bFloorOnly)
						{
							Position.Z -= WallHeight / 2.0f;
						}
						else if (Rule.bCeilingOnly)
						{
							Position.Z += WallHeight / 2.0f;
						}

						if (IsValidSpawnLocation(Position, Rule.MinSpacing))
						{
							float Scale = RandomStream.FRandRange(Rule.ScaleRange.X, Rule.ScaleRange.Y);
							FRotator Rotation = FRotator::ZeroRotator;

							if (Rule.bRandomRotation)
							{
								Rotation.Yaw = RandomStream.FRandRange(0, 360);
							}

							if (Rule.bAlignToSurface)
							{
								FVector Normal = GetSurfaceNormal(X, Y);
								Rotation = Normal.Rotation();
							}

							FTransform Transform(Rotation, Position, FVector(Scale));
							ISMC->AddInstance(Transform);
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Placed props"));
}

void UCaveDecorator::ScatterDebris()
{
	if (!CellularAutomata || RockMeshes.Num() == 0)
	{
		return;
	}

	for (UStaticMesh* RockMesh : RockMeshes)
	{
		if (!RockMesh)
		{
			continue;
		}

		UInstancedStaticMeshComponent* ISMC = GetOrCreateInstancedMeshComponent(RockMesh, RockMaterial);

		for (int32 X = 1; X < CellularAutomata->MapWidth - 1; X++)
		{
			for (int32 Y = 1; Y < CellularAutomata->MapHeight - 1; Y++)
			{
				if (CellularAutomata->GetTile2D(X, Y) == 0 && IsFloorSurface(X, Y))
				{
					if (RandomStream.FRand() < RockSpawnChance)
					{
						FVector Position = GetWorldPosition(X, Y, 0);
						Position.Z -= WallHeight / 2.0f;

						if (IsValidSpawnLocation(Position, MinRockSpacing))
						{
							float Scale = RandomStream.FRandRange(RockScaleRange.X, RockScaleRange.Y);
							FRotator Rotation(
								RandomStream.FRandRange(-15, 15),
								RandomStream.FRandRange(0, 360),
								RandomStream.FRandRange(-15, 15)
							);

							FTransform Transform(Rotation, Position, FVector(Scale));
							ISMC->AddInstance(Transform);
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Scattered debris"));
}

void UCaveDecorator::PlaceLighting()
{
	if (!CellularAutomata || LightingRules.Num() == 0)
	{
		return;
	}

	// Place lights in each room
	for (const FRoom& Room : CellularAutomata->SurvivingRooms)
	{
		for (const FLightingPlacement& LightRule : LightingRules)
		{
			if (!LightRule.LightClass || RandomStream.FRand() > LightRule.SpawnChance)
			{
				continue;
			}

			int32 NumLights = RandomStream.RandRange(LightRule.MinLightsPerRoom, LightRule.MaxLightsPerRoom);

			for (int32 i = 0; i < NumLights; i++)
			{
				// Pick random tile in room
				if (Room.Tiles.Num() > 0)
				{
					int32 RandomIndex = RandomStream.RandRange(0, Room.Tiles.Num() - 1);
					FCoord3D Tile = Room.Tiles[RandomIndex];

					FVector Position = GetWorldPosition(Tile.X, Tile.Y, Tile.Z);
					Position.Z -= WallHeight / 2.0f - LightRule.HeightOffset;

					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = GetOwner();

					AActor* LightActor = GetWorld()->SpawnActor<AActor>(
						LightRule.LightClass,
						Position,
						FRotator::ZeroRotator,
						SpawnParams
					);

					if (LightActor)
					{
						SpawnedActors.Add(LightActor);

						// Set light properties if it has a point light component
						UPointLightComponent* PointLight = LightActor->FindComponentByClass<UPointLightComponent>();
						if (PointLight)
						{
							float Intensity = RandomStream.FRandRange(LightRule.IntensityRange.X, LightRule.IntensityRange.Y);
							PointLight->SetIntensity(Intensity);
							PointLight->SetLightColor(LightRule.LightColor);
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Placed lighting"));
}

void UCaveDecorator::PlaceParticles()
{
	if (!CellularAutomata || ParticleRules.Num() == 0)
	{
		return;
	}

	for (const FParticlePlacement& ParticleRule : ParticleRules)
	{
		if (!ParticleRule.ParticleSystem && !ParticleRule.NiagaraSystem)
		{
			continue;
		}

		for (int32 X = 1; X < CellularAutomata->MapWidth - 1; X++)
		{
			for (int32 Y = 1; Y < CellularAutomata->MapHeight - 1; Y++)
			{
				if (CellularAutomata->GetTile2D(X, Y) == 0 && RandomStream.FRand() < ParticleRule.SpawnChance)
				{
					FVector Position = GetWorldPosition(X, Y, 0);

					if (IsValidSpawnLocation(Position, ParticleRule.MinSpacing))
					{
						if (ParticleRule.NiagaraSystem)
						{
							// Spawn Niagara system
							UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
								GetWorld(),
								ParticleRule.NiagaraSystem,
								Position,
								FRotator::ZeroRotator,
								FVector(1.0f),
								true,
								true,
								ENCPoolMethod::None,
								true
							);

							if (NiagaraComp && NiagaraComp->GetOwner())
							{
								SpawnedActors.Add(NiagaraComp->GetOwner());
							}
						}
						else if (ParticleRule.ParticleSystem)
						{
							// Spawn legacy particle system
							FActorSpawnParameters SpawnParams;
							SpawnParams.Owner = GetOwner();

							// Note: In production, you'd create a proper actor with ParticleSystemComponent
							// This is simplified for demonstration
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Placed particles"));
}

void UCaveDecorator::PlaceCrystals()
{
	if (!CellularAutomata || !CrystalMesh)
	{
		return;
	}

	UInstancedStaticMeshComponent* CrystalISMC = GetOrCreateInstancedMeshComponent(CrystalMesh, CrystalMaterial);

	for (int32 X = 1; X < CellularAutomata->MapWidth - 1; X++)
	{
		for (int32 Y = 1; Y < CellularAutomata->MapHeight - 1; Y++)
		{
			if (CellularAutomata->GetTile2D(X, Y) == 0 && IsFloorSurface(X, Y))
			{
				if (RandomStream.FRand() < CrystalSpawnChance)
				{
					FVector Position = GetWorldPosition(X, Y, 0);
					Position.Z -= WallHeight / 2.0f;

					float Scale = RandomStream.FRandRange(CrystalScaleRange.X, CrystalScaleRange.Y);
					FRotator Rotation(0, RandomStream.FRandRange(0, 360), 0);

					FTransform Transform(Rotation, Position, FVector(Scale));
					CrystalISMC->AddInstance(Transform);

					// Spawn light for crystal
					if (bCrystalsEmitLight)
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.Owner = GetOwner();

						AActor* LightActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Position, FRotator::ZeroRotator, SpawnParams);
						if (LightActor)
						{
							UPointLightComponent* PointLight = NewObject<UPointLightComponent>(LightActor);
							if (PointLight)
							{
								PointLight->SetIntensity(CrystalLightIntensity * Scale);
								PointLight->SetLightColor(CrystalLightColor);
								PointLight->SetAttenuationRadius(300.0f * Scale);
								PointLight->RegisterComponent();
								PointLight->AttachToComponent(LightActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
							}

							SpawnedActors.Add(LightActor);
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Placed crystals"));
}

void UCaveDecorator::ClearDecorations()
{
	// Clear instanced mesh components
	for (UInstancedStaticMeshComponent* ISMC : InstancedMeshComponents)
	{
		if (ISMC)
		{
			ISMC->ClearInstances();
			ISMC->DestroyComponent();
		}
	}
	InstancedMeshComponents.Empty();

	// Destroy spawned actors
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor && !Actor->IsPendingKill())
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}

FVector UCaveDecorator::GetWorldPosition(int32 X, int32 Y, int32 Z) const
{
	FVector Position = FVector(X * VoxelSize, Y * VoxelSize, Z * VoxelSize);

	if (GetOwner())
	{
		Position += GetOwner()->GetActorLocation();
	}

	return Position;
}

bool UCaveDecorator::IsFloorSurface(int32 X, int32 Y) const
{
	if (!CellularAutomata)
	{
		return false;
	}

	// Check if there's a wall below
	return CellularAutomata->GetTile2D(X, Y) == 0 &&
		   CellularAutomata->GetTile2D(X, Y + 1) == 1;
}

bool UCaveDecorator::IsCeilingSurface(int32 X, int32 Y) const
{
	if (!CellularAutomata)
	{
		return false;
	}

	// Check if there's a wall above
	return CellularAutomata->GetTile2D(X, Y) == 0 &&
		   CellularAutomata->GetTile2D(X, Y - 1) == 1;
}

bool UCaveDecorator::IsWallSurface(int32 X, int32 Y) const
{
	if (!CellularAutomata)
	{
		return false;
	}

	// Check if there's a wall on either side
	return CellularAutomata->GetTile2D(X, Y) == 0 &&
		   (CellularAutomata->GetTile2D(X - 1, Y) == 1 ||
			CellularAutomata->GetTile2D(X + 1, Y) == 1);
}

UInstancedStaticMeshComponent* UCaveDecorator::GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh, UMaterialInterface* Material)
{
	if (!Mesh || !GetOwner())
	{
		return nullptr;
	}

	// Check if we already have an ISMC for this mesh
	for (UInstancedStaticMeshComponent* ISMC : InstancedMeshComponents)
	{
		if (ISMC && ISMC->GetStaticMesh() == Mesh)
		{
			return ISMC;
		}
	}

	// Create new ISMC
	UInstancedStaticMeshComponent* NewISMC = NewObject<UInstancedStaticMeshComponent>(GetOwner());
	if (NewISMC)
	{
		NewISMC->SetStaticMesh(Mesh);

		if (Material)
		{
			NewISMC->SetMaterial(0, Material);
		}

		NewISMC->SetCastShadow(true);
		NewISMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		NewISMC->RegisterComponent();
		NewISMC->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		InstancedMeshComponents.Add(NewISMC);
	}

	return NewISMC;
}

bool UCaveDecorator::IsValidSpawnLocation(FVector Location, float MinDistance) const
{
	// Check against all instanced mesh components
	for (UInstancedStaticMeshComponent* ISMC : InstancedMeshComponents)
	{
		if (ISMC)
		{
			TArray<FInstancedStaticMeshInstanceData> InstanceData;
			for (int32 i = 0; i < ISMC->GetInstanceCount(); i++)
			{
				FTransform InstanceTransform;
				ISMC->GetInstanceTransform(i, InstanceTransform, true);

				float Distance = FVector::Dist(Location, InstanceTransform.GetLocation());
				if (Distance < MinDistance)
				{
					return false;
				}
			}
		}
	}

	// Check against spawned actors
	for (const AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			float Distance = FVector::Dist(Location, Actor->GetActorLocation());
			if (Distance < MinDistance)
			{
				return false;
			}
		}
	}

	return true;
}

FVector UCaveDecorator::GetSurfaceNormal(int32 X, int32 Y) const
{
	if (!CellularAutomata)
	{
		return FVector::UpVector;
	}

	// Calculate gradient to determine surface normal
	float DX = (float)(CellularAutomata->GetTile2D(X + 1, Y) - CellularAutomata->GetTile2D(X - 1, Y));
	float DY = (float)(CellularAutomata->GetTile2D(X, Y + 1) - CellularAutomata->GetTile2D(X, Y - 1));

	return FVector(-DX, -DY, 1.0f).GetSafeNormal();
}
