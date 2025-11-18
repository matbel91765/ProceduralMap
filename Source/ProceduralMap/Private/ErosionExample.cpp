// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ErosionExample.h"
#include "Math/UnrealMathUtility.h"

AErosionExample::AErosionExample()
	: MapSize(256)
	, bEnableThermalErosion(true)
	, bAutoApplyOnBeginPlay(false)
	, bPreviewInEditor(false)
	, bIsErosionRunning(false)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create default erosion systems
	HydraulicErosion = CreateDefaultSubobject<UHydraulicErosion>(TEXT("HydraulicErosion"));
	ThermalErosion = CreateDefaultSubobject<UThermalErosion>(TEXT("ThermalErosion"));
}

void AErosionExample::BeginPlay()
{
	Super::BeginPlay();

	// Generate test heightmap if empty
	if (HeightMap.Num() == 0)
	{
		GenerateTestHeightmap();
	}

	// Auto-apply erosion if enabled
	if (bAutoApplyOnBeginPlay)
	{
		ApplyErosion(false, true);
	}
}

void AErosionExample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if async erosion is complete
	if (bIsErosionRunning)
	{
		if (IsErosionComplete())
		{
			bIsErosionRunning = false;
			UE_LOG(LogTemp, Log, TEXT("Erosion completed!"));

			// Apply thermal erosion if enabled and hydraulic is done
			if (bEnableThermalErosion && ThermalErosion && ThermalErosion->IsErosionComplete())
			{
				UE_LOG(LogTemp, Log, TEXT("Starting thermal erosion..."));
				ApplyThermalErosion(true);
			}
		}
	}
}

void AErosionExample::ApplyErosion(bool bUseGPU, bool bAsync)
{
	if (HeightMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No heightmap to erode. Generate test heightmap first."));
		return;
	}

	// Apply hydraulic erosion first
	ApplyHydraulicErosion(bUseGPU, bAsync);

	// Thermal erosion will be applied after hydraulic completes (in Tick)
	bIsErosionRunning = bAsync;
}

void AErosionExample::ApplyHydraulicErosion(bool bUseGPU, bool bAsync)
{
	if (!HydraulicErosion)
	{
		UE_LOG(LogTemp, Error, TEXT("HydraulicErosion is null!"));
		return;
	}

	if (HeightMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No heightmap to erode."));
		return;
	}

	const double StartTime = FPlatformTime::Seconds();

	if (bUseGPU)
	{
		// Use GPU acceleration
		UE_LOG(LogTemp, Log, TEXT("Applying GPU-accelerated hydraulic erosion..."));

		FErosionParameters Params;
		Params.MaxLifetime = 30;
		Params.ErosionRadius = 3;
		Params.RandomSeed = FMath::Rand();
		Params.Inertia = HydraulicErosion->Inertia;
		Params.SedimentCapacityFactor = HydraulicErosion->SedimentCapacityFactor;
		Params.MinSedimentCapacity = HydraulicErosion->MinSedimentCapacity;
		Params.ErodeSpeed = HydraulicErosion->ErodeSpeed;
		Params.DepositSpeed = HydraulicErosion->DepositSpeed;
		Params.EvaporateSpeed = HydraulicErosion->EvaporateSpeed;
		Params.Gravity = HydraulicErosion->Gravity;
		Params.InitialWaterVolume = HydraulicErosion->InitialWaterVolume;
		Params.InitialSpeed = HydraulicErosion->InitialSpeed;

		TArray<float> OutputHeightMap;
		const bool Success = FErosionGPUInterface::RunErosion(
			HeightMap,
			MapSize,
			OutputHeightMap,
			HydraulicErosion->NumIterations,
			&Params
		);

		if (Success)
		{
			HeightMap = OutputHeightMap;
			const double ElapsedTime = FPlatformTime::Seconds() - StartTime;
			UE_LOG(LogTemp, Log, TEXT("GPU hydraulic erosion completed in %.3f seconds"), ElapsedTime);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GPU hydraulic erosion failed!"));
		}
	}
	else
	{
		// Use CPU implementation
		UE_LOG(LogTemp, Log, TEXT("Applying CPU hydraulic erosion (%d iterations)..."), HydraulicErosion->NumIterations);

		const bool Success = HydraulicErosion->Erode(HeightMap, MapSize, bAsync);

		if (Success)
		{
			if (!bAsync)
			{
				const double ElapsedTime = FPlatformTime::Seconds() - StartTime;
				UE_LOG(LogTemp, Log, TEXT("CPU hydraulic erosion completed in %.3f seconds"), ElapsedTime);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("CPU hydraulic erosion started (async)..."));
				bIsErosionRunning = true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CPU hydraulic erosion failed!"));
		}
	}
}

void AErosionExample::ApplyThermalErosion(bool bAsync)
{
	if (!ThermalErosion)
	{
		UE_LOG(LogTemp, Error, TEXT("ThermalErosion is null!"));
		return;
	}

	if (HeightMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No heightmap to erode."));
		return;
	}

	const double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogTemp, Log, TEXT("Applying thermal erosion (%d iterations)..."), ThermalErosion->NumIterations);

	const bool Success = ThermalErosion->Erode(HeightMap, MapSize, bAsync);

	if (Success)
	{
		if (!bAsync)
		{
			const double ElapsedTime = FPlatformTime::Seconds() - StartTime;
			UE_LOG(LogTemp, Log, TEXT("Thermal erosion completed in %.3f seconds"), ElapsedTime);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Thermal erosion started (async)..."));
			bIsErosionRunning = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Thermal erosion failed!"));
	}
}

void AErosionExample::GenerateTestHeightmap()
{
	UE_LOG(LogTemp, Log, TEXT("Generating test heightmap (%dx%d)..."), MapSize, MapSize);

	// Initialize heightmap
	HeightMap.SetNum(MapSize * MapSize);

	// Generate simple noise-based heightmap
	FRandomStream Random(12345);

	for (int32 Y = 0; Y < MapSize; ++Y)
	{
		for (int32 X = 0; X < MapSize; ++X)
		{
			const int32 Index = Y * MapSize + X;

			// Normalized coordinates (0-1)
			const float NormX = static_cast<float>(X) / MapSize;
			const float NormY = static_cast<float>(Y) / MapSize;

			// Simple multi-octave noise
			float Height = 0.0f;
			float Amplitude = 1.0f;
			float Frequency = 1.0f;

			for (int32 Octave = 0; Octave < 6; ++Octave)
			{
				const float NoiseX = NormX * Frequency * 10.0f;
				const float NoiseY = NormY * Frequency * 10.0f;

				// Simple noise function
				const float Noise = FMath::PerlinNoise2D(FVector2D(NoiseX, NoiseY));

				Height += Noise * Amplitude;

				Amplitude *= 0.5f;
				Frequency *= 2.0f;
			}

			// Add a dome shape for interesting erosion
			const float CenterX = NormX - 0.5f;
			const float CenterY = NormY - 0.5f;
			const float DistFromCenter = FMath::Sqrt(CenterX * CenterX + CenterY * CenterY);
			const float Dome = FMath::Max(0.0f, 1.0f - DistFromCenter * 2.0f);

			Height = (Height * 0.5f + 0.5f) * 100.0f + Dome * 50.0f;

			HeightMap[Index] = Height;
		}
	}

	// Store original for reset
	OriginalHeightMap = HeightMap;

	UE_LOG(LogTemp, Log, TEXT("Test heightmap generated successfully"));
}

bool AErosionExample::IsErosionComplete() const
{
	bool bHydraulicComplete = !HydraulicErosion || HydraulicErosion->IsErosionComplete();
	bool bThermalComplete = !ThermalErosion || ThermalErosion->IsErosionComplete();

	return bHydraulicComplete && bThermalComplete;
}

void AErosionExample::ResetHeightmap()
{
	if (OriginalHeightMap.Num() > 0)
	{
		HeightMap = OriginalHeightMap;
		UE_LOG(LogTemp, Log, TEXT("Heightmap reset to original state"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No original heightmap to reset to"));
	}
}

void AErosionExample::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

#if WITH_EDITOR
	// Auto-preview in editor if enabled
	if (bPreviewInEditor && PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();

		// Re-apply erosion if erosion parameters changed
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AErosionExample, MapSize))
		{
			GenerateTestHeightmap();
		}
	}
#endif
}
