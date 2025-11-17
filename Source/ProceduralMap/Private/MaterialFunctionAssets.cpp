// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaterialFunctionAssets.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

UMaterialFunctionAssetHelper::UMaterialFunctionAssetHelper()
{
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateMaterialFunction(const FMaterialFunctionConfig& Config)
{
	UMaterialFunction* Function = CreateBaseMaterialFunction(Config.FunctionName, Config.Description);

	if (!Function)
	{
		return nullptr;
	}

	// Setup function based on template
	switch (Config.Template)
	{
	case EMaterialFunctionTemplate::TriplanarProjection:
		SetupTriplanarProjection(Function);
		break;

	case EMaterialFunctionTemplate::HeightBlending:
		SetupHeightBlending(Function);
		break;

	case EMaterialFunctionTemplate::SlopeMapping:
		SetupSlopeMapping(Function);
		break;

	case EMaterialFunctionTemplate::DistanceFading:
		SetupDistanceFading(Function);
		break;

	case EMaterialFunctionTemplate::NormalBlending:
		SetupNormalBlending(Function);
		break;

	case EMaterialFunctionTemplate::WorldAlignedTexture:
		SetupWorldAlignedTexture(Function);
		break;

	default:
		break;
	}

	// Load and apply shader code if specified
	if (!Config.ShaderCodePath.IsEmpty())
	{
		FString ShaderCode = LoadShaderCode(Config.ShaderCodePath);
		if (!ShaderCode.IsEmpty())
		{
			AddCustomShaderNode(Function, ShaderCode, Config.Inputs, Config.Outputs);
		}
	}

	return Function;
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateTriplanarProjectionFunction(const FString& FunctionName)
{
	FMaterialFunctionConfig Config = GetDefaultConfig(EMaterialFunctionTemplate::TriplanarProjection);
	Config.FunctionName = FunctionName;
	return CreateMaterialFunction(Config);
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateHeightBlendingFunction(const FString& FunctionName)
{
	FMaterialFunctionConfig Config = GetDefaultConfig(EMaterialFunctionTemplate::HeightBlending);
	Config.FunctionName = FunctionName;
	return CreateMaterialFunction(Config);
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateSlopeMappingFunction(const FString& FunctionName)
{
	FMaterialFunctionConfig Config = GetDefaultConfig(EMaterialFunctionTemplate::SlopeMapping);
	Config.FunctionName = FunctionName;
	return CreateMaterialFunction(Config);
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateDistanceFadingFunction(const FString& FunctionName)
{
	FMaterialFunctionConfig Config = GetDefaultConfig(EMaterialFunctionTemplate::DistanceFading);
	Config.FunctionName = FunctionName;
	return CreateMaterialFunction(Config);
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateNormalBlendingFunction(const FString& FunctionName)
{
	FMaterialFunctionConfig Config = GetDefaultConfig(EMaterialFunctionTemplate::NormalBlending);
	Config.FunctionName = FunctionName;
	return CreateMaterialFunction(Config);
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateWorldAlignedTextureFunction(const FString& FunctionName)
{
	FMaterialFunctionConfig Config = GetDefaultConfig(EMaterialFunctionTemplate::WorldAlignedTexture);
	Config.FunctionName = FunctionName;
	return CreateMaterialFunction(Config);
}

bool UMaterialFunctionAssetHelper::ExportMaterialFunction(
	UMaterialFunction* MaterialFunction,
	const FString& ExportPath,
	const FString& FileName)
{
	if (!MaterialFunction)
	{
		UE_LOG(LogTemp, Error, TEXT("UMaterialFunctionAssetHelper::ExportMaterialFunction - MaterialFunction is null"));
		return false;
	}

	// Create package for the material function
	FString PackageName = ExportPath + FileName;
	UPackage* Package = CreatePackage(*PackageName);

	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("UMaterialFunctionAssetHelper::ExportMaterialFunction - Failed to create package"));
		return false;
	}

	// Rename the function to the package
	MaterialFunction->Rename(*FileName, Package, REN_DontCreateRedirectors);

	// Mark package as dirty
	Package->MarkPackageDirty();

	// Notify asset registry
	FAssetRegistryModule::AssetCreated(MaterialFunction);

	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::ExportMaterialFunction - Exported to %s"), *PackageName);

	return true;
}

FMaterialFunctionConfig UMaterialFunctionAssetHelper::GetDefaultConfig(EMaterialFunctionTemplate Template)
{
	FMaterialFunctionConfig Config;
	Config.Template = Template;

	switch (Template)
	{
	case EMaterialFunctionTemplate::TriplanarProjection:
		Config.FunctionName = TEXT("TriplanarProjection");
		Config.Description = TEXT("Triplanar projection mapping for textures");
		Config.ShaderCodePath = TEXT("/Shaders/Private/TriplanarProjection.ush");
		break;

	case EMaterialFunctionTemplate::HeightBlending:
		Config.FunctionName = TEXT("HeightBlending");
		Config.Description = TEXT("Height-based texture blending");
		Config.ShaderCodePath = TEXT("/Shaders/Private/HeightBlending.ush");
		break;

	case EMaterialFunctionTemplate::SlopeMapping:
		Config.FunctionName = TEXT("SlopeMapping");
		Config.Description = TEXT("Slope-based texture mapping");
		Config.ShaderCodePath = TEXT("/Shaders/Private/SlopeMapping.ush");
		break;

	case EMaterialFunctionTemplate::DistanceFading:
		Config.FunctionName = TEXT("DistanceFading");
		Config.Description = TEXT("Distance-based detail fading");
		break;

	case EMaterialFunctionTemplate::NormalBlending:
		Config.FunctionName = TEXT("NormalBlending");
		Config.Description = TEXT("Triplanar normal map blending");
		break;

	case EMaterialFunctionTemplate::WorldAlignedTexture:
		Config.FunctionName = TEXT("WorldAlignedTexture");
		Config.Description = TEXT("World-aligned texture projection");
		break;

	default:
		break;
	}

	return Config;
}

FString UMaterialFunctionAssetHelper::LoadShaderCode(const FString& ShaderFilePath)
{
	FString ShaderCode;
	FString FullPath = FPaths::ProjectDir() + ShaderFilePath;

	if (FFileHelper::LoadFileToString(ShaderCode, *FullPath))
	{
		UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::LoadShaderCode - Loaded shader from %s"), *FullPath);
		return ShaderCode;
	}

	UE_LOG(LogTemp, Warning, TEXT("UMaterialFunctionAssetHelper::LoadShaderCode - Failed to load shader from %s"), *FullPath);
	return FString();
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateCustomShaderFunction(
	const FString& FunctionName,
	const FString& ShaderCode,
	const TArray<FMaterialFunctionInput>& Inputs,
	const TArray<FMaterialFunctionOutput>& Outputs)
{
	UMaterialFunction* Function = CreateBaseMaterialFunction(FunctionName, TEXT("Custom shader function"));

	if (!Function)
	{
		return nullptr;
	}

	if (!AddCustomShaderNode(Function, ShaderCode, Inputs, Outputs))
	{
		return nullptr;
	}

	return Function;
}

bool UMaterialFunctionAssetHelper::ValidateMaterialFunction(UMaterialFunction* MaterialFunction)
{
	if (!MaterialFunction)
	{
		return false;
	}

	// Check if function has at least one output
	bool bHasOutput = false;
	for (UMaterialExpression* Expression : MaterialFunction->GetExpressions())
	{
		if (Expression->IsA<UMaterialExpressionFunctionOutput>())
		{
			bHasOutput = true;
			break;
		}
	}

	if (!bHasOutput)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMaterialFunctionAssetHelper::ValidateMaterialFunction - Function has no outputs"));
		return false;
	}

	return true;
}

void UMaterialFunctionAssetHelper::SetupTriplanarProjection(UMaterialFunction* Function)
{
	if (!Function)
	{
		return;
	}

	// This is a simplified setup
	// In a real implementation, you would create the full node graph
	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::SetupTriplanarProjection - Setting up triplanar projection function"));

	// Create function inputs
	UMaterialExpressionFunctionInput* TextureInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	TextureInput->InputName = TEXT("Texture");
	TextureInput->Description = TEXT("Texture to project");
	Function->GetExpressions().Add(TextureInput);

	UMaterialExpressionFunctionInput* BlendSharpnessInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	BlendSharpnessInput->InputName = TEXT("BlendSharpness");
	BlendSharpnessInput->Description = TEXT("Blend sharpness between projections");
	Function->GetExpressions().Add(BlendSharpnessInput);

	// Create function output
	UMaterialExpressionFunctionOutput* Output = NewObject<UMaterialExpressionFunctionOutput>(Function);
	Output->OutputName = TEXT("Result");
	Function->GetExpressions().Add(Output);

	// Mark as changed
	Function->UpdateParameterSet();
}

void UMaterialFunctionAssetHelper::SetupHeightBlending(UMaterialFunction* Function)
{
	if (!Function)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::SetupHeightBlending - Setting up height blending function"));

	// Create function inputs for multiple layers
	UMaterialExpressionFunctionInput* Layer1Input = NewObject<UMaterialExpressionFunctionInput>(Function);
	Layer1Input->InputName = TEXT("Layer1");
	Layer1Input->Description = TEXT("First layer color");
	Function->GetExpressions().Add(Layer1Input);

	UMaterialExpressionFunctionInput* Height1Input = NewObject<UMaterialExpressionFunctionInput>(Function);
	Height1Input->InputName = TEXT("Height1");
	Height1Input->Description = TEXT("First layer height");
	Function->GetExpressions().Add(Height1Input);

	UMaterialExpressionFunctionInput* Layer2Input = NewObject<UMaterialExpressionFunctionInput>(Function);
	Layer2Input->InputName = TEXT("Layer2");
	Layer2Input->Description = TEXT("Second layer color");
	Function->GetExpressions().Add(Layer2Input);

	UMaterialExpressionFunctionInput* Height2Input = NewObject<UMaterialExpressionFunctionInput>(Function);
	Height2Input->InputName = TEXT("Height2");
	Height2Input->Description = TEXT("Second layer height");
	Function->GetExpressions().Add(Height2Input);

	UMaterialExpressionFunctionInput* BlendStrengthInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	BlendStrengthInput->InputName = TEXT("BlendStrength");
	BlendStrengthInput->Description = TEXT("Strength of height-based blending");
	Function->GetExpressions().Add(BlendStrengthInput);

	// Create function output
	UMaterialExpressionFunctionOutput* Output = NewObject<UMaterialExpressionFunctionOutput>(Function);
	Output->OutputName = TEXT("BlendedColor");
	Function->GetExpressions().Add(Output);

	Function->UpdateParameterSet();
}

void UMaterialFunctionAssetHelper::SetupSlopeMapping(UMaterialFunction* Function)
{
	if (!Function)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::SetupSlopeMapping - Setting up slope mapping function"));

	// Create function inputs
	UMaterialExpressionFunctionInput* MinSlopeInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	MinSlopeInput->InputName = TEXT("MinSlope");
	MinSlopeInput->Description = TEXT("Minimum slope angle");
	Function->GetExpressions().Add(MinSlopeInput);

	UMaterialExpressionFunctionInput* MaxSlopeInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	MaxSlopeInput->InputName = TEXT("MaxSlope");
	MaxSlopeInput->Description = TEXT("Maximum slope angle");
	Function->GetExpressions().Add(MaxSlopeInput);

	// Create function output
	UMaterialExpressionFunctionOutput* Output = NewObject<UMaterialExpressionFunctionOutput>(Function);
	Output->OutputName = TEXT("SlopeMask");
	Function->GetExpressions().Add(Output);

	Function->UpdateParameterSet();
}

void UMaterialFunctionAssetHelper::SetupDistanceFading(UMaterialFunction* Function)
{
	if (!Function)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::SetupDistanceFading - Setting up distance fading function"));

	// Create function inputs
	UMaterialExpressionFunctionInput* NearDistanceInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	NearDistanceInput->InputName = TEXT("NearDistance");
	NearDistanceInput->Description = TEXT("Distance where near detail is at full strength");
	Function->GetExpressions().Add(NearDistanceInput);

	UMaterialExpressionFunctionInput* FarDistanceInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	FarDistanceInput->InputName = TEXT("FarDistance");
	FarDistanceInput->Description = TEXT("Distance where far detail is at full strength");
	Function->GetExpressions().Add(FarDistanceInput);

	// Create function output
	UMaterialExpressionFunctionOutput* Output = NewObject<UMaterialExpressionFunctionOutput>(Function);
	Output->OutputName = TEXT("FadeAlpha");
	Function->GetExpressions().Add(Output);

	Function->UpdateParameterSet();
}

void UMaterialFunctionAssetHelper::SetupNormalBlending(UMaterialFunction* Function)
{
	if (!Function)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::SetupNormalBlending - Setting up normal blending function"));

	// Create function inputs for triplanar normals
	UMaterialExpressionFunctionInput* NormalXInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	NormalXInput->InputName = TEXT("NormalX");
	NormalXInput->Description = TEXT("Normal sample from X projection");
	Function->GetExpressions().Add(NormalXInput);

	UMaterialExpressionFunctionInput* NormalYInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	NormalYInput->InputName = TEXT("NormalY");
	NormalYInput->Description = TEXT("Normal sample from Y projection");
	Function->GetExpressions().Add(NormalYInput);

	UMaterialExpressionFunctionInput* NormalZInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	NormalZInput->InputName = TEXT("NormalZ");
	NormalZInput->Description = TEXT("Normal sample from Z projection");
	Function->GetExpressions().Add(NormalZInput);

	UMaterialExpressionFunctionInput* BlendWeightsInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	BlendWeightsInput->InputName = TEXT("BlendWeights");
	BlendWeightsInput->Description = TEXT("Blend weights (XYZ)");
	Function->GetExpressions().Add(BlendWeightsInput);

	// Create function output
	UMaterialExpressionFunctionOutput* Output = NewObject<UMaterialExpressionFunctionOutput>(Function);
	Output->OutputName = TEXT("BlendedNormal");
	Function->GetExpressions().Add(Output);

	Function->UpdateParameterSet();
}

void UMaterialFunctionAssetHelper::SetupWorldAlignedTexture(UMaterialFunction* Function)
{
	if (!Function)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::SetupWorldAlignedTexture - Setting up world-aligned texture function"));

	// Create function inputs
	UMaterialExpressionFunctionInput* TextureInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	TextureInput->InputName = TEXT("Texture");
	TextureInput->Description = TEXT("Texture to project");
	Function->GetExpressions().Add(TextureInput);

	UMaterialExpressionFunctionInput* TilingInput = NewObject<UMaterialExpressionFunctionInput>(Function);
	TilingInput->InputName = TEXT("Tiling");
	TilingInput->Description = TEXT("Texture tiling scale");
	Function->GetExpressions().Add(TilingInput);

	// Create function output
	UMaterialExpressionFunctionOutput* Output = NewObject<UMaterialExpressionFunctionOutput>(Function);
	Output->OutputName = TEXT("Color");
	Function->GetExpressions().Add(Output);

	Function->UpdateParameterSet();
}

bool UMaterialFunctionAssetHelper::AddCustomShaderNode(
	UMaterialFunction* Function,
	const FString& ShaderCode,
	const TArray<FMaterialFunctionInput>& Inputs,
	const TArray<FMaterialFunctionOutput>& Outputs)
{
	if (!Function || ShaderCode.IsEmpty())
	{
		return false;
	}

	// Create custom expression node
	UMaterialExpressionCustom* CustomNode = NewObject<UMaterialExpressionCustom>(Function);
	CustomNode->Code = ShaderCode;

	// Add inputs
	for (const FMaterialFunctionInput& Input : Inputs)
	{
		FCustomInput CustomInput;
		CustomInput.InputName = FName(*Input.InputName);
		CustomNode->Inputs.Add(CustomInput);
	}

	// Set output type based on first output
	if (Outputs.Num() > 0)
	{
		if (Outputs[0].OutputType == TEXT("Float"))
		{
			CustomNode->OutputType = CMOT_Float1;
		}
		else if (Outputs[0].OutputType == TEXT("Float2"))
		{
			CustomNode->OutputType = CMOT_Float2;
		}
		else if (Outputs[0].OutputType == TEXT("Float3"))
		{
			CustomNode->OutputType = CMOT_Float3;
		}
		else if (Outputs[0].OutputType == TEXT("Float4"))
		{
			CustomNode->OutputType = CMOT_Float4;
		}
	}

	Function->GetExpressions().Add(CustomNode);

	// Create function output
	UMaterialExpressionFunctionOutput* OutputNode = NewObject<UMaterialExpressionFunctionOutput>(Function);
	if (Outputs.Num() > 0)
	{
		OutputNode->OutputName = FName(*Outputs[0].OutputName);
	}
	Function->GetExpressions().Add(OutputNode);

	Function->UpdateParameterSet();

	return true;
}

UMaterialFunction* UMaterialFunctionAssetHelper::CreateBaseMaterialFunction(
	const FString& FunctionName,
	const FString& Description)
{
	// Create transient material function
	UMaterialFunction* Function = NewObject<UMaterialFunction>(
		GetTransientPackage(),
		*FunctionName,
		RF_Transient | RF_Public
	);

	if (!Function)
	{
		UE_LOG(LogTemp, Error, TEXT("UMaterialFunctionAssetHelper::CreateBaseMaterialFunction - Failed to create material function"));
		return nullptr;
	}

	Function->Description = Description;

	UE_LOG(LogTemp, Log, TEXT("UMaterialFunctionAssetHelper::CreateBaseMaterialFunction - Created function '%s'"), *FunctionName);

	return Function;
}

FString UMaterialFunctionAssetHelper::GetShaderIncludePath(EMaterialFunctionTemplate Template) const
{
	switch (Template)
	{
	case EMaterialFunctionTemplate::TriplanarProjection:
		return TEXT("/Shaders/Private/TriplanarProjection.ush");

	case EMaterialFunctionTemplate::HeightBlending:
		return TEXT("/Shaders/Private/HeightBlending.ush");

	case EMaterialFunctionTemplate::SlopeMapping:
		return TEXT("/Shaders/Private/SlopeMapping.ush");

	default:
		return TEXT("");
	}
}
