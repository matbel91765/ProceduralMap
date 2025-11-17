// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "SplineUtilities.h"
#include "GameFramework/Actor.h"

// ========== Spline Sampling ==========

FSplinePointData USplineUtilities::SampleSplineAtDistance(USplineComponent* SplineComponent, float Distance, bool bUseConstantVelocity)
{
	FSplinePointData Result;

	if (!SplineComponent)
	{
		return Result;
	}

	ESplineCoordinateSpace::Type CoordSpace = ESplineCoordinateSpace::World;

	Result.Location = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, CoordSpace);
	Result.Rotation = SplineComponent->GetRotationAtDistanceAlongSpline(Distance, CoordSpace);
	Result.Scale = SplineComponent->GetScaleAtDistanceAlongSpline(Distance);

	FVector Direction = SplineComponent->GetDirectionAtDistanceAlongSpline(Distance, CoordSpace);
	Result.ArriveTangent = Direction * 100.0f;
	Result.LeaveTangent = Direction * 100.0f;

	return Result;
}

FSplinePointData USplineUtilities::SampleSplineAtAlpha(USplineComponent* SplineComponent, float Alpha, bool bUseConstantVelocity)
{
	if (!SplineComponent)
	{
		return FSplinePointData();
	}

	float Distance = Alpha * SplineComponent->GetSplineLength();
	return SampleSplineAtDistance(SplineComponent, Distance, bUseConstantVelocity);
}

TArray<FSplinePointData> USplineUtilities::SampleSplineMultiple(USplineComponent* SplineComponent, int32 NumSamples, bool bEvenSpacing)
{
	TArray<FSplinePointData> Samples;

	if (!SplineComponent || NumSamples < 1)
	{
		return Samples;
	}

	float SplineLength = SplineComponent->GetSplineLength();

	for (int32 i = 0; i < NumSamples; i++)
	{
		float Alpha = i / FMath::Max(1.0f, (float)(NumSamples - 1));
		float Distance = Alpha * SplineLength;

		FSplinePointData Sample = SampleSplineAtDistance(SplineComponent, Distance, bEvenSpacing);
		Samples.Add(Sample);
	}

	return Samples;
}

// ========== Spline Point Extraction ==========

TArray<FVector> USplineUtilities::GetSplinePointsArray(USplineComponent* SplineComponent)
{
	TArray<FVector> Points;

	if (!SplineComponent)
	{
		return Points;
	}

	int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
	for (int32 i = 0; i < NumPoints; i++)
	{
		FVector Location = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		Points.Add(Location);
	}

	return Points;
}

TArray<FSplinePointData> USplineUtilities::GetSplinePointsDataArray(USplineComponent* SplineComponent)
{
	TArray<FSplinePointData> PointsData;

	if (!SplineComponent)
	{
		return PointsData;
	}

	int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
	for (int32 i = 0; i < NumPoints; i++)
	{
		FSplinePointData Data;
		Data.Location = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		Data.Rotation = SplineComponent->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::World);
		Data.Scale = SplineComponent->GetScaleAtSplinePoint(i);
		Data.ArriveTangent = SplineComponent->GetArriveTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
		Data.LeaveTangent = SplineComponent->GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::World);

		PointsData.Add(Data);
	}

	return PointsData;
}

TArray<FVector> USplineUtilities::GetEvenlySpacedPoints(USplineComponent* SplineComponent, float Spacing)
{
	TArray<FVector> Points;

	if (!SplineComponent || Spacing <= 0.0f)
	{
		return Points;
	}

	float SplineLength = SplineComponent->GetSplineLength();
	int32 NumPoints = FMath::CeilToInt(SplineLength / Spacing);

	for (int32 i = 0; i < NumPoints; i++)
	{
		float Distance = i * Spacing;
		if (Distance > SplineLength)
		{
			Distance = SplineLength;
		}

		FVector Location = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		Points.Add(Location);
	}

	return Points;
}

// ========== Spline Smoothing ==========

void USplineUtilities::SmoothSpline(USplineComponent* SplineComponent, float Smoothness, int32 Iterations)
{
	if (!SplineComponent || SplineComponent->GetNumberOfSplinePoints() < 3)
	{
		return;
	}

	Smoothness = FMath::Clamp(Smoothness, 0.0f, 1.0f);

	for (int32 Iter = 0; Iter < Iterations; Iter++)
	{
		TArray<FVector> OriginalPoints = GetSplinePointsArray(SplineComponent);

		for (int32 i = 1; i < OriginalPoints.Num() - 1; i++)
		{
			FVector Prev = OriginalPoints[i - 1];
			FVector Current = OriginalPoints[i];
			FVector Next = OriginalPoints[i + 1];

			FVector Averaged = (Prev + Current * 2.0f + Next) * 0.25f;
			FVector Smoothed = FMath::Lerp(Current, Averaged, Smoothness);

			SplineComponent->SetLocationAtSplinePoint(i, Smoothed, ESplineCoordinateSpace::World, false);
		}

		SplineComponent->UpdateSpline();
	}
}

void USplineUtilities::SmoothSplineCatmullRom(USplineComponent* SplineComponent, float TangentScale)
{
	if (!SplineComponent || SplineComponent->GetNumberOfSplinePoints() < 3)
	{
		return;
	}

	TArray<FVector> Points = GetSplinePointsArray(SplineComponent);

	for (int32 i = 1; i < Points.Num() - 1; i++)
	{
		FVector Prev = (i > 0) ? Points[i - 1] : Points[i];
		FVector Current = Points[i];
		FVector Next = (i < Points.Num() - 1) ? Points[i + 1] : Points[i];

		FVector Tangent = CalculateCatmullRomTangent(Prev, Current, Next, TangentScale);

		SplineComponent->SetTangentAtSplinePoint(i, Tangent, ESplineCoordinateSpace::World, false);
	}

	SplineComponent->UpdateSpline();
}

TArray<FVector> USplineUtilities::ApplyBezierSmoothing(const TArray<FVector>& Points, float Smoothness)
{
	if (Points.Num() < 4)
	{
		return Points;
	}

	TArray<FVector> SmoothedPoints;
	int32 SegmentsPerCurve = FMath::Max(1, FMath::FloorToInt(10.0f * Smoothness));

	for (int32 i = 0; i < Points.Num() - 3; i += 3)
	{
		FBezierCurveParams Params;
		Params.Point0 = Points[i];
		Params.Point1 = Points[i + 1];
		Params.Point2 = Points[i + 2];
		Params.Point3 = (i + 3 < Points.Num()) ? Points[i + 3] : Points.Last();

		for (int32 j = 0; j < SegmentsPerCurve; j++)
		{
			float T = j / (float)SegmentsPerCurve;
			FVector Point = EvaluateBezierCurve(Params, T);
			SmoothedPoints.Add(Point);
		}
	}

	SmoothedPoints.Add(Points.Last());

	return SmoothedPoints;
}

// ========== Spline Connections ==========

FSplineConnectionResult USplineUtilities::ConnectSplines(USplineComponent* Spline1, USplineComponent* Spline2, bool bConnectEnd1ToStart2, float BlendDistance)
{
	FSplineConnectionResult Result;

	if (!Spline1 || !Spline2)
	{
		Result.Message = TEXT("Invalid spline references");
		return Result;
	}

	int32 NumPoints1 = Spline1->GetNumberOfSplinePoints();
	int32 NumPoints2 = Spline2->GetNumberOfSplinePoints();

	if (NumPoints1 < 2 || NumPoints2 < 2)
	{
		Result.Message = TEXT("Splines must have at least 2 points");
		return Result;
	}

	FVector ConnectionPoint;

	if (bConnectEnd1ToStart2)
	{
		FVector EndPoint1 = Spline1->GetLocationAtSplinePoint(NumPoints1 - 1, ESplineCoordinateSpace::World);
		FVector StartPoint2 = Spline2->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

		ConnectionPoint = (EndPoint1 + StartPoint2) * 0.5f;

		// Add points from Spline2 to Spline1
		for (int32 i = 1; i < NumPoints2; i++)
		{
			FVector Point = Spline2->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
			FVector LocalPoint = Spline1->GetComponentTransform().InverseTransformPosition(Point);
			Spline1->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local, false);
		}
	}
	else
	{
		FVector StartPoint1 = Spline1->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector EndPoint2 = Spline2->GetLocationAtSplinePoint(NumPoints2 - 1, ESplineCoordinateSpace::World);

		ConnectionPoint = (StartPoint1 + EndPoint2) * 0.5f;
	}

	Spline1->UpdateSpline();

	Result.bSuccess = true;
	Result.ConnectionPoint = ConnectionPoint;
	Result.SplinePointIndex = NumPoints1 - 1;
	Result.Message = TEXT("Splines connected successfully");

	return Result;
}

void USplineUtilities::MergeSplines(const TArray<USplineComponent*>& Splines, USplineComponent* TargetSpline, bool bSmooth)
{
	if (!TargetSpline || Splines.Num() == 0)
	{
		return;
	}

	TargetSpline->ClearSplinePoints();

	for (USplineComponent* Spline : Splines)
	{
		if (!Spline)
		{
			continue;
		}

		TArray<FVector> Points = GetSplinePointsArray(Spline);

		for (const FVector& Point : Points)
		{
			FVector LocalPoint = TargetSpline->GetComponentTransform().InverseTransformPosition(Point);
			TargetSpline->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local, false);
		}
	}

	TargetSpline->UpdateSpline();

	if (bSmooth)
	{
		SmoothSpline(TargetSpline, 0.5f, 2);
	}
}

USplineComponent* USplineUtilities::CreateJunction(const TArray<USplineComponent*>& Splines, const FVector& JunctionCenter, float JunctionRadius)
{
	// This would create a circular junction spline
	// For now, return nullptr as this requires an actor context
	return nullptr;
}

// ========== Bezier Curve Utilities ==========

FVector USplineUtilities::EvaluateBezierCurve(const FBezierCurveParams& Params, float T)
{
	T = FMath::Clamp(T, 0.0f, 1.0f);

	float OneMinusT = 1.0f - T;
	float T2 = T * T;
	float T3 = T2 * T;
	float OneMinusT2 = OneMinusT * OneMinusT;
	float OneMinusT3 = OneMinusT2 * OneMinusT;

	FVector Result = Params.Point0 * OneMinusT3 +
		Params.Point1 * 3.0f * OneMinusT2 * T +
		Params.Point2 * 3.0f * OneMinusT * T2 +
		Params.Point3 * T3;

	return Result;
}

FVector USplineUtilities::GetBezierTangent(const FBezierCurveParams& Params, float T)
{
	T = FMath::Clamp(T, 0.0f, 1.0f);

	float OneMinusT = 1.0f - T;
	float OneMinusT2 = OneMinusT * OneMinusT;
	float T2 = T * T;

	FVector Tangent = Params.Point0 * (-3.0f * OneMinusT2) +
		Params.Point1 * (3.0f * OneMinusT2 - 6.0f * OneMinusT * T) +
		Params.Point2 * (6.0f * OneMinusT * T - 3.0f * T2) +
		Params.Point3 * (3.0f * T2);

	return Tangent.GetSafeNormal();
}

float USplineUtilities::CalculateBezierLength(const FBezierCurveParams& Params, int32 Samples)
{
	float Length = 0.0f;
	FVector PrevPoint = Params.Point0;

	for (int32 i = 1; i <= Samples; i++)
	{
		float T = i / (float)Samples;
		FVector Point = EvaluateBezierCurve(Params, T);

		Length += FVector::Dist(PrevPoint, Point);
		PrevPoint = Point;
	}

	return Length;
}

// ========== Spline Analysis ==========

float USplineUtilities::CalculateSplineCurvature(USplineComponent* SplineComponent, int32 SampleCount)
{
	if (!SplineComponent || SampleCount < 2)
	{
		return 0.0f;
	}

	float TotalCurvature = 0.0f;
	float SplineLength = SplineComponent->GetSplineLength();

	for (int32 i = 0; i < SampleCount - 1; i++)
	{
		float Distance1 = (i / (float)SampleCount) * SplineLength;
		float Distance2 = ((i + 1) / (float)SampleCount) * SplineLength;

		FVector Dir1 = SplineComponent->GetDirectionAtDistanceAlongSpline(Distance1, ESplineCoordinateSpace::World);
		FVector Dir2 = SplineComponent->GetDirectionAtDistanceAlongSpline(Distance2, ESplineCoordinateSpace::World);

		float AngleDiff = FMath::Acos(FVector::DotProduct(Dir1, Dir2));
		TotalCurvature += AngleDiff;
	}

	return TotalCurvature / SampleCount;
}

float USplineUtilities::FindMaxCurvature(USplineComponent* SplineComponent, float& OutDistance)
{
	if (!SplineComponent)
	{
		OutDistance = 0.0f;
		return 0.0f;
	}

	float MaxCurvature = 0.0f;
	OutDistance = 0.0f;

	float SplineLength = SplineComponent->GetSplineLength();
	int32 SampleCount = 100;

	for (int32 i = 0; i < SampleCount - 1; i++)
	{
		float Distance1 = (i / (float)SampleCount) * SplineLength;
		float Distance2 = ((i + 1) / (float)SampleCount) * SplineLength;

		FVector Dir1 = SplineComponent->GetDirectionAtDistanceAlongSpline(Distance1, ESplineCoordinateSpace::World);
		FVector Dir2 = SplineComponent->GetDirectionAtDistanceAlongSpline(Distance2, ESplineCoordinateSpace::World);

		float Curvature = FMath::Acos(FVector::DotProduct(Dir1, Dir2));

		if (Curvature > MaxCurvature)
		{
			MaxCurvature = Curvature;
			OutDistance = Distance1;
		}
	}

	return MaxCurvature;
}

float USplineUtilities::CalculateSegmentLength(USplineComponent* SplineComponent, float StartDistance, float EndDistance)
{
	if (!SplineComponent)
	{
		return 0.0f;
	}

	return FMath::Abs(EndDistance - StartDistance);
}

// ========== Spline Modification ==========

void USplineUtilities::ReverseSpline(USplineComponent* SplineComponent)
{
	if (!SplineComponent)
	{
		return;
	}

	TArray<FSplinePointData> PointsData = GetSplinePointsDataArray(SplineComponent);

	if (PointsData.Num() < 2)
	{
		return;
	}

	SplineComponent->ClearSplinePoints();

	for (int32 i = PointsData.Num() - 1; i >= 0; i--)
	{
		FVector LocalPos = SplineComponent->GetComponentTransform().InverseTransformPosition(PointsData[i].Location);
		SplineComponent->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	SplineComponent->UpdateSpline();
}

void USplineUtilities::SubdivideSpline(USplineComponent* SplineComponent, int32 SegmentsPerPoint)
{
	if (!SplineComponent || SegmentsPerPoint < 1)
	{
		return;
	}

	TArray<FVector> OriginalPoints = GetSplinePointsArray(SplineComponent);

	if (OriginalPoints.Num() < 2)
	{
		return;
	}

	SplineComponent->ClearSplinePoints();

	for (int32 i = 0; i < OriginalPoints.Num() - 1; i++)
	{
		FVector Start = OriginalPoints[i];
		FVector End = OriginalPoints[i + 1];

		for (int32 j = 0; j <= SegmentsPerPoint; j++)
		{
			float Alpha = j / (float)SegmentsPerPoint;
			FVector InterpPoint = FMath::Lerp(Start, End, Alpha);

			FVector LocalPos = SplineComponent->GetComponentTransform().InverseTransformPosition(InterpPoint);
			SplineComponent->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
		}
	}

	// Add last point
	FVector LastPoint = OriginalPoints.Last();
	FVector LocalLastPoint = SplineComponent->GetComponentTransform().InverseTransformPosition(LastPoint);
	SplineComponent->AddSplinePoint(LocalLastPoint, ESplineCoordinateSpace::Local, false);

	SplineComponent->UpdateSpline();
}

void USplineUtilities::SimplifySpline(USplineComponent* SplineComponent, float Tolerance)
{
	if (!SplineComponent || Tolerance <= 0.0f)
	{
		return;
	}

	TArray<FVector> Points = GetSplinePointsArray(SplineComponent);

	if (Points.Num() < 3)
	{
		return;
	}

	TArray<bool> PointsToKeep;
	PointsToKeep.Init(false, Points.Num());

	DouglasPeucker(Points, Tolerance, PointsToKeep);

	// Rebuild spline with simplified points
	SplineComponent->ClearSplinePoints();

	for (int32 i = 0; i < Points.Num(); i++)
	{
		if (PointsToKeep[i])
		{
			FVector LocalPos = SplineComponent->GetComponentTransform().InverseTransformPosition(Points[i]);
			SplineComponent->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
		}
	}

	SplineComponent->UpdateSpline();
}

USplineComponent* USplineUtilities::OffsetSpline(USplineComponent* SplineComponent, float OffsetDistance, bool bRightSide)
{
	// This requires an actor context to create a new spline component
	// Return nullptr for now
	return nullptr;
}

// ========== Spline Creation ==========

USplineComponent* USplineUtilities::CreateSplineFromPoints(AActor* Owner, const TArray<FVector>& Points, bool bClosedLoop)
{
	if (!Owner || Points.Num() < 2)
	{
		return nullptr;
	}

	USplineComponent* NewSpline = NewObject<USplineComponent>(Owner);
	NewSpline->RegisterComponent();
	NewSpline->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	NewSpline->SetClosedLoop(bClosedLoop);

	for (const FVector& Point : Points)
	{
		FVector LocalPos = Owner->GetActorTransform().InverseTransformPosition(Point);
		NewSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local, false);
	}

	NewSpline->UpdateSpline();

	return NewSpline;
}

USplineComponent* USplineUtilities::CloneSpline(USplineComponent* SourceSpline, AActor* Owner)
{
	if (!SourceSpline || !Owner)
	{
		return nullptr;
	}

	TArray<FVector> Points = GetSplinePointsArray(SourceSpline);
	bool bClosedLoop = SourceSpline->IsClosedLoop();

	return CreateSplineFromPoints(Owner, Points, bClosedLoop);
}

// ========== Distance and Projection ==========

FVector USplineUtilities::FindNearestPointOnSpline(USplineComponent* SplineComponent, const FVector& WorldPosition, float& OutDistance)
{
	if (!SplineComponent)
	{
		OutDistance = 0.0f;
		return FVector::ZeroVector;
	}

	float SplineLength = SplineComponent->GetSplineLength();
	int32 Samples = FMath::Max(50, FMath::CeilToInt(SplineLength / 100.0f));

	float NearestDistance = FLT_MAX;
	FVector NearestPoint = WorldPosition;
	OutDistance = 0.0f;

	for (int32 i = 0; i <= Samples; i++)
	{
		float Distance = (i / (float)Samples) * SplineLength;
		FVector Point = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		float Dist = FVector::Dist(Point, WorldPosition);
		if (Dist < NearestDistance)
		{
			NearestDistance = Dist;
			NearestPoint = Point;
			OutDistance = Distance;
		}
	}

	return NearestPoint;
}

float USplineUtilities::CalculateDistanceToSpline(USplineComponent* SplineComponent, const FVector& WorldPosition)
{
	float OutDistance;
	FVector NearestPoint = FindNearestPointOnSpline(SplineComponent, WorldPosition, OutDistance);
	return FVector::Dist(NearestPoint, WorldPosition);
}

bool USplineUtilities::IsPointNearSpline(USplineComponent* SplineComponent, const FVector& WorldPosition, float Threshold)
{
	float Distance = CalculateDistanceToSpline(SplineComponent, WorldPosition);
	return Distance <= Threshold;
}

// ========== Private Helper Functions ==========

FVector USplineUtilities::CalculateCatmullRomTangent(const FVector& Previous, const FVector& Current, const FVector& Next, float Alpha)
{
	FVector Tangent = (Next - Previous) * Alpha;
	return Tangent;
}

void USplineUtilities::DouglasPeucker(const TArray<FVector>& Points, float Tolerance, TArray<bool>& PointsToKeep)
{
	if (Points.Num() < 3)
	{
		PointsToKeep.Init(true, Points.Num());
		return;
	}

	PointsToKeep.Init(false, Points.Num());
	PointsToKeep[0] = true;
	PointsToKeep[Points.Num() - 1] = true;

	// Find point with maximum distance
	float MaxDistance = 0.0f;
	int32 MaxIndex = 0;

	for (int32 i = 1; i < Points.Num() - 1; i++)
	{
		float Distance = PerpendicularDistance(Points[i], Points[0], Points.Last());
		if (Distance > MaxDistance)
		{
			MaxDistance = Distance;
			MaxIndex = i;
		}
	}

	// If max distance is greater than tolerance, recursively simplify
	if (MaxDistance > Tolerance)
	{
		PointsToKeep[MaxIndex] = true;

		// Recursively simplify segments (simplified here - full implementation would split)
		for (int32 i = 1; i < Points.Num() - 1; i++)
		{
			float Distance = PerpendicularDistance(Points[i], Points[0], Points.Last());
			if (Distance > Tolerance * 0.5f)
			{
				PointsToKeep[i] = true;
			}
		}
	}
}

float USplineUtilities::PerpendicularDistance(const FVector& Point, const FVector& LineStart, const FVector& LineEnd)
{
	FVector LineDir = LineEnd - LineStart;
	float LineLength = LineDir.Size();

	if (LineLength < KINDA_SMALL_NUMBER)
	{
		return FVector::Dist(Point, LineStart);
	}

	LineDir /= LineLength;

	FVector PointToStart = Point - LineStart;
	float Projection = FVector::DotProduct(PointToStart, LineDir);

	if (Projection < 0.0f)
	{
		return FVector::Dist(Point, LineStart);
	}
	else if (Projection > LineLength)
	{
		return FVector::Dist(Point, LineEnd);
	}

	FVector ClosestPoint = LineStart + LineDir * Projection;
	return FVector::Dist(Point, ClosestPoint);
}
