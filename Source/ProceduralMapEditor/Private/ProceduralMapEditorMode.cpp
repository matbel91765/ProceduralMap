// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "ProceduralMapEditorMode.h"
#include "ProceduralMapEditorModule.h"
#include "TerrainPreviewWidget.h"
#include "QuadTreeTerrain.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Commands/UICommandList.h"
#include "Toolkits/ToolkitManager.h"
#include "Toolkits/BaseToolkit.h"
#include "Engine/Selection.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Text/STextBlock.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

#define LOCTEXT_NAMESPACE "ProceduralMapEditorMode"

const FEditorModeID FProceduralMapEditorMode::EM_ProceduralMapEditorModeId = TEXT("EM_ProceduralMapEditorMode");

FProceduralMapEditorMode::FProceduralMapEditorMode()
	: FEdMode()
	, CurrentTool(EProceduralTerrainTool::None)
	, BrushSize(500.0f)
	, BrushStrength(1.0f)
	, BrushFalloff(0.5f)
	, bBrushActive(false)
	, LastMousePosition(FVector2D::ZeroVector)
	, PreviewUpdateTimer(0.0f)
	, PreviewUpdateInterval(0.1f)
	, PreviewResolution(256)
	, bPreviewDirty(true)
	, NumVerticesModified(0)
	, LastToolApplicationTime(0.0f)
{
	// Initialize generation parameters
	GenerationParams = FTerrainGenerationParams();
}

FProceduralMapEditorMode::~FProceduralMapEditorMode()
{
}

void FProceduralMapEditorMode::Enter()
{
	FEdMode::Enter();

	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditorMode: Entering mode"));

	// Register UI commands
	RegisterUICommands();

	// Find or create terrain actor
	UpdateSelectedTerrain();

	// Mark preview as dirty
	bPreviewDirty = true;

	// Generate initial preview data
	GeneratePreviewData();
}

void FProceduralMapEditorMode::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditorMode: Exiting mode"));

	// Clear references
	SelectedTerrain.Reset();
	PreviewWidget.Reset();
	UICommandList.Reset();

	FEdMode::Exit();
}

void FProceduralMapEditorMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);

	// Update preview timer
	PreviewUpdateTimer += DeltaTime;

	// Update preview if dirty and enough time has passed
	if (bPreviewDirty && PreviewUpdateTimer >= PreviewUpdateInterval)
	{
		UpdatePreview();
		PreviewUpdateTimer = 0.0f;
		bPreviewDirty = false;
	}

	// Apply tool if brush is active
	if (bBrushActive && CurrentTool != EProceduralTerrainTool::None)
	{
		switch (CurrentTool)
		{
		case EProceduralTerrainTool::Paint:
			ApplyPaintTool(BrushLocation, DeltaTime);
			break;
		case EProceduralTerrainTool::Sculpt:
			ApplySculptTool(BrushLocation, DeltaTime);
			break;
		case EProceduralTerrainTool::Smooth:
			ApplySmoothTool(BrushLocation, DeltaTime);
			break;
		case EProceduralTerrainTool::Flatten:
			ApplyFlattenTool(BrushLocation, DeltaTime);
			break;
		default:
			break;
		}
	}
}

void FProceduralMapEditorMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);

	// Draw brush visualization
	if (CurrentTool != EProceduralTerrainTool::None)
	{
		DrawBrush(PDI);
	}
}

void FProceduralMapEditorMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);

	// Draw tool information
	if (CurrentTool != EProceduralTerrainTool::None)
	{
		FString ToolName;
		switch (CurrentTool)
		{
		case EProceduralTerrainTool::Paint: ToolName = TEXT("Paint"); break;
		case EProceduralTerrainTool::Sculpt: ToolName = TEXT("Sculpt"); break;
		case EProceduralTerrainTool::Smooth: ToolName = TEXT("Smooth"); break;
		case EProceduralTerrainTool::Flatten: ToolName = TEXT("Flatten"); break;
		case EProceduralTerrainTool::Generate: ToolName = TEXT("Generate"); break;
		default: ToolName = TEXT("None"); break;
		}

		FString InfoText = FString::Printf(TEXT("Tool: %s | Size: %.0f | Strength: %.2f | Falloff: %.2f"),
			*ToolName, BrushSize, BrushStrength, BrushFalloff);

		FCanvasTextItem TextItem(FVector2D(10, 50), FText::FromString(InfoText), GEngine->GetLargeFont(), FLinearColor::Green);
		TextItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TextItem);

		// Draw performance stats
		if (NumVerticesModified > 0)
		{
			FString StatsText = FString::Printf(TEXT("Vertices Modified: %d"), NumVerticesModified);
			FCanvasTextItem StatsTextItem(FVector2D(10, 80), FText::FromString(StatsText), GEngine->GetMediumFont(), FLinearColor::Yellow);
			StatsTextItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(StatsTextItem);
		}
	}
}

bool FProceduralMapEditorMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	// Handle left click
	if (Click.GetKey() == EKeys::LeftMouseButton)
	{
		FVector HitLocation;
		if (TraceForTerrainHit(InViewportClient, Click, HitLocation))
		{
			BrushLocation = HitLocation;
			bBrushActive = true;

			// Apply tool immediately for single-click tools
			if (CurrentTool == EProceduralTerrainTool::Generate)
			{
				GenerateTerrain();
				return true;
			}

			return true;
		}
	}

	return FEdMode::HandleClick(InViewportClient, HitProxy, Click);
}

bool FProceduralMapEditorMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	// Handle mouse release
	if (Key == EKeys::LeftMouseButton && Event == IE_Released)
	{
		bBrushActive = false;
		NumVerticesModified = 0;
		return true;
	}

	// Handle tool shortcuts
	if (Event == IE_Pressed)
	{
		if (Key == EKeys::One)
		{
			SetCurrentTool(EProceduralTerrainTool::Paint);
			return true;
		}
		else if (Key == EKeys::Two)
		{
			SetCurrentTool(EProceduralTerrainTool::Sculpt);
			return true;
		}
		else if (Key == EKeys::Three)
		{
			SetCurrentTool(EProceduralTerrainTool::Smooth);
			return true;
		}
		else if (Key == EKeys::Four)
		{
			SetCurrentTool(EProceduralTerrainTool::Flatten);
			return true;
		}
		else if (Key == EKeys::Five)
		{
			SetCurrentTool(EProceduralTerrainTool::Generate);
			return true;
		}
		else if (Key == EKeys::LeftBracket)
		{
			SetBrushSize(FMath::Max(BrushSize - 50.0f, 50.0f));
			return true;
		}
		else if (Key == EKeys::RightBracket)
		{
			SetBrushSize(FMath::Min(BrushSize + 50.0f, 5000.0f));
			return true;
		}
	}

	return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

bool FProceduralMapEditorMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	// Update brush location during drag
	if (bBrushActive)
	{
		// This would update brush location based on mouse movement
		// Implementation depends on viewport interaction
	}

	return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
}

void FProceduralMapEditorMode::CreateToolkit()
{
	// Toolkit creation handled by editor framework
	// Custom toolkit can be implemented here if needed
}

TSharedPtr<class FModeToolkit> FProceduralMapEditorMode::GetToolkit()
{
	return Toolkit;
}

void FProceduralMapEditorMode::SetCurrentTool(EProceduralTerrainTool Tool)
{
	if (CurrentTool != Tool)
	{
		CurrentTool = Tool;
		UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditorMode: Tool changed to %d"), (int32)Tool);
	}
}

void FProceduralMapEditorMode::SetBrushSize(float Size)
{
	BrushSize = FMath::Clamp(Size, 10.0f, 10000.0f);
}

void FProceduralMapEditorMode::SetBrushStrength(float Strength)
{
	BrushStrength = FMath::Clamp(Strength, 0.0f, 10.0f);
}

void FProceduralMapEditorMode::SetBrushFalloff(float Falloff)
{
	BrushFalloff = FMath::Clamp(Falloff, 0.0f, 1.0f);
}

void FProceduralMapEditorMode::GenerateTerrain()
{
	if (SelectedTerrain.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditorMode: Generating terrain"));

		// Update terrain with current generation parameters
		SelectedTerrain->GenerationParams = GenerationParams;
		SelectedTerrain->RegenerateTerrain();

		// Update preview
		bPreviewDirty = true;
	}
}

void FProceduralMapEditorMode::ClearTerrain()
{
	if (SelectedTerrain.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditorMode: Clearing terrain"));
		SelectedTerrain->CleanupTerrain();
		bPreviewDirty = true;
	}
}

void FProceduralMapEditorMode::ExportHeightmap()
{
	// Implementation for heightmap export
	UE_LOG(LogTemp, Warning, TEXT("ProceduralMapEditorMode: Export heightmap not yet implemented"));
}

void FProceduralMapEditorMode::ImportHeightmap()
{
	// Implementation for heightmap import
	UE_LOG(LogTemp, Warning, TEXT("ProceduralMapEditorMode: Import heightmap not yet implemented"));
}

void FProceduralMapEditorMode::UpdatePreview()
{
	if (PreviewWidget.IsValid())
	{
		PreviewWidget->UpdatePreview(GenerationParams, PreviewHeightData);
	}
}

void FProceduralMapEditorMode::SetGenerationParams(const FTerrainGenerationParams& Params)
{
	GenerationParams = Params;
	bPreviewDirty = true;
	GeneratePreviewData();
}

void FProceduralMapEditorMode::ApplyPaintTool(const FVector& Location, float DeltaTime)
{
	// Paint tool implementation - raises terrain
	if (!SelectedTerrain.IsValid()) return;

	// This is a placeholder - actual implementation would modify terrain mesh
	NumVerticesModified++;
}

void FProceduralMapEditorMode::ApplySculptTool(const FVector& Location, float DeltaTime)
{
	// Sculpt tool implementation - raises/lowers based on mouse movement
	if (!SelectedTerrain.IsValid()) return;

	// This is a placeholder - actual implementation would modify terrain mesh
	NumVerticesModified++;
}

void FProceduralMapEditorMode::ApplySmoothTool(const FVector& Location, float DeltaTime)
{
	// Smooth tool implementation - averages neighboring heights
	if (!SelectedTerrain.IsValid()) return;

	// This is a placeholder - actual implementation would modify terrain mesh
	NumVerticesModified++;
}

void FProceduralMapEditorMode::ApplyFlattenTool(const FVector& Location, float DeltaTime)
{
	// Flatten tool implementation - flattens to target height
	if (!SelectedTerrain.IsValid()) return;

	// This is a placeholder - actual implementation would modify terrain mesh
	NumVerticesModified++;
}

float FProceduralMapEditorMode::GetBrushWeight(float Distance) const
{
	if (Distance >= BrushSize)
		return 0.0f;

	float NormalizedDistance = Distance / BrushSize;
	return FMath::Pow(1.0f - NormalizedDistance, BrushFalloff * 4.0f);
}

bool FProceduralMapEditorMode::TraceForTerrainHit(FEditorViewportClient* ViewportClient, const FViewportClick& Click, FVector& OutHitLocation)
{
	// Get ray from viewport click
	FVector RayOrigin, RayDirection;
	if (ViewportClient && ViewportClient->Viewport)
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			ViewportClient->GetScene(),
			ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(ViewportClient->IsRealtime()));

		FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);

		FViewportCursorLocation Cursor(View, ViewportClient, Click.GetClickPos().X, Click.GetClickPos().Y);
		RayOrigin = Cursor.GetOrigin();
		RayDirection = Cursor.GetDirection();

		// Trace against world
		FHitResult HitResult;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TerrainTrace), true);

		if (ViewportClient->GetWorld()->LineTraceSingleByChannel(
			HitResult,
			RayOrigin,
			RayOrigin + RayDirection * 100000.0f,
			ECC_Visibility,
			QueryParams))
		{
			OutHitLocation = HitResult.Location;
			return true;
		}
	}

	return false;
}

void FProceduralMapEditorMode::RegisterUICommands()
{
	UICommandList = MakeShareable(new FUICommandList);
	// Commands would be bound here
}

void FProceduralMapEditorMode::BindCommands()
{
	// Command bindings would go here
}

void FProceduralMapEditorMode::GeneratePreviewData()
{
	// Generate preview heightmap data
	PreviewHeightData.SetNum(PreviewResolution * PreviewResolution);

	// This would use FastNoiseLite to generate preview data
	// Placeholder for now
	for (int32 i = 0; i < PreviewHeightData.Num(); i++)
	{
		PreviewHeightData[i] = FMath::FRandRange(0.0f, 1.0f);
	}

	bPreviewDirty = true;
}

UQuadTreeTerrain* FProceduralMapEditorMode::FindOrCreateTerrainActor()
{
	// Find existing terrain actor in level
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		for (TActorIterator<AActor> It(GEditor->GetEditorWorldContext().World()); It; ++It)
		{
			UQuadTreeTerrain* TerrainComponent = It->FindComponentByClass<UQuadTreeTerrain>();
			if (TerrainComponent)
			{
				return TerrainComponent;
			}
		}
	}

	return nullptr;
}

void FProceduralMapEditorMode::UpdateSelectedTerrain()
{
	SelectedTerrain = FindOrCreateTerrainActor();

	if (SelectedTerrain.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("ProceduralMapEditorMode: Selected terrain found"));
		GenerationParams = SelectedTerrain->GenerationParams;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ProceduralMapEditorMode: No terrain found in level"));
	}
}

void FProceduralMapEditorMode::DrawBrush(FPrimitiveDrawInterface* PDI)
{
	if (bBrushActive || CurrentTool != EProceduralTerrainTool::None)
	{
		// Draw brush circle at brush location
		FColor BrushColor = FColor::Green;

		switch (CurrentTool)
		{
		case EProceduralTerrainTool::Paint: BrushColor = FColor::Green; break;
		case EProceduralTerrainTool::Sculpt: BrushColor = FColor::Blue; break;
		case EProceduralTerrainTool::Smooth: BrushColor = FColor::Yellow; break;
		case EProceduralTerrainTool::Flatten: BrushColor = FColor::Red; break;
		default: BrushColor = FColor::White; break;
		}

		// Draw circle
		const int32 NumSegments = 64;
		const float AngleStep = 2.0f * PI / NumSegments;

		for (int32 i = 0; i < NumSegments; i++)
		{
			float Angle1 = i * AngleStep;
			float Angle2 = (i + 1) * AngleStep;

			FVector Point1 = BrushLocation + FVector(FMath::Cos(Angle1) * BrushSize, FMath::Sin(Angle1) * BrushSize, 0.0f);
			FVector Point2 = BrushLocation + FVector(FMath::Cos(Angle2) * BrushSize, FMath::Sin(Angle2) * BrushSize, 0.0f);

			PDI->DrawLine(Point1, Point2, BrushColor, SDPG_Foreground, 2.0f);
		}

		// Draw center point
		PDI->DrawPoint(BrushLocation, BrushColor, 10.0f, SDPG_Foreground);
	}
}

#undef LOCTEXT_NAMESPACE
