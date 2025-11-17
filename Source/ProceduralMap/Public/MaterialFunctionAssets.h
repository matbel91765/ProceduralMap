// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "MaterialFunctionAssets.generated.h"

/**
 * Material function template type
 */
UENUM(BlueprintType)
enum class EMaterialFunctionTemplate : uint8
{
	TriplanarProjection UMETA(DisplayName = "Triplanar Projection"),
	HeightBlending UMETA(DisplayName = "Height Blending"),
	SlopeMapping UMETA(DisplayName = "Slope-Based Mapping"),
	DistanceFading UMETA(DisplayName = "Distance-Based Detail Fading"),
	NormalBlending UMETA(DisplayName = "Normal Blending"),
	WorldAlignedTexture UMETA(DisplayName = "World-Aligned Texture")
};

/**
 * Material function input parameter
 */
USTRUCT(BlueprintType)
struct FMaterialFunctionInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString InputName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString InputType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FLinearColor DefaultValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	bool bIsTexture;

	FMaterialFunctionInput()
		: InputName(TEXT("Input"))
		, InputType(TEXT("Float"))
		, Description(TEXT(""))
		, DefaultValue(FLinearColor::Black)
		, bIsTexture(false)
	{
	}
};

/**
 * Material function output parameter
 */
USTRUCT(BlueprintType)
struct FMaterialFunctionOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString OutputName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString OutputType;

	FMaterialFunctionOutput()
		: OutputName(TEXT("Result"))
		, OutputType(TEXT("Float3"))
	{
	}
};

/**
 * Material function configuration
 */
USTRUCT(BlueprintType)
struct FMaterialFunctionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString FunctionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	EMaterialFunctionTemplate Template;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	TArray<FMaterialFunctionInput> Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	TArray<FMaterialFunctionOutput> Outputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Function")
	FString ShaderCodePath;

	FMaterialFunctionConfig()
		: FunctionName(TEXT("MaterialFunction"))
		, Description(TEXT(""))
		, Template(EMaterialFunctionTemplate::TriplanarProjection)
		, ShaderCodePath(TEXT(""))
	{
	}
};

/**
 * Material Function Asset Helper
 * Creates material functions programmatically for terrain rendering
 */
UCLASS(BlueprintType)
class PROCEDURALMAP_API UMaterialFunctionAssetHelper : public UObject
{
	GENERATED_BODY()

public:
	UMaterialFunctionAssetHelper();

	/**
	 * Create a material function from template
	 * @param Config - Function configuration
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateMaterialFunction(const FMaterialFunctionConfig& Config);

	/**
	 * Create triplanar projection material function
	 * @param FunctionName - Name for the function
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateTriplanarProjectionFunction(const FString& FunctionName = TEXT("TriplanarProjection"));

	/**
	 * Create height blending material function
	 * @param FunctionName - Name for the function
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateHeightBlendingFunction(const FString& FunctionName = TEXT("HeightBlending"));

	/**
	 * Create slope-based mapping material function
	 * @param FunctionName - Name for the function
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateSlopeMappingFunction(const FString& FunctionName = TEXT("SlopeMapping"));

	/**
	 * Create distance-based detail fading material function
	 * @param FunctionName - Name for the function
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateDistanceFadingFunction(const FString& FunctionName = TEXT("DistanceFading"));

	/**
	 * Create normal blending material function
	 * @param FunctionName - Name for the function
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateNormalBlendingFunction(const FString& FunctionName = TEXT("NormalBlending"));

	/**
	 * Create world-aligned texture material function
	 * @param FunctionName - Name for the function
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateWorldAlignedTextureFunction(const FString& FunctionName = TEXT("WorldAlignedTexture"));

	/**
	 * Export material function to .uasset file
	 * @param MaterialFunction - Function to export
	 * @param ExportPath - Path to export to (e.g., "/Game/Materials/Functions/")
	 * @param FileName - File name without extension
	 * @return Success status
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	bool ExportMaterialFunction(
		UMaterialFunction* MaterialFunction,
		const FString& ExportPath,
		const FString& FileName
	);

	/**
	 * Get default configuration for a template type
	 * @param Template - Template type
	 * @return Default configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Material Function")
	static FMaterialFunctionConfig GetDefaultConfig(EMaterialFunctionTemplate Template);

	/**
	 * Load shader code from file
	 * @param ShaderFilePath - Path to shader file
	 * @return Shader code as string
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	static FString LoadShaderCode(const FString& ShaderFilePath);

	/**
	 * Create custom shader code material function
	 * @param FunctionName - Name for the function
	 * @param ShaderCode - Custom HLSL shader code
	 * @param Inputs - Input parameters
	 * @param Outputs - Output parameters
	 * @return Created material function
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Function")
	UMaterialFunction* CreateCustomShaderFunction(
		const FString& FunctionName,
		const FString& ShaderCode,
		const TArray<FMaterialFunctionInput>& Inputs,
		const TArray<FMaterialFunctionOutput>& Outputs
	);

	/**
	 * Validate material function inputs and outputs
	 * @param MaterialFunction - Function to validate
	 * @return True if valid
	 */
	UFUNCTION(BlueprintPure, Category = "Material Function")
	static bool ValidateMaterialFunction(UMaterialFunction* MaterialFunction);

protected:
	/**
	 * Setup triplanar projection function internals
	 */
	void SetupTriplanarProjection(UMaterialFunction* Function);

	/**
	 * Setup height blending function internals
	 */
	void SetupHeightBlending(UMaterialFunction* Function);

	/**
	 * Setup slope mapping function internals
	 */
	void SetupSlopeMapping(UMaterialFunction* Function);

	/**
	 * Setup distance fading function internals
	 */
	void SetupDistanceFading(UMaterialFunction* Function);

	/**
	 * Setup normal blending function internals
	 */
	void SetupNormalBlending(UMaterialFunction* Function);

	/**
	 * Setup world-aligned texture function internals
	 */
	void SetupWorldAlignedTexture(UMaterialFunction* Function);

	/**
	 * Add custom shader code node to material function
	 */
	bool AddCustomShaderNode(
		UMaterialFunction* Function,
		const FString& ShaderCode,
		const TArray<FMaterialFunctionInput>& Inputs,
		const TArray<FMaterialFunctionOutput>& Outputs
	);

private:
	/**
	 * Create base material function object
	 */
	UMaterialFunction* CreateBaseMaterialFunction(const FString& FunctionName, const FString& Description);

	/**
	 * Get shader include path
	 */
	FString GetShaderIncludePath(EMaterialFunctionTemplate Template) const;
};
