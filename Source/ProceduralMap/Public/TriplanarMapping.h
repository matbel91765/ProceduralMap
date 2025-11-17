// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TriplanarMapping.generated.h"

/**
 * Triplanar mapping mode
 */
UENUM(BlueprintType)
enum class ETriplanarMappingMode : uint8
{
	WorldAligned UMETA(DisplayName = "World Aligned"),
	ObjectAligned UMETA(DisplayName = "Object Aligned"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Triplanar axis configuration
 */
USTRUCT(BlueprintType)
struct FTriplanarAxisConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar")
	FVector2D Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar")
	float Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar")
	FVector2D Scale;

	FTriplanarAxisConfig()
		: Offset(FVector2D::ZeroVector)
		, Rotation(0.0f)
		, Scale(FVector2D(1.0f, 1.0f))
	{
	}
};

/**
 * Complete triplanar mapping configuration
 */
USTRUCT(BlueprintType)
struct FTriplanarMappingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar")
	ETriplanarMappingMode MappingMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar")
	float BlendSharpness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar|X Axis")
	FTriplanarAxisConfig XAxisConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar|Y Axis")
	FTriplanarAxisConfig YAxisConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar|Z Axis")
	FTriplanarAxisConfig ZAxisConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar")
	float GlobalScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triplanar")
	bool bNormalizeBlendWeights;

	FTriplanarMappingConfig()
		: MappingMode(ETriplanarMappingMode::WorldAligned)
		, BlendSharpness(8.0f)
		, GlobalScale(1.0f)
		, bNormalizeBlendWeights(true)
	{
	}
};

/**
 * Triplanar blend weights result
 */
USTRUCT(BlueprintType)
struct FTriplanarBlendWeights
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Triplanar")
	float XWeight;

	UPROPERTY(BlueprintReadOnly, Category = "Triplanar")
	float YWeight;

	UPROPERTY(BlueprintReadOnly, Category = "Triplanar")
	float ZWeight;

	FTriplanarBlendWeights()
		: XWeight(0.0f)
		, YWeight(0.0f)
		, ZWeight(0.0f)
	{
	}

	FTriplanarBlendWeights(float InX, float InY, float InZ)
		: XWeight(InX)
		, YWeight(InY)
		, ZWeight(InZ)
	{
	}
};

/**
 * Triplanar Mapping Helper
 * Utility class for calculating triplanar projection blend weights and coordinates
 */
UCLASS(BlueprintType)
class PROCEDURALMAP_API UTriplanarMappingHelper : public UObject
{
	GENERATED_BODY()

public:
	UTriplanarMappingHelper();

	/**
	 * Calculate blend weights based on surface normal
	 * @param WorldNormal - Surface normal in world space
	 * @param BlendSharpness - Sharpness of blend between axes (higher = sharper transition)
	 * @param bNormalize - Whether to normalize weights to sum to 1.0
	 * @return Blend weights for X, Y, and Z axes
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FTriplanarBlendWeights CalculateBlendWeights(
		const FVector& WorldNormal,
		float BlendSharpness = 8.0f,
		bool bNormalize = true
	);

	/**
	 * Calculate blend weights with custom axis vectors
	 * @param WorldNormal - Surface normal in world space
	 * @param XAxis - Custom X axis direction
	 * @param YAxis - Custom Y axis direction
	 * @param ZAxis - Custom Z axis direction
	 * @param BlendSharpness - Sharpness of blend
	 * @param bNormalize - Whether to normalize weights
	 * @return Blend weights for custom axes
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FTriplanarBlendWeights CalculateBlendWeightsCustomAxes(
		const FVector& WorldNormal,
		const FVector& XAxis,
		const FVector& YAxis,
		const FVector& ZAxis,
		float BlendSharpness = 8.0f,
		bool bNormalize = true
	);

	/**
	 * Get UV coordinates for X axis projection
	 * @param WorldPosition - Position in world space
	 * @param Config - Axis configuration (offset, rotation, scale)
	 * @param GlobalScale - Global scale multiplier
	 * @return UV coordinates for X axis
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FVector2D GetXAxisUV(
		const FVector& WorldPosition,
		const FTriplanarAxisConfig& Config,
		float GlobalScale = 1.0f
	);

	/**
	 * Get UV coordinates for Y axis projection
	 * @param WorldPosition - Position in world space
	 * @param Config - Axis configuration (offset, rotation, scale)
	 * @param GlobalScale - Global scale multiplier
	 * @return UV coordinates for Y axis
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FVector2D GetYAxisUV(
		const FVector& WorldPosition,
		const FTriplanarAxisConfig& Config,
		float GlobalScale = 1.0f
	);

	/**
	 * Get UV coordinates for Z axis projection
	 * @param WorldPosition - Position in world space
	 * @param Config - Axis configuration (offset, rotation, scale)
	 * @param GlobalScale - Global scale multiplier
	 * @return UV coordinates for Z axis
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FVector2D GetZAxisUV(
		const FVector& WorldPosition,
		const FTriplanarAxisConfig& Config,
		float GlobalScale = 1.0f
	);

	/**
	 * Apply rotation to UV coordinates
	 * @param UV - Input UV coordinates
	 * @param RotationDegrees - Rotation in degrees
	 * @return Rotated UV coordinates
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FVector2D RotateUV(const FVector2D& UV, float RotationDegrees);

	/**
	 * Get world-aligned position for triplanar mapping
	 * @param WorldPosition - Position in world space
	 * @return Position normalized for texture mapping
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FVector GetWorldAlignedPosition(const FVector& WorldPosition);

	/**
	 * Get object-aligned position for triplanar mapping
	 * @param ObjectSpacePosition - Position in object space
	 * @param ObjectBounds - Bounds of the object
	 * @return Position normalized for texture mapping
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FVector GetObjectAlignedPosition(const FVector& ObjectSpacePosition, const FVector& ObjectBounds);

	/**
	 * Blend normal maps from triplanar projection
	 * @param NormalX - Normal map sample from X axis
	 * @param NormalY - Normal map sample from Y axis
	 * @param NormalZ - Normal map sample from Z axis
	 * @param BlendWeights - Blend weights for each axis
	 * @return Blended normal vector
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FVector BlendTriplanarNormals(
		const FVector& NormalX,
		const FVector& NormalY,
		const FVector& NormalZ,
		const FTriplanarBlendWeights& BlendWeights
	);

	/**
	 * Calculate derivative-based blend weights (for better normal map blending)
	 * @param WorldNormal - Surface normal in world space
	 * @param ddx - Derivative in X direction
	 * @param ddy - Derivative in Y direction
	 * @param BlendSharpness - Sharpness of blend
	 * @return Enhanced blend weights
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static FTriplanarBlendWeights CalculateDerivativeBlendWeights(
		const FVector& WorldNormal,
		const FVector& ddx,
		const FVector& ddy,
		float BlendSharpness = 8.0f
	);

	/**
	 * Get texture coordinate scale factor based on world-space distance
	 * Used for distance-based detail fading
	 * @param CameraPosition - Camera position in world space
	 * @param SurfacePosition - Surface position in world space
	 * @param NearDistance - Distance where near detail is at full strength
	 * @param FarDistance - Distance where far detail is at full strength
	 * @return Blend factor between 0 (near) and 1 (far)
	 */
	UFUNCTION(BlueprintPure, Category = "Triplanar Mapping")
	static float GetDistanceBlendFactor(
		const FVector& CameraPosition,
		const FVector& SurfacePosition,
		float NearDistance,
		float FarDistance
	);

protected:
	/**
	 * Apply 2D rotation matrix to UV coordinates
	 */
	static FVector2D ApplyRotation2D(const FVector2D& UV, float CosAngle, float SinAngle);

	/**
	 * Normalize blend weights to sum to 1.0
	 */
	static FTriplanarBlendWeights NormalizeWeights(const FTriplanarBlendWeights& Weights);
};
