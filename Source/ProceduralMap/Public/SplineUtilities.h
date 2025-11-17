// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "SplineUtilities.generated.h"

/**
 * Spline point data structure
 */
USTRUCT(BlueprintType)
struct FSplinePointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ArriveTangent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeaveTangent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale;

	FSplinePointData()
		: Location(FVector::ZeroVector)
		, ArriveTangent(FVector::ForwardVector)
		, LeaveTangent(FVector::ForwardVector)
		, Rotation(FRotator::ZeroRotator)
		, Scale(FVector::OneVector)
	{}
};

/**
 * Bezier curve parameters
 */
USTRUCT(BlueprintType)
struct FBezierCurveParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Point0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Point1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Point2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Point3;

	FBezierCurveParams()
		: Point0(FVector::ZeroVector)
		, Point1(FVector::ZeroVector)
		, Point2(FVector::ZeroVector)
		, Point3(FVector::ZeroVector)
	{}
};

/**
 * Spline connection result
 */
USTRUCT(BlueprintType)
struct FSplineConnectionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bSuccess;

	UPROPERTY(BlueprintReadWrite)
	FVector ConnectionPoint;

	UPROPERTY(BlueprintReadWrite)
	int32 SplinePointIndex;

	UPROPERTY(BlueprintReadWrite)
	FString Message;

	FSplineConnectionResult()
		: bSuccess(false)
		, ConnectionPoint(FVector::ZeroVector)
		, SplinePointIndex(-1)
		, Message(TEXT(""))
	{}
};

/**
 * Static utility class for spline manipulation
 * Blueprint function library providing helper functions for spline operations
 */
UCLASS()
class PROCEDURALMAP_API USplineUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========== Spline Sampling ==========

	/**
	 * Sample spline at a specific distance along its length
	 * @param SplineComponent Spline to sample
	 * @param Distance Distance along spline in cm
	 * @param bUseConstantVelocity Use constant velocity interpolation
	 * @return Spline point data at distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Sampling")
	static FSplinePointData SampleSplineAtDistance(USplineComponent* SplineComponent, float Distance, bool bUseConstantVelocity = true);

	/**
	 * Sample spline at normalized position (0-1)
	 * @param SplineComponent Spline to sample
	 * @param Alpha Normalized position (0-1)
	 * @param bUseConstantVelocity Use constant velocity interpolation
	 * @return Spline point data at alpha
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Sampling")
	static FSplinePointData SampleSplineAtAlpha(USplineComponent* SplineComponent, float Alpha, bool bUseConstantVelocity = true);

	/**
	 * Sample spline at multiple points
	 * @param SplineComponent Spline to sample
	 * @param NumSamples Number of samples to take
	 * @param bEvenSpacing Use even spacing along spline
	 * @return Array of spline point data
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Sampling")
	static TArray<FSplinePointData> SampleSplineMultiple(USplineComponent* SplineComponent, int32 NumSamples, bool bEvenSpacing = true);

	// ========== Spline Point Extraction ==========

	/**
	 * Get all spline points as array
	 * @param SplineComponent Spline component
	 * @return Array of spline point locations
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Points")
	static TArray<FVector> GetSplinePointsArray(USplineComponent* SplineComponent);

	/**
	 * Get spline points with full data
	 * @param SplineComponent Spline component
	 * @return Array of complete spline point data
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Points")
	static TArray<FSplinePointData> GetSplinePointsDataArray(USplineComponent* SplineComponent);

	/**
	 * Get evenly spaced points along spline
	 * @param SplineComponent Spline component
	 * @param Spacing Distance between points in cm
	 * @return Array of evenly spaced locations
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Points")
	static TArray<FVector> GetEvenlySpacedPoints(USplineComponent* SplineComponent, float Spacing);

	// ========== Spline Smoothing ==========

	/**
	 * Smooth spline using Bezier curves
	 * @param SplineComponent Spline to smooth
	 * @param Smoothness Smoothness factor (0-1)
	 * @param Iterations Number of smoothing iterations
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Smoothing")
	static void SmoothSpline(USplineComponent* SplineComponent, float Smoothness = 0.5f, int32 Iterations = 3);

	/**
	 * Smooth spline using Catmull-Rom interpolation
	 * @param SplineComponent Spline to smooth
	 * @param TangentScale Scale factor for tangents
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Smoothing")
	static void SmoothSplineCatmullRom(USplineComponent* SplineComponent, float TangentScale = 0.5f);

	/**
	 * Apply Bezier smoothing to path points
	 * @param Points Input points
	 * @param Smoothness Smoothness factor
	 * @return Smoothed points
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Smoothing")
	static TArray<FVector> ApplyBezierSmoothing(const TArray<FVector>& Points, float Smoothness = 0.5f);

	// ========== Spline Connections ==========

	/**
	 * Connect two splines at their endpoints
	 * @param Spline1 First spline
	 * @param Spline2 Second spline
	 * @param bConnectEnd1ToStart2 Connect end of Spline1 to start of Spline2
	 * @param BlendDistance Distance to blend over
	 * @return Connection result
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Connections")
	static FSplineConnectionResult ConnectSplines(USplineComponent* Spline1, USplineComponent* Spline2, bool bConnectEnd1ToStart2 = true, float BlendDistance = 500.0f);

	/**
	 * Merge multiple splines into one
	 * @param Splines Array of splines to merge
	 * @param TargetSpline Spline to merge into
	 * @param bSmooth Apply smoothing at junctions
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Connections")
	static void MergeSplines(const TArray<USplineComponent*>& Splines, USplineComponent* TargetSpline, bool bSmooth = true);

	/**
	 * Create junction between multiple splines
	 * @param Splines Splines to connect
	 * @param JunctionCenter Center point of junction
	 * @param JunctionRadius Radius of junction
	 * @return Junction spline
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Connections")
	static USplineComponent* CreateJunction(const TArray<USplineComponent*>& Splines, const FVector& JunctionCenter, float JunctionRadius);

	// ========== Bezier Curve Utilities ==========

	/**
	 * Evaluate cubic Bezier curve at parameter t
	 * @param Params Bezier curve parameters
	 * @param T Parameter (0-1)
	 * @return Point on curve
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Bezier")
	static FVector EvaluateBezierCurve(const FBezierCurveParams& Params, float T);

	/**
	 * Get tangent of Bezier curve at parameter t
	 * @param Params Bezier curve parameters
	 * @param T Parameter (0-1)
	 * @return Tangent vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Bezier")
	static FVector GetBezierTangent(const FBezierCurveParams& Params, float T);

	/**
	 * Calculate Bezier curve length
	 * @param Params Bezier curve parameters
	 * @param Samples Number of samples for approximation
	 * @return Approximate length
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Bezier")
	static float CalculateBezierLength(const FBezierCurveParams& Params, int32 Samples = 20);

	// ========== Spline Analysis ==========

	/**
	 * Calculate total curvature of spline
	 * @param SplineComponent Spline to analyze
	 * @param SampleCount Number of samples
	 * @return Average curvature
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Analysis")
	static float CalculateSplineCurvature(USplineComponent* SplineComponent, int32 SampleCount = 50);

	/**
	 * Find point of maximum curvature
	 * @param SplineComponent Spline to analyze
	 * @param OutDistance Distance along spline of max curvature
	 * @return Maximum curvature value
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Analysis")
	static float FindMaxCurvature(USplineComponent* SplineComponent, float& OutDistance);

	/**
	 * Calculate length of spline segment
	 * @param SplineComponent Spline component
	 * @param StartDistance Start distance
	 * @param EndDistance End distance
	 * @return Segment length
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Analysis")
	static float CalculateSegmentLength(USplineComponent* SplineComponent, float StartDistance, float EndDistance);

	// ========== Spline Modification ==========

	/**
	 * Reverse spline direction
	 * @param SplineComponent Spline to reverse
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Modification")
	static void ReverseSpline(USplineComponent* SplineComponent);

	/**
	 * Subdivide spline by adding intermediate points
	 * @param SplineComponent Spline to subdivide
	 * @param SegmentsPerPoint Number of segments to add between each point
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Modification")
	static void SubdivideSpline(USplineComponent* SplineComponent, int32 SegmentsPerPoint = 2);

	/**
	 * Simplify spline by removing redundant points
	 * @param SplineComponent Spline to simplify
	 * @param Tolerance Simplification tolerance
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Modification")
	static void SimplifySpline(USplineComponent* SplineComponent, float Tolerance = 10.0f);

	/**
	 * Offset spline perpendicular to its direction
	 * @param SplineComponent Spline to offset
	 * @param OffsetDistance Offset distance
	 * @param bRightSide Offset to right (true) or left (false)
	 * @return New offset spline
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Modification")
	static USplineComponent* OffsetSpline(USplineComponent* SplineComponent, float OffsetDistance, bool bRightSide = true);

	// ========== Spline Creation ==========

	/**
	 * Create spline from points array
	 * @param Owner Actor to attach spline to
	 * @param Points Array of points
	 * @param bClosedLoop Create closed loop
	 * @return Created spline component
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Creation")
	static USplineComponent* CreateSplineFromPoints(AActor* Owner, const TArray<FVector>& Points, bool bClosedLoop = false);

	/**
	 * Clone spline component
	 * @param SourceSpline Spline to clone
	 * @param Owner Actor to attach clone to
	 * @return Cloned spline
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Creation")
	static USplineComponent* CloneSpline(USplineComponent* SourceSpline, AActor* Owner);

	// ========== Distance and Projection ==========

	/**
	 * Find nearest point on spline to world position
	 * @param SplineComponent Spline to query
	 * @param WorldPosition Position to find nearest point to
	 * @param OutDistance Distance along spline
	 * @return Nearest point on spline
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Distance")
	static FVector FindNearestPointOnSpline(USplineComponent* SplineComponent, const FVector& WorldPosition, float& OutDistance);

	/**
	 * Calculate distance from point to spline
	 * @param SplineComponent Spline to query
	 * @param WorldPosition Position to measure from
	 * @return Distance to nearest point on spline
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Distance")
	static float CalculateDistanceToSpline(USplineComponent* SplineComponent, const FVector& WorldPosition);

	/**
	 * Check if point is near spline
	 * @param SplineComponent Spline to query
	 * @param WorldPosition Position to check
	 * @param Threshold Distance threshold
	 * @return True if within threshold
	 */
	UFUNCTION(BlueprintCallable, Category = "Spline Utilities|Distance")
	static bool IsPointNearSpline(USplineComponent* SplineComponent, const FVector& WorldPosition, float Threshold);

private:
	/**
	 * Calculate Catmull-Rom tangent
	 */
	static FVector CalculateCatmullRomTangent(const FVector& Previous, const FVector& Current, const FVector& Next, float Alpha);

	/**
	 * Douglas-Peucker line simplification algorithm
	 */
	static void DouglasPeucker(const TArray<FVector>& Points, float Tolerance, TArray<bool>& PointsToKeep);

	/**
	 * Perpendicular distance from point to line segment
	 */
	static float PerpendicularDistance(const FVector& Point, const FVector& LineStart, const FVector& LineEnd);
};
