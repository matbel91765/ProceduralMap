// Copyright Epic Games, Inc. All Rights Reserved.

#include "TriplanarMapping.h"
#include "Math/UnrealMathUtility.h"

UTriplanarMappingHelper::UTriplanarMappingHelper()
{
}

FTriplanarBlendWeights UTriplanarMappingHelper::CalculateBlendWeights(
	const FVector& WorldNormal,
	float BlendSharpness,
	bool bNormalize)
{
	// Use absolute values of normal components
	FVector AbsNormal = WorldNormal.GetAbs();

	// Apply power function for sharpness
	float XWeight = FMath::Pow(AbsNormal.X, BlendSharpness);
	float YWeight = FMath::Pow(AbsNormal.Y, BlendSharpness);
	float ZWeight = FMath::Pow(AbsNormal.Z, BlendSharpness);

	FTriplanarBlendWeights Weights(XWeight, YWeight, ZWeight);

	// Normalize if requested
	if (bNormalize)
	{
		Weights = NormalizeWeights(Weights);
	}

	return Weights;
}

FTriplanarBlendWeights UTriplanarMappingHelper::CalculateBlendWeightsCustomAxes(
	const FVector& WorldNormal,
	const FVector& XAxis,
	const FVector& YAxis,
	const FVector& ZAxis,
	float BlendSharpness,
	bool bNormalize)
{
	// Calculate dot products with custom axes
	float XDot = FMath::Abs(FVector::DotProduct(WorldNormal, XAxis.GetSafeNormal()));
	float YDot = FMath::Abs(FVector::DotProduct(WorldNormal, YAxis.GetSafeNormal()));
	float ZDot = FMath::Abs(FVector::DotProduct(WorldNormal, ZAxis.GetSafeNormal()));

	// Apply power function for sharpness
	float XWeight = FMath::Pow(XDot, BlendSharpness);
	float YWeight = FMath::Pow(YDot, BlendSharpness);
	float ZWeight = FMath::Pow(ZDot, BlendSharpness);

	FTriplanarBlendWeights Weights(XWeight, YWeight, ZWeight);

	// Normalize if requested
	if (bNormalize)
	{
		Weights = NormalizeWeights(Weights);
	}

	return Weights;
}

FVector2D UTriplanarMappingHelper::GetXAxisUV(
	const FVector& WorldPosition,
	const FTriplanarAxisConfig& Config,
	float GlobalScale)
{
	// X axis projection uses Y and Z coordinates
	FVector2D UV = FVector2D(WorldPosition.Y, WorldPosition.Z);

	// Apply global scale
	UV *= GlobalScale;

	// Apply per-axis scale
	UV.X *= Config.Scale.X;
	UV.Y *= Config.Scale.Y;

	// Apply rotation
	if (!FMath::IsNearlyZero(Config.Rotation))
	{
		UV = RotateUV(UV, Config.Rotation);
	}

	// Apply offset
	UV += Config.Offset;

	return UV;
}

FVector2D UTriplanarMappingHelper::GetYAxisUV(
	const FVector& WorldPosition,
	const FTriplanarAxisConfig& Config,
	float GlobalScale)
{
	// Y axis projection uses X and Z coordinates
	FVector2D UV = FVector2D(WorldPosition.X, WorldPosition.Z);

	// Apply global scale
	UV *= GlobalScale;

	// Apply per-axis scale
	UV.X *= Config.Scale.X;
	UV.Y *= Config.Scale.Y;

	// Apply rotation
	if (!FMath::IsNearlyZero(Config.Rotation))
	{
		UV = RotateUV(UV, Config.Rotation);
	}

	// Apply offset
	UV += Config.Offset;

	return UV;
}

FVector2D UTriplanarMappingHelper::GetZAxisUV(
	const FVector& WorldPosition,
	const FTriplanarAxisConfig& Config,
	float GlobalScale)
{
	// Z axis projection uses X and Y coordinates
	FVector2D UV = FVector2D(WorldPosition.X, WorldPosition.Y);

	// Apply global scale
	UV *= GlobalScale;

	// Apply per-axis scale
	UV.X *= Config.Scale.X;
	UV.Y *= Config.Scale.Y;

	// Apply rotation
	if (!FMath::IsNearlyZero(Config.Rotation))
	{
		UV = RotateUV(UV, Config.Rotation);
	}

	// Apply offset
	UV += Config.Offset;

	return UV;
}

FVector2D UTriplanarMappingHelper::RotateUV(const FVector2D& UV, float RotationDegrees)
{
	float RotationRadians = FMath::DegreesToRadians(RotationDegrees);
	float CosAngle = FMath::Cos(RotationRadians);
	float SinAngle = FMath::Sin(RotationRadians);

	return ApplyRotation2D(UV, CosAngle, SinAngle);
}

FVector UTriplanarMappingHelper::GetWorldAlignedPosition(const FVector& WorldPosition)
{
	// For world-aligned texturing, simply return the world position
	// Typically scaled down to reasonable texture coordinates
	return WorldPosition;
}

FVector UTriplanarMappingHelper::GetObjectAlignedPosition(
	const FVector& ObjectSpacePosition,
	const FVector& ObjectBounds)
{
	// Normalize object-space position by bounds to get 0-1 range
	FVector NormalizedPosition = ObjectSpacePosition;

	if (!FMath::IsNearlyZero(ObjectBounds.X))
		NormalizedPosition.X /= ObjectBounds.X;

	if (!FMath::IsNearlyZero(ObjectBounds.Y))
		NormalizedPosition.Y /= ObjectBounds.Y;

	if (!FMath::IsNearlyZero(ObjectBounds.Z))
		NormalizedPosition.Z /= ObjectBounds.Z;

	return NormalizedPosition;
}

FVector UTriplanarMappingHelper::BlendTriplanarNormals(
	const FVector& NormalX,
	const FVector& NormalY,
	const FVector& NormalZ,
	const FTriplanarBlendWeights& BlendWeights)
{
	// Blend the three normal samples using blend weights
	FVector BlendedNormal =
		NormalX * BlendWeights.XWeight +
		NormalY * BlendWeights.YWeight +
		NormalZ * BlendWeights.ZWeight;

	// Normalize the result
	BlendedNormal.Normalize();

	return BlendedNormal;
}

FTriplanarBlendWeights UTriplanarMappingHelper::CalculateDerivativeBlendWeights(
	const FVector& WorldNormal,
	const FVector& ddx,
	const FVector& ddy,
	float BlendSharpness)
{
	// Enhanced blend weights that take into account surface derivatives
	// This helps reduce texture swimming and improves normal map blending

	FVector AbsNormal = WorldNormal.GetAbs();
	FVector Absddx = ddx.GetAbs();
	FVector Absddy = ddy.GetAbs();

	// Combine normal and derivative information
	FVector CombinedWeight = AbsNormal + (Absddx + Absddy) * 0.1f;

	// Apply power function for sharpness
	float XWeight = FMath::Pow(CombinedWeight.X, BlendSharpness);
	float YWeight = FMath::Pow(CombinedWeight.Y, BlendSharpness);
	float ZWeight = FMath::Pow(CombinedWeight.Z, BlendSharpness);

	FTriplanarBlendWeights Weights(XWeight, YWeight, ZWeight);

	// Always normalize derivative-based weights
	return NormalizeWeights(Weights);
}

float UTriplanarMappingHelper::GetDistanceBlendFactor(
	const FVector& CameraPosition,
	const FVector& SurfacePosition,
	float NearDistance,
	float FarDistance)
{
	float Distance = FVector::Dist(CameraPosition, SurfacePosition);

	// Clamp and normalize distance to 0-1 range
	float BlendFactor = FMath::Clamp(
		(Distance - NearDistance) / FMath::Max(FarDistance - NearDistance, 1.0f),
		0.0f,
		1.0f
	);

	return BlendFactor;
}

FVector2D UTriplanarMappingHelper::ApplyRotation2D(const FVector2D& UV, float CosAngle, float SinAngle)
{
	// Apply 2D rotation matrix
	// [ cos  -sin ] [ x ]
	// [ sin   cos ] [ y ]

	FVector2D RotatedUV;
	RotatedUV.X = UV.X * CosAngle - UV.Y * SinAngle;
	RotatedUV.Y = UV.X * SinAngle + UV.Y * CosAngle;

	return RotatedUV;
}

FTriplanarBlendWeights UTriplanarMappingHelper::NormalizeWeights(const FTriplanarBlendWeights& Weights)
{
	float TotalWeight = Weights.XWeight + Weights.YWeight + Weights.ZWeight;

	if (FMath::IsNearlyZero(TotalWeight))
	{
		// If all weights are zero, return equal distribution
		return FTriplanarBlendWeights(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);
	}

	// Normalize weights to sum to 1.0
	return FTriplanarBlendWeights(
		Weights.XWeight / TotalWeight,
		Weights.YWeight / TotalWeight,
		Weights.ZWeight / TotalWeight
	);
}
