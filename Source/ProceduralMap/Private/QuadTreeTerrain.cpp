// Ultimate Procedural Map Generation System
// Copyright (C) 2025. All Rights Reserved.

#include "QuadTreeTerrain.h"
#include "ThirdParty/FastNoiseLite/FastNoiseLite.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

UQuadTreeTerrain::UQuadTreeTerrain()
	: TerrainSize(10000.0f)
	, MaxLODLevel(6)
	, LODDistanceMultiplier(2.0f)
	, MeshResolution(32)
	, TerrainMaterial(nullptr)
	, bEnableCollision(true)
	, CollisionComplexity(ECollisionTraceFlag::CTF_UseDefault)
	, BiomeName(NAME_None)
	, UpdateFrequency(1)
	, bEnableAsyncGeneration(false)
	, MaxMeshGenerationsPerFrame(5)
	, bSmoothNormals(true)
	, bUseTangents(false)
	, bShowDebugQuadTree(false)
	, bShowLODColors(false)
	, bShowWireframe(false)
	, LODManager(nullptr)
	, FrameCounter(0)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UQuadTreeTerrain::BeginPlay()
{
	Super::BeginPlay();

	// Get or create LOD Manager
	LODManager = ULODManager::Get(GetWorld());

	// Initialize terrain
	InitializeTerrain();
}

void UQuadTreeTerrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check update frequency
	FrameCounter++;
	if (FrameCounter % UpdateFrequency != 0)
	{
		return;
	}

	// Begin frame statistics
	if (LODManager)
	{
		LODManager->BeginFrame();
	}

	// Get camera position
	FVector CameraPosition = GetCameraPosition();
	CachedCameraPosition = CameraPosition;

	// Update LOD
	UpdateLOD(CameraPosition);

	// Process pending mesh updates
	ProcessUpdateQueue(MaxMeshGenerationsPerFrame);

	// End frame statistics
	if (LODManager)
	{
		LODManager->EndFrame(DeltaTime);
	}

	// Debug visualization
	if (bShowDebugQuadTree && RootNode.IsValid())
	{
		DrawDebugNode(RootNode);
	}
}

void UQuadTreeTerrain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupTerrain();
	Super::EndPlay(EndPlayReason);
}

void UQuadTreeTerrain::InitializeTerrain()
{
	// Cleanup existing terrain
	CleanupTerrain();

	// Initialize noise generator
	InitializeNoiseGenerator();

	// Create root node
	FVector2D Center(0.0f, 0.0f);
	RootNode = MakeShared<FQuadTreeNode>(Center, TerrainSize, 0);

	// Generate initial mesh for root
	GenerateMeshForNode(RootNode);

	UE_LOG(LogTemp, Log, TEXT("QuadTreeTerrain initialized: Size=%.0f, MaxLOD=%d"), TerrainSize, MaxLODLevel);
}

void UQuadTreeTerrain::UpdateLOD(const FVector& CameraPosition)
{
	if (!RootNode.IsValid())
	{
		return;
	}

	// Convert to 2D
	FVector2D CameraPosition2D(CameraPosition.X, CameraPosition.Y);

	// Get current time
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Recursively update LOD
	UpdateNodeLOD(RootNode, CameraPosition2D, CurrentTime);
}

void UQuadTreeTerrain::UpdateNodeLOD(TSharedPtr<FQuadTreeNode> Node, const FVector2D& CameraPosition2D, float CurrentTime)
{
	if (!Node.IsValid() || !LODManager)
	{
		return;
	}

	// Calculate distance to camera
	float Distance = CalculateDistanceToCamera(Node->Center, CameraPosition2D);
	Node->LastCameraDistance = Distance;

	// Check visibility (frustum culling)
	if (LODManager->bEnableFrustumCulling && !IsNodeVisible(Node))
	{
		// Node not visible, can potentially merge if has children
		if (Node->HasChildren())
		{
			if (CanMergeThisFrame() && MergeNode(Node))
			{
				if (LODManager)
				{
					LODManager->RecordMerge();
				}
			}
		}
		return;
	}

	// Check if we're at a leaf node
	if (Node->bIsLeaf)
	{
		// Check if we should split
		bool bShouldSplit = LODManager->ShouldSplitNode(
			Distance,
			Node->LODLevel,
			Node->Size,
			Node->LastUpdateTime,
			CurrentTime,
			BiomeName
		);

		if (bShouldSplit && Node->LODLevel < MaxLODLevel && CanSplitThisFrame())
		{
			if (SplitNode(Node))
			{
				Node->LastUpdateTime = CurrentTime;
				if (LODManager)
				{
					LODManager->RecordSplit();
				}

				// Recursively update children
				for (int32 i = 0; i < 4; ++i)
				{
					if (Node->Children[i].IsValid())
					{
						UpdateNodeLOD(Node->Children[i], CameraPosition2D, CurrentTime);
					}
				}
			}
		}
		else if (Node->bNeedsUpdate)
		{
			// Regenerate mesh if needed
			GenerateMeshForNode(Node);
			Node->bNeedsUpdate = false;
		}
	}
	else // Has children
	{
		// Check if all children should merge back
		bool bShouldMerge = true;
		for (int32 i = 0; i < 4; ++i)
		{
			if (Node->Children[i].IsValid())
			{
				float ChildDistance = CalculateDistanceToCamera(Node->Children[i]->Center, CameraPosition2D);

				if (!LODManager->ShouldMergeNode(
					ChildDistance,
					Node->Children[i]->LODLevel,
					Node->Children[i]->Size,
					Node->LastUpdateTime,
					CurrentTime,
					BiomeName))
				{
					bShouldMerge = false;
					break;
				}
			}
		}

		if (bShouldMerge && CanMergeThisFrame())
		{
			if (MergeNode(Node))
			{
				Node->LastUpdateTime = CurrentTime;
				if (LODManager)
				{
					LODManager->RecordMerge();
				}
			}
		}
		else
		{
			// Recursively update children
			for (int32 i = 0; i < 4; ++i)
			{
				if (Node->Children[i].IsValid())
				{
					UpdateNodeLOD(Node->Children[i], CameraPosition2D, CurrentTime);
				}
			}
		}
	}
}

bool UQuadTreeTerrain::SplitNode(TSharedPtr<FQuadTreeNode> Node)
{
	if (!Node.IsValid() || !Node->bIsLeaf)
	{
		return false;
	}

	// Destroy current mesh
	DestroyMeshComponent(Node);

	// Create 4 children
	float ChildSize = Node->Size * 0.5f;
	int32 ChildLODLevel = Node->LODLevel + 1;

	for (int32 i = 0; i < 4; ++i)
	{
		FVector2D ChildCenter = Node->GetChildCenter(i);
		Node->Children[i] = MakeShared<FQuadTreeNode>(ChildCenter, ChildSize, ChildLODLevel);
		Node->Children[i]->Parent = Node;

		// Generate mesh for child
		GenerateMeshForNode(Node->Children[i]);
	}

	Node->bIsLeaf = false;

	return true;
}

bool UQuadTreeTerrain::MergeNode(TSharedPtr<FQuadTreeNode> Node)
{
	if (!Node.IsValid() || Node->bIsLeaf)
	{
		return false;
	}

	// Destroy all children meshes
	for (int32 i = 0; i < 4; ++i)
	{
		if (Node->Children[i].IsValid())
		{
			DestroyMeshComponent(Node->Children[i]);
			Node->Children[i].Reset();
		}
	}

	// Mark as leaf
	Node->bIsLeaf = true;

	// Generate mesh for this node
	GenerateMeshForNode(Node);

	return true;
}

void UQuadTreeTerrain::GenerateMeshForNode(TSharedPtr<FQuadTreeNode> Node)
{
	if (!Node.IsValid() || !Node->bIsLeaf)
	{
		return;
	}

	// Get mesh resolution for this LOD level
	int32 Resolution = LODManager ?
		LODManager->GetMeshResolutionForLOD(Node->LODLevel, MeshResolution, BiomeName) :
		FMath::Max(4, MeshResolution >> Node->LODLevel);

	// Create or get mesh component
	if (!Node->MeshComponent)
	{
		Node->MeshComponent = CreateMeshComponent(Node);
	}

	if (!Node->MeshComponent)
	{
		return;
	}

	// Generate mesh data
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Calculate vertex positions
	float HalfSize = Node->Size * 0.5f;
	float StepSize = Node->Size / static_cast<float>(Resolution - 1);

	Vertices.Reserve(Resolution * Resolution);
	UVs.Reserve(Resolution * Resolution);
	Normals.Reserve(Resolution * Resolution);
	VertexColors.Reserve(Resolution * Resolution);

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			// Calculate world position
			float LocalX = -HalfSize + X * StepSize;
			float LocalY = -HalfSize + Y * StepSize;
			FVector2D WorldPos2D = Node->Center + FVector2D(LocalX, LocalY);

			// Get height from noise
			float Height = CalculateHeightAt(WorldPos2D);

			// Add vertex
			FVector Vertex(WorldPos2D.X, WorldPos2D.Y, Height);
			Vertices.Add(Vertex);

			// Calculate UV
			float U = static_cast<float>(X) / static_cast<float>(Resolution - 1);
			float V = static_cast<float>(Y) / static_cast<float>(Resolution - 1);
			UVs.Add(FVector2D(U, V));

			// Calculate normal
			FVector Normal = bSmoothNormals ?
				CalculateNormalAt(WorldPos2D, StepSize) :
				FVector::UpVector;
			Normals.Add(Normal);

			// Vertex color (for LOD debugging)
			FColor VertexColor = bShowLODColors && LODManager ?
				LODManager->GetDebugColorForLOD(Node->LODLevel) :
				FColor::White;
			VertexColors.Add(VertexColor);
		}
	}

	// Generate triangles
	Triangles.Reserve((Resolution - 1) * (Resolution - 1) * 6);

	for (int32 Y = 0; Y < Resolution - 1; ++Y)
	{
		for (int32 X = 0; X < Resolution - 1; ++X)
		{
			int32 Index0 = Y * Resolution + X;
			int32 Index1 = Y * Resolution + (X + 1);
			int32 Index2 = (Y + 1) * Resolution + X;
			int32 Index3 = (Y + 1) * Resolution + (X + 1);

			// Triangle 1
			Triangles.Add(Index0);
			Triangles.Add(Index2);
			Triangles.Add(Index1);

			// Triangle 2
			Triangles.Add(Index1);
			Triangles.Add(Index2);
			Triangles.Add(Index3);
		}
	}

	// Calculate tangents if needed
	if (bUseTangents)
	{
		Tangents.Reserve(Vertices.Num());
		for (int32 i = 0; i < Vertices.Num(); ++i)
		{
			FVector Tangent = FVector(1.0f, 0.0f, 0.0f);
			Tangents.Add(FProcMeshTangent(Tangent, false));
		}
	}

	// Create mesh section
	Node->MeshComponent->ClearAllMeshSections();
	Node->MeshComponent->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UVs,
		VertexColors,
		Tangents,
		bEnableCollision
	);

	// Set material
	if (TerrainMaterial)
	{
		Node->MeshComponent->SetMaterial(0, TerrainMaterial);
	}

	// Record mesh update
	if (LODManager)
	{
		LODManager->RecordMeshUpdate();
	}
}

float UQuadTreeTerrain::CalculateHeightAt(const FVector2D& Position) const
{
	if (!NoiseGenerator.IsValid())
	{
		return 0.0f;
	}

	// Get noise value
	float NoiseValue = NoiseGenerator->GetNoise(Position.X, Position.Y);

	// Apply height scale and offset
	float Height = (NoiseValue * GenerationParams.HeightScale) + GenerationParams.HeightOffset;

	return Height;
}

FVector UQuadTreeTerrain::CalculateNormalAt(const FVector2D& Position, float SampleDistance) const
{
	// Sample heights around the position (finite differences)
	float HeightL = CalculateHeightAt(Position + FVector2D(-SampleDistance, 0.0f));
	float HeightR = CalculateHeightAt(Position + FVector2D(SampleDistance, 0.0f));
	float HeightD = CalculateHeightAt(Position + FVector2D(0.0f, -SampleDistance));
	float HeightU = CalculateHeightAt(Position + FVector2D(0.0f, SampleDistance));

	// Calculate tangent vectors
	FVector TangentX(2.0f * SampleDistance, 0.0f, HeightR - HeightL);
	FVector TangentY(0.0f, 2.0f * SampleDistance, HeightU - HeightD);

	// Cross product to get normal
	FVector Normal = FVector::CrossProduct(TangentY, TangentX);
	Normal.Normalize();

	return Normal;
}

UProceduralMeshComponent* UQuadTreeTerrain::CreateMeshComponent(TSharedPtr<FQuadTreeNode> Node)
{
	if (!Node.IsValid())
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Create new procedural mesh component
	FString ComponentName = FString::Printf(TEXT("TerrainMesh_LOD%d_%d"), Node->LODLevel, FMath::Rand());
	UProceduralMeshComponent* MeshComp = NewObject<UProceduralMeshComponent>(Owner, FName(*ComponentName));

	if (MeshComp)
	{
		MeshComp->RegisterComponent();
		MeshComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		MeshComp->SetRelativeLocation(FVector::ZeroVector);
		MeshComp->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		MeshComp->SetCollisionObjectType(ECC_WorldStatic);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

		// Set collision complexity
		MeshComp->bUseComplexAsSimpleCollision = (CollisionComplexity == ECollisionTraceFlag::CTF_UseComplexAsSimple);
	}

	return MeshComp;
}

void UQuadTreeTerrain::DestroyMeshComponent(TSharedPtr<FQuadTreeNode> Node)
{
	if (Node.IsValid() && Node->MeshComponent)
	{
		Node->MeshComponent->ClearAllMeshSections();
		Node->MeshComponent->DestroyComponent();
		Node->MeshComponent = nullptr;
	}
}

bool UQuadTreeTerrain::IsNodeVisible(TSharedPtr<FQuadTreeNode> Node) const
{
	if (!Node.IsValid())
	{
		return false;
	}

	// Simple distance culling for now
	if (LODManager && LODManager->bEnableDistanceCulling)
	{
		float Distance = Node->LastCameraDistance;
		if (Distance > LODManager->MaxRenderDistance)
		{
			return false;
		}
	}

	// TODO: Implement proper frustum culling
	// This would require getting the camera's frustum planes and testing node bounds

	return true;
}

void UQuadTreeTerrain::DrawDebugNode(TSharedPtr<FQuadTreeNode> Node) const
{
	if (!Node.IsValid() || !GetWorld())
	{
		return;
	}

	// Get debug color based on LOD level
	FColor DebugColor = LODManager ?
		LODManager->GetDebugColorForLOD(Node->LODLevel) :
		FColor::White;

	// Draw bounds
	FVector Min(Node->Bounds.Min.X, Node->Bounds.Min.Y, 0.0f);
	FVector Max(Node->Bounds.Max.X, Node->Bounds.Max.Y, 1000.0f);
	FBox DebugBox(Min, Max);

	DrawDebugBox(GetWorld(), DebugBox.GetCenter(), DebugBox.GetExtent(), DebugColor, false, -1.0f, 0, 5.0f);

	// Recursively draw children
	if (Node->HasChildren())
	{
		for (int32 i = 0; i < 4; ++i)
		{
			if (Node->Children[i].IsValid())
			{
				DrawDebugNode(Node->Children[i]);
			}
		}
	}
}

void UQuadTreeTerrain::CollectStatistics(TSharedPtr<FQuadTreeNode> Node, FLODStatistics& OutStats) const
{
	if (!Node.IsValid())
	{
		return;
	}

	OutStats.TotalNodes++;

	if (Node->bIsLeaf)
	{
		OutStats.VisibleLeafNodes++;

		// Count nodes per LOD level
		while (OutStats.NodesPerLODLevel.Num() <= Node->LODLevel)
		{
			OutStats.NodesPerLODLevel.Add(0);
		}
		OutStats.NodesPerLODLevel[Node->LODLevel]++;
	}

	OutStats.TotalMemoryUsage += Node->GetMemoryUsage();

	// Recursively collect from children
	if (Node->HasChildren())
	{
		for (int32 i = 0; i < 4; ++i)
		{
			if (Node->Children[i].IsValid())
			{
				CollectStatistics(Node->Children[i], OutStats);
			}
		}
	}
}

FVector UQuadTreeTerrain::GetCameraPosition() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FVector::ZeroVector;
	}

	// Try to get player camera
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC && PC->PlayerCameraManager)
	{
		return PC->PlayerCameraManager->GetCameraLocation();
	}

	// Fallback to player pawn location
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (PlayerPawn)
	{
		return PlayerPawn->GetActorLocation();
	}

	return FVector::ZeroVector;
}

void UQuadTreeTerrain::InitializeNoiseGenerator()
{
	NoiseGenerator = MakeShared<FastNoiseLite>();

	NoiseGenerator->SetSeed(GenerationParams.Seed);
	NoiseGenerator->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	NoiseGenerator->SetFrequency(GenerationParams.Frequency);

	// Setup fractal for more realistic terrain
	NoiseGenerator->SetFractalType(FastNoiseLite::FractalType_FBm);
	NoiseGenerator->SetFractalOctaves(GenerationParams.Octaves);
	NoiseGenerator->SetFractalLacunarity(GenerationParams.Lacunarity);
	NoiseGenerator->SetFractalGain(GenerationParams.Gain);
}

void UQuadTreeTerrain::ProcessUpdateQueue(int32 MaxUpdates)
{
	int32 UpdatesProcessed = 0;

	while (UpdateQueue.Num() > 0 && UpdatesProcessed < MaxUpdates)
	{
		TSharedPtr<FQuadTreeNode> Node = UpdateQueue[0];
		UpdateQueue.RemoveAt(0);

		if (Node.IsValid() && Node->bNeedsUpdate)
		{
			GenerateMeshForNode(Node);
			Node->bNeedsUpdate = false;
			UpdatesProcessed++;
		}
	}
}

float UQuadTreeTerrain::CalculateDistanceToCamera(const FVector2D& Position, const FVector2D& CameraPosition2D) const
{
	return FVector2D::Distance(Position, CameraPosition2D);
}

bool UQuadTreeTerrain::CanSplitThisFrame() const
{
	if (!LODManager)
	{
		return true;
	}

	return LODManager->CurrentStatistics.SplitsThisFrame < LODManager->MaxSplitsPerFrame;
}

bool UQuadTreeTerrain::CanMergeThisFrame() const
{
	if (!LODManager)
	{
		return true;
	}

	return LODManager->CurrentStatistics.MergesThisFrame < LODManager->MaxMergesPerFrame;
}

void UQuadTreeTerrain::RegenerateTerrain()
{
	InitializeTerrain();
}

void UQuadTreeTerrain::CleanupTerrain()
{
	if (RootNode.IsValid())
	{
		// This will recursively clean up all children
		RootNode.Reset();
	}

	UpdateQueue.Empty();
}

float UQuadTreeTerrain::GetHeightAtPosition(const FVector& WorldPosition) const
{
	FVector2D Position2D(WorldPosition.X, WorldPosition.Y);
	return CalculateHeightAt(Position2D);
}

FVector UQuadTreeTerrain::GetNormalAtPosition(const FVector& WorldPosition) const
{
	FVector2D Position2D(WorldPosition.X, WorldPosition.Y);
	return CalculateNormalAt(Position2D);
}

FLODStatistics UQuadTreeTerrain::GetLODStatistics() const
{
	FLODStatistics Stats;

	if (RootNode.IsValid())
	{
		CollectStatistics(RootNode, Stats);
	}

	if (LODManager)
	{
		Stats.SplitsThisFrame = LODManager->CurrentStatistics.SplitsThisFrame;
		Stats.MergesThisFrame = LODManager->CurrentStatistics.MergesThisFrame;
		Stats.MeshUpdatesThisFrame = LODManager->CurrentStatistics.MeshUpdatesThisFrame;
		Stats.UpdateTimeMS = LODManager->CurrentStatistics.UpdateTimeMS;
	}

	return Stats;
}
