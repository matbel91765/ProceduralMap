// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "TerrainGenerationTask.h"

class FUICommandList;
class STerrainPreviewWidget;
class UQuadTreeTerrain;

/**
 * Editor mode tool types for procedural terrain editing
 */
enum class EProceduralTerrainTool : uint8
{
	None,
	Paint,
	Sculpt,
	Smooth,
	Flatten,
	Generate
};

/**
 * FProceduralMapEditorMode
 * Custom editor mode for real-time procedural terrain editing
 * Provides interactive tools: Paint, Sculpt, Smooth, Flatten, Generate
 */
class PROCEDURALMAPEDITOR_API FProceduralMapEditorMode : public FEdMode
{
public:
	/** Mode ID constant */
	const static FEditorModeID EM_ProceduralMapEditorModeId;

	FProceduralMapEditorMode();
	virtual ~FProceduralMapEditorMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual bool UsesToolkits() const override { return true; }
	virtual void CreateToolkit() override;
	virtual TSharedPtr<class FModeToolkit> GetToolkit() override;
	// End of FEdMode interface

	/** Get current tool */
	EProceduralTerrainTool GetCurrentTool() const { return CurrentTool; }

	/** Set current tool */
	void SetCurrentTool(EProceduralTerrainTool Tool);

	/** Get brush size */
	float GetBrushSize() const { return BrushSize; }

	/** Set brush size */
	void SetBrushSize(float Size);

	/** Get brush strength */
	float GetBrushStrength() const { return BrushStrength; }

	/** Set brush strength */
	void SetBrushStrength(float Strength);

	/** Get brush falloff */
	float GetBrushFalloff() const { return BrushFalloff; }

	/** Set brush falloff */
	void SetBrushFalloff(float Falloff);

	/** Generate terrain with current parameters */
	void GenerateTerrain();

	/** Clear current terrain */
	void ClearTerrain();

	/** Export heightmap */
	void ExportHeightmap();

	/** Import heightmap */
	void ImportHeightmap();

	/** Get preview widget */
	TSharedPtr<STerrainPreviewWidget> GetPreviewWidget() const { return PreviewWidget; }

	/** Update preview with current parameters */
	void UpdatePreview();

	/** Get terrain generation parameters */
	FTerrainGenerationParams& GetGenerationParams() { return GenerationParams; }

	/** Set terrain generation parameters */
	void SetGenerationParams(const FTerrainGenerationParams& Params);

protected:
	/** Current active tool */
	EProceduralTerrainTool CurrentTool;

	/** Brush parameters */
	float BrushSize;
	float BrushStrength;
	float BrushFalloff;

	/** Terrain generation parameters */
	FTerrainGenerationParams GenerationParams;

	/** Preview widget for real-time visualization */
	TSharedPtr<STerrainPreviewWidget> PreviewWidget;

	/** Mode toolkit */
	TSharedPtr<class FModeToolkit> Toolkit;

	/** UI Commands */
	TSharedPtr<FUICommandList> UICommandList;

	/** Currently selected terrain actor */
	TWeakObjectPtr<UQuadTreeTerrain> SelectedTerrain;

	/** Brush location in world space */
	FVector BrushLocation;

	/** Is brush active (mouse down) */
	bool bBrushActive;

	/** Last mouse position */
	FVector2D LastMousePosition;

	/** Preview update timer */
	float PreviewUpdateTimer;

	/** Preview update interval (seconds) */
	float PreviewUpdateInterval;

	/** Cached heightmap data for preview */
	TArray<float> PreviewHeightData;

	/** Preview heightmap resolution */
	int32 PreviewResolution;

private:
	/** Apply paint tool at location */
	void ApplyPaintTool(const FVector& Location, float DeltaTime);

	/** Apply sculpt tool at location */
	void ApplySculptTool(const FVector& Location, float DeltaTime);

	/** Apply smooth tool at location */
	void ApplySmoothTool(const FVector& Location, float DeltaTime);

	/** Apply flatten tool at location */
	void ApplyFlattenTool(const FVector& Location, float DeltaTime);

	/** Calculate brush weight at distance from center */
	float GetBrushWeight(float Distance) const;

	/** Trace to find terrain hit under cursor */
	bool TraceForTerrainHit(FEditorViewportClient* ViewportClient, const FViewportClick& Click, FVector& OutHitLocation);

	/** Register UI commands */
	void RegisterUICommands();

	/** Bind UI commands */
	void BindCommands();

	/** Generate preview heightmap data */
	void GeneratePreviewData();

	/** Find or create terrain actor in level */
	UQuadTreeTerrain* FindOrCreateTerrainActor();

	/** Update selected terrain */
	void UpdateSelectedTerrain();

	/** Draw brush visualization */
	void DrawBrush(FPrimitiveDrawInterface* PDI);

	/** Is preview dirty and needs update */
	bool bPreviewDirty;

	/** Performance stats */
	int32 NumVerticesModified;
	float LastToolApplicationTime;
};
