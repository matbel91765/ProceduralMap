// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "HydraulicErosion.h"
#include "Math/UnrealMathUtility.h"

UHydraulicErosion::UHydraulicErosion()
	: NumIterations(50000)
	, ErosionRadius(3)
	, RandomSeed(0)
	, MaxDropletLifetime(30)
	, Inertia(0.05f)
	, SedimentCapacityFactor(4.0f)
	, MinSedimentCapacity(0.01f)
	, ErodeSpeed(0.3f)
	, DepositSpeed(0.3f)
	, EvaporateSpeed(0.01f)
	, Gravity(4.0f)
	, InitialWaterVolume(1.0f)
	, InitialSpeed(1.0f)
	, CurrentMapSize(0)
	, CurrentBrushRadius(0)
	, AsyncErosionTask(nullptr)
{
}

bool UHydraulicErosion::Erode(TArray<float>& HeightMap, int32 MapSize, bool bAsync)
{
	// Validate input
	if (HeightMap.Num() != MapSize * MapSize)
	{
		UE_LOG(LogTemp, Error, TEXT("HydraulicErosion: HeightMap size (%d) doesn't match MapSize^2 (%d)"),
			HeightMap.Num(), MapSize * MapSize);
		return false;
	}

	if (MapSize < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("HydraulicErosion: MapSize must be at least 2"));
		return false;
	}

	// If async requested, spawn task
	if (bAsync)
	{
		FScopeLock Lock(&ErosionMutex);

		// Check if already running
		if (AsyncErosionTask && !AsyncErosionTask->IsDone())
		{
			UE_LOG(LogTemp, Warning, TEXT("HydraulicErosion: Already running async erosion"));
			return false;
		}

		// Clean up old task
		if (AsyncErosionTask)
		{
			delete AsyncErosionTask;
			AsyncErosionTask = nullptr;
		}

		// Create new task
		AsyncErosionTask = new FAsyncTask<FHydraulicErosionTask>(this, &HeightMap, MapSize);
		AsyncErosionTask->StartBackgroundTask();

		return true;
	}

	// Initialize brush indices if needed
	if (CurrentMapSize != MapSize || CurrentBrushRadius != ErosionRadius)
	{
		CurrentMapSize = MapSize;
		CurrentBrushRadius = ErosionRadius;
		InitializeBrushIndices();
	}

	// Initialize random stream
	FRandomStream RandomStream(RandomSeed);

	// Simulate droplets
	const int32 TotalIterations = FMath::Max(1, NumIterations);

	for (int32 Iteration = 0; Iteration < TotalIterations; ++Iteration)
	{
		// Random starting position (0-1 range)
		const float StartX = RandomStream.FRand();
		const float StartY = RandomStream.FRand();

		// Simulate this droplet
		SimulateDroplet(HeightMap, MapSize, StartX, StartY, RandomStream);
	}

	return true;
}

bool UHydraulicErosion::IsErosionComplete() const
{
	FScopeLock Lock(const_cast<FCriticalSection*>(&ErosionMutex));
	return !AsyncErosionTask || AsyncErosionTask->IsDone();
}

bool UHydraulicErosion::WaitForCompletion(float TimeoutSeconds)
{
	const double StartTime = FPlatformTime::Seconds();
	const double Timeout = TimeoutSeconds > 0.0f ? TimeoutSeconds : TNumericLimits<double>::Max();

	while (!IsErosionComplete())
	{
		FPlatformProcess::Sleep(0.001f); // 1ms sleep

		if (FPlatformTime::Seconds() - StartTime > Timeout)
		{
			return false;
		}
	}

	return true;
}

void UHydraulicErosion::InitializeBrushIndices()
{
	BrushIndices.Empty();
	BrushIndices.SetNum((ErosionRadius + 1) * 2);

	// For each possible brush center offset
	for (int32 BrushY = -ErosionRadius; BrushY <= ErosionRadius; ++BrushY)
	{
		for (int32 BrushX = -ErosionRadius; BrushX <= ErosionRadius; ++BrushX)
		{
			const float Distance = FMath::Sqrt(static_cast<float>(BrushX * BrushX + BrushY * BrushY));

			if (Distance <= ErosionRadius)
			{
				const int32 BrushIndex = (BrushY + ErosionRadius) * (ErosionRadius * 2 + 1) + (BrushX + ErosionRadius);

				// Add indices for this brush position
				for (int32 Y = -ErosionRadius; Y <= ErosionRadius; ++Y)
				{
					for (int32 X = -ErosionRadius; X <= ErosionRadius; ++X)
					{
						const float OffsetDistance = FMath::Sqrt(static_cast<float>(X * X + Y * Y));

						if (OffsetDistance <= ErosionRadius)
						{
							if (BrushIndex >= BrushIndices.Num())
							{
								BrushIndices.SetNum(BrushIndex + 1);
							}

							const int32 LocalIndex = (Y + ErosionRadius) * (ErosionRadius * 2 + 1) + (X + ErosionRadius);
							const float Weight = 1.0f - (OffsetDistance / ErosionRadius);

							BrushIndices[BrushIndex].Indices.Add(LocalIndex);
							BrushIndices[BrushIndex].Weights.Add(Weight);
							BrushIndices[BrushIndex].NumIndices++;
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HydraulicErosion: Initialized %d brush patterns for radius %d"),
		BrushIndices.Num(), ErosionRadius);
}

void UHydraulicErosion::SimulateDroplet(TArray<float>& HeightMap, int32 MapSize, float StartX, float StartY, FRandomStream& RandomStream)
{
	// Initialize droplet
	FErosionDroplet Droplet;
	Droplet.Position = FVector2D(StartX * (MapSize - 1), StartY * (MapSize - 1));
	Droplet.Direction = FVector2D::ZeroVector;
	Droplet.Speed = InitialSpeed;
	Droplet.Water = InitialWaterVolume;
	Droplet.Sediment = 0.0f;

	// Simulate droplet path
	for (int32 Lifetime = 0; Lifetime < MaxDropletLifetime; ++Lifetime)
	{
		const int32 NodeX = FMath::FloorToInt(Droplet.Position.X);
		const int32 NodeY = FMath::FloorToInt(Droplet.Position.Y);
		const float CellOffsetX = Droplet.Position.X - NodeX;
		const float CellOffsetY = Droplet.Position.Y - NodeY;

		// Check if droplet is still on map
		if (NodeX < 0 || NodeX >= MapSize - 1 || NodeY < 0 || NodeY >= MapSize - 1)
		{
			break;
		}

		// Calculate height and gradient
		FVector2D Gradient;
		const float Height = CalculateHeightAndGradient(HeightMap, MapSize, Droplet.Position.X, Droplet.Position.Y, Gradient);

		// Update direction and speed
		Droplet.Direction = Droplet.Direction * Inertia - Gradient * (1.0f - Inertia);

		// Normalize direction
		const float DirectionLength = Droplet.Direction.Size();
		if (DirectionLength > 0.0001f)
		{
			Droplet.Direction /= DirectionLength;
		}
		else
		{
			// No gradient, stop droplet
			break;
		}

		// Update position
		const FVector2D NewPosition = Droplet.Position + Droplet.Direction;

		// Calculate new height
		FVector2D NewGradient;
		const float NewHeight = CalculateHeightAndGradient(HeightMap, MapSize, NewPosition.X, NewPosition.Y, NewGradient);

		// Calculate height difference
		const float DeltaHeight = NewHeight - Height;

		// Calculate sediment capacity
		const float SedimentCapacity = FMath::Max(
			-DeltaHeight * Droplet.Speed * Droplet.Water * SedimentCapacityFactor,
			MinSedimentCapacity
		);

		// Erode or deposit sediment
		if (Droplet.Sediment > SedimentCapacity || DeltaHeight > 0.0f)
		{
			// Deposit sediment
			const float AmountToDeposit = (DeltaHeight > 0.0f)
				? FMath::Min(DeltaHeight, Droplet.Sediment)
				: (Droplet.Sediment - SedimentCapacity) * DepositSpeed;

			Droplet.Sediment -= AmountToDeposit;
			ErodeOrDeposit(HeightMap, MapSize, Droplet.Position.X, Droplet.Position.Y, -AmountToDeposit);
		}
		else
		{
			// Erode terrain
			const float AmountToErode = FMath::Min((SedimentCapacity - Droplet.Sediment) * ErodeSpeed, -DeltaHeight);

			ErodeOrDeposit(HeightMap, MapSize, Droplet.Position.X, Droplet.Position.Y, AmountToErode);
			Droplet.Sediment += AmountToErode;
		}

		// Update speed and water
		Droplet.Speed = FMath::Sqrt(FMath::Max(0.0f, Droplet.Speed * Droplet.Speed + DeltaHeight * Gravity));
		Droplet.Water *= (1.0f - EvaporateSpeed);
		Droplet.Position = NewPosition;

		// Stop if water is gone
		if (Droplet.Water < 0.01f)
		{
			break;
		}
	}
}

float UHydraulicErosion::CalculateHeightAndGradient(const TArray<float>& HeightMap, int32 MapSize, float PosX, float PosY, FVector2D& OutGradient) const
{
	// Get cell coordinates
	const int32 X0 = FMath::FloorToInt(PosX);
	const int32 Y0 = FMath::FloorToInt(PosY);
	const int32 X1 = FMath::Min(X0 + 1, MapSize - 1);
	const int32 Y1 = FMath::Min(Y0 + 1, MapSize - 1);

	// Clamp to valid range
	const int32 ClampedX0 = FMath::Clamp(X0, 0, MapSize - 1);
	const int32 ClampedY0 = FMath::Clamp(Y0, 0, MapSize - 1);

	// Calculate interpolation weights
	const float Tx = PosX - X0;
	const float Ty = PosY - Y0;

	// Sample heights at four corners
	const float H00 = HeightMap[ClampedY0 * MapSize + ClampedX0];
	const float H10 = HeightMap[ClampedY0 * MapSize + X1];
	const float H01 = HeightMap[Y1 * MapSize + ClampedX0];
	const float H11 = HeightMap[Y1 * MapSize + X1];

	// Bilinear interpolation for height
	const float Height = H00 * (1.0f - Tx) * (1.0f - Ty)
		+ H10 * Tx * (1.0f - Ty)
		+ H01 * (1.0f - Tx) * Ty
		+ H11 * Tx * Ty;

	// Calculate gradient (derivative of bilinear interpolation)
	const float GradientX = (H10 - H00) * (1.0f - Ty) + (H11 - H01) * Ty;
	const float GradientY = (H01 - H00) * (1.0f - Tx) + (H11 - H10) * Tx;

	OutGradient = FVector2D(GradientX, GradientY);

	return Height;
}

void UHydraulicErosion::ErodeOrDeposit(TArray<float>& HeightMap, int32 MapSize, float PosX, float PosY, float Amount)
{
	// Get cell coordinates
	const int32 CenterX = FMath::RoundToInt(PosX);
	const int32 CenterY = FMath::RoundToInt(PosY);

	// Apply erosion/deposition using brush
	for (int32 BrushY = -ErosionRadius; BrushY <= ErosionRadius; ++BrushY)
	{
		for (int32 BrushX = -ErosionRadius; BrushX <= ErosionRadius; ++BrushX)
		{
			const int32 X = CenterX + BrushX;
			const int32 Y = CenterY + BrushY;

			// Check bounds
			if (X >= 0 && X < MapSize && Y >= 0 && Y < MapSize)
			{
				// Calculate weight based on distance
				const float Distance = FMath::Sqrt(static_cast<float>(BrushX * BrushX + BrushY * BrushY));

				if (Distance <= ErosionRadius)
				{
					const float Weight = 1.0f - (Distance / ErosionRadius);
					const int32 Index = Y * MapSize + X;

					// Apply weighted erosion/deposition
					HeightMap[Index] -= Amount * Weight;
				}
			}
		}
	}
}
