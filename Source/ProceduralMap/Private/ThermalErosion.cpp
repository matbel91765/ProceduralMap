// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ThermalErosion.h"
#include "Math/UnrealMathUtility.h"

UThermalErosion::UThermalErosion()
	: NumIterations(10)
	, TalusAngle(35.0f)
	, ErosionRate(0.5f)
	, CellSize(100.0f)
	, bUse8Neighbors(true)
	, bApplySmoothing(false)
	, SmoothingFactor(0.1f)
	, AsyncErosionTask(nullptr)
{
}

bool UThermalErosion::Erode(TArray<float>& HeightMap, int32 MapSize, bool bAsync)
{
	// Validate input
	if (HeightMap.Num() != MapSize * MapSize)
	{
		UE_LOG(LogTemp, Error, TEXT("ThermalErosion: HeightMap size (%d) doesn't match MapSize^2 (%d)"),
			HeightMap.Num(), MapSize * MapSize);
		return false;
	}

	if (MapSize < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("ThermalErosion: MapSize must be at least 2"));
		return false;
	}

	// If async requested, spawn task
	if (bAsync)
	{
		FScopeLock Lock(&ErosionMutex);

		// Check if already running
		if (AsyncErosionTask && !AsyncErosionTask->IsDone())
		{
			UE_LOG(LogTemp, Warning, TEXT("ThermalErosion: Already running async erosion"));
			return false;
		}

		// Clean up old task
		if (AsyncErosionTask)
		{
			delete AsyncErosionTask;
			AsyncErosionTask = nullptr;
		}

		// Create new task
		AsyncErosionTask = new FAsyncTask<FThermalErosionTask>(this, &HeightMap, MapSize);
		AsyncErosionTask->StartBackgroundTask();

		return true;
	}

	// Apply erosion iterations
	const int32 TotalIterations = FMath::Max(1, NumIterations);

	for (int32 Iteration = 0; Iteration < TotalIterations; ++Iteration)
	{
		ApplySingleIteration(HeightMap, MapSize);

		// Apply smoothing if enabled
		if (bApplySmoothing && SmoothingFactor > 0.0f)
		{
			ApplySmoothingFilter(HeightMap, MapSize);
		}
	}

	return true;
}

void UThermalErosion::ApplySingleIteration(TArray<float>& HeightMap, int32 MapSize)
{
	// Get neighbor offsets
	TArray<FIntPoint> Neighbors;
	GetNeighborOffsets(Neighbors);

	// Calculate talus threshold
	const float TalusThreshold = CalculateTalusThreshold();

	// Create temporary array for height differences
	TArray<float> HeightDifferences;
	HeightDifferences.SetNumZeroed(MapSize * MapSize);

	// First pass: calculate material to move
	for (int32 Y = 0; Y < MapSize; ++Y)
	{
		for (int32 X = 0; X < MapSize; ++X)
		{
			const int32 Index = Y * MapSize + X;
			const float CurrentHeight = HeightMap[Index];

			float TotalDifference = 0.0f;
			int32 LowerNeighborCount = 0;

			// Check all neighbors
			for (const FIntPoint& Offset : Neighbors)
			{
				const int32 NX = X + Offset.X;
				const int32 NY = Y + Offset.Y;

				// Check bounds
				if (NX >= 0 && NX < MapSize && NY >= 0 && NY < MapSize)
				{
					const int32 NeighborIndex = NY * MapSize + NX;
					const float NeighborHeight = HeightMap[NeighborIndex];
					const float HeightDiff = CurrentHeight - NeighborHeight;

					// Calculate distance to neighbor (diagonal neighbors are farther)
					const float NeighborDistance = FMath::Sqrt(
						static_cast<float>(Offset.X * Offset.X + Offset.Y * Offset.Y)
					) * CellSize;

					// Adjust threshold for diagonal neighbors
					const float AdjustedThreshold = TalusThreshold * (NeighborDistance / CellSize);

					// If difference exceeds talus angle, material should slide
					if (HeightDiff > AdjustedThreshold)
					{
						TotalDifference += HeightDiff - AdjustedThreshold;
						LowerNeighborCount++;
					}
				}
			}

			// Calculate amount to erode from this cell
			if (LowerNeighborCount > 0)
			{
				const float AmountToMove = TotalDifference * ErosionRate * 0.5f / static_cast<float>(LowerNeighborCount);
				HeightDifferences[Index] = AmountToMove;
			}
		}
	}

	// Second pass: apply erosion and deposition
	for (int32 Y = 0; Y < MapSize; ++Y)
	{
		for (int32 X = 0; X < MapSize; ++X)
		{
			const int32 Index = Y * MapSize + X;
			const float AmountToMove = HeightDifferences[Index];

			if (AmountToMove > 0.0f)
			{
				const float CurrentHeight = HeightMap[Index];

				// Count lower neighbors
				TArray<int32> LowerNeighbors;

				for (const FIntPoint& Offset : Neighbors)
				{
					const int32 NX = X + Offset.X;
					const int32 NY = Y + Offset.Y;

					if (NX >= 0 && NX < MapSize && NY >= 0 && NY < MapSize)
					{
						const int32 NeighborIndex = NY * MapSize + NX;
						const float NeighborHeight = HeightMap[NeighborIndex];

						if (CurrentHeight > NeighborHeight)
						{
							LowerNeighbors.Add(NeighborIndex);
						}
					}
				}

				// Distribute material to lower neighbors
				if (LowerNeighbors.Num() > 0)
				{
					const float MaterialPerNeighbor = AmountToMove / static_cast<float>(LowerNeighbors.Num());

					// Erode from current cell
					HeightMap[Index] -= AmountToMove;

					// Deposit to neighbors
					for (int32 NeighborIndex : LowerNeighbors)
					{
						HeightMap[NeighborIndex] += MaterialPerNeighbor;
					}
				}
			}
		}
	}
}

bool UThermalErosion::IsErosionComplete() const
{
	FScopeLock Lock(const_cast<FCriticalSection*>(&ErosionMutex));
	return !AsyncErosionTask || AsyncErosionTask->IsDone();
}

bool UThermalErosion::WaitForCompletion(float TimeoutSeconds)
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

void UThermalErosion::GetNeighborOffsets(TArray<FIntPoint>& OutOffsets) const
{
	OutOffsets.Empty();

	if (bUse8Neighbors)
	{
		// Moore neighborhood (8 neighbors)
		OutOffsets.Add(FIntPoint(-1, -1)); // Top-left
		OutOffsets.Add(FIntPoint(0, -1));  // Top
		OutOffsets.Add(FIntPoint(1, -1));  // Top-right
		OutOffsets.Add(FIntPoint(-1, 0));  // Left
		OutOffsets.Add(FIntPoint(1, 0));   // Right
		OutOffsets.Add(FIntPoint(-1, 1));  // Bottom-left
		OutOffsets.Add(FIntPoint(0, 1));   // Bottom
		OutOffsets.Add(FIntPoint(1, 1));   // Bottom-right
	}
	else
	{
		// Von Neumann neighborhood (4 neighbors)
		OutOffsets.Add(FIntPoint(0, -1));  // Top
		OutOffsets.Add(FIntPoint(-1, 0));  // Left
		OutOffsets.Add(FIntPoint(1, 0));   // Right
		OutOffsets.Add(FIntPoint(0, 1));   // Bottom
	}
}

float UThermalErosion::CalculateExcessHeight(const TArray<float>& HeightMap, int32 MapSize, int32 X, int32 Y, const TArray<FIntPoint>& Neighbors) const
{
	const int32 Index = Y * MapSize + X;
	const float CurrentHeight = HeightMap[Index];
	const float TalusThreshold = CalculateTalusThreshold();

	float TotalExcess = 0.0f;

	for (const FIntPoint& Offset : Neighbors)
	{
		const int32 NX = X + Offset.X;
		const int32 NY = Y + Offset.Y;

		if (NX >= 0 && NX < MapSize && NY >= 0 && NY < MapSize)
		{
			const int32 NeighborIndex = NY * MapSize + NX;
			const float NeighborHeight = HeightMap[NeighborIndex];
			const float HeightDiff = CurrentHeight - NeighborHeight;

			// Calculate distance
			const float Distance = FMath::Sqrt(
				static_cast<float>(Offset.X * Offset.X + Offset.Y * Offset.Y)
			) * CellSize;

			const float AdjustedThreshold = TalusThreshold * (Distance / CellSize);

			if (HeightDiff > AdjustedThreshold)
			{
				TotalExcess += HeightDiff - AdjustedThreshold;
			}
		}
	}

	return TotalExcess;
}

void UThermalErosion::ApplySmoothingFilter(TArray<float>& HeightMap, int32 MapSize)
{
	// Create temporary array for smoothed heights
	TArray<float> SmoothedHeights = HeightMap;

	// Apply 3x3 box filter
	for (int32 Y = 1; Y < MapSize - 1; ++Y)
	{
		for (int32 X = 1; X < MapSize - 1; ++X)
		{
			const int32 Index = Y * MapSize + X;

			// Calculate average of 3x3 neighborhood
			float Sum = 0.0f;
			int32 Count = 0;

			for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
			{
				for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
				{
					const int32 NX = X + OffsetX;
					const int32 NY = Y + OffsetY;
					const int32 NeighborIndex = NY * MapSize + NX;

					Sum += HeightMap[NeighborIndex];
					Count++;
				}
			}

			const float Average = Sum / static_cast<float>(Count);

			// Blend between original and smoothed
			SmoothedHeights[Index] = FMath::Lerp(HeightMap[Index], Average, SmoothingFactor);
		}
	}

	// Copy smoothed heights back
	HeightMap = SmoothedHeights;
}
