# ⚡ Performance Optimization Guide

> **Maximize performance across all platforms**

---

## 📑 Table of Contents

1. [Platform Configurations](#platform-configurations)
2. [LOD Optimization](#lod-optimization)
3. [GPU Compute Shaders](#gpu-compute-shaders)
4. [Multi-threading](#multi-threading)
5. [Memory Management](#memory-management)
6. [Mesh Generation](#mesh-generation)
7. [Foliage Optimization](#foliage-optimization)
8. [Profiling Tools](#profiling-tools)
9. [Common Bottlenecks](#common-bottlenecks)
10. [Platform-Specific Tips](#platform-specific-tips)

---

## 🎮 Platform Configurations

### High-End PC (RTX 3080 / RX 6800 XT+)

**Target**: 60+ FPS at 1440p/4K, Max quality

```cpp
// Terrain Configuration
TerrainSize = 20000.0f;           // 20km x 20km
MaxLODLevel = 6;                  // Very detailed
MeshResolution = 128;             // High poly at LOD 0
LODDistanceMultiplier = 2.0f;     // Balanced
UpdateFrequency = 1;              // Every frame
MaxMeshGenerationsPerFrame = 10;  // Aggressive generation
bEnableAsyncGeneration = true;    // Non-blocking

// Generation Parameters
Octaves = 8;                      // Maximum detail
HeightScale = 2000.0f;            // Large mountains
Frequency = 0.003f;               // Fine detail

// Erosion
NumIterations = 200000;           // Very detailed erosion
bUseGPUErosion = true;            // GPU acceleration
ErosionRadius = 4;                // Larger brush

// Foliage
VegetationDensity = 1.0f;         // Maximum
bUseHISM = true;                  // Hierarchical instancing
CullingDistance = 10000.0f;       // Far culling
bEnableFrustumCulling = true;
```

**Expected Performance:**
- FPS: 60-120
- Draw Calls: 500-1000
- Vertices: 5-10M
- Triangles: 8-15M
- Memory: 4-6 GB VRAM

---

### Mid-Range PC (GTX 1660 / RX 5600 XT)

**Target**: 60 FPS at 1080p, High quality

```cpp
// Terrain Configuration
TerrainSize = 15000.0f;           // 15km x 15km
MaxLODLevel = 5;                  // Good detail
MeshResolution = 64;              // Medium poly
LODDistanceMultiplier = 2.5f;     // More aggressive LOD
UpdateFrequency = 2;              // Every other frame
MaxMeshGenerationsPerFrame = 5;   // Moderate generation
bEnableAsyncGeneration = true;

// Generation Parameters
Octaves = 6;                      // Good detail
HeightScale = 1000.0f;
Frequency = 0.005f;

// Erosion
NumIterations = 100000;           // Moderate erosion
bUseGPUErosion = true;
ErosionRadius = 3;

// Foliage
VegetationDensity = 0.7f;         // Reduced density
bUseHISM = true;
CullingDistance = 7000.0f;
bEnableFrustumCulling = true;
```

**Expected Performance:**
- FPS: 60-90
- Draw Calls: 300-600
- Vertices: 2-4M
- Triangles: 3-6M
- Memory: 2-4 GB VRAM

---

### Console (PS5 / Xbox Series X)

**Target**: 60 FPS at 4K (or 120 FPS at 1080p), Optimized quality

```cpp
// Terrain Configuration
TerrainSize = 15000.0f;
MaxLODLevel = 5;
MeshResolution = 64;
LODDistanceMultiplier = 2.5f;
UpdateFrequency = 2;
MaxMeshGenerationsPerFrame = 6;
bEnableAsyncGeneration = true;

// Use GPU compute shaders (console GPUs are strong)
bUseGPUErosion = true;
bUseGPUBiomes = true;
bUseGPUNormals = true;

// Generation Parameters
Octaves = 6;
HeightScale = 1200.0f;
Frequency = 0.005f;

// Foliage
VegetationDensity = 0.8f;
bUseHISM = true;
CullingDistance = 8000.0f;
bEnableFrustumCulling = true;

// Special console optimizations
bUseDynamicResolution = true;     // DRS for stable framerate
bEnableAsyncCompute = true;       // Use async compute queues
```

**Expected Performance:**
- FPS: 60 (or 120 at lower res)
- Draw Calls: 400-700
- Vertices: 3-5M
- Triangles: 4-8M
- Memory: 3-5 GB VRAM

---

### Last-Gen Console (PS4 / Xbox One)

**Target**: 30 FPS at 1080p, Medium quality

```cpp
// Terrain Configuration
TerrainSize = 10000.0f;           // Smaller world
MaxLODLevel = 4;                  // Fewer LOD levels
MeshResolution = 48;              // Lower poly
LODDistanceMultiplier = 3.0f;     // Aggressive LOD
UpdateFrequency = 3;              // Every 3 frames
MaxMeshGenerationsPerFrame = 3;   // Conservative
bEnableAsyncGeneration = true;

// Generation Parameters
Octaves = 5;
HeightScale = 800.0f;
Frequency = 0.006f;

// Erosion
NumIterations = 50000;            // Lighter erosion
bUseGPUErosion = false;           // Use CPU
ErosionRadius = 2;

// Foliage
VegetationDensity = 0.5f;         // Half density
bUseHISM = true;
CullingDistance = 5000.0f;
bEnableFrustumCulling = true;
bEnableDistanceCulling = true;
```

**Expected Performance:**
- FPS: 30-40
- Draw Calls: 200-400
- Vertices: 1-2M
- Triangles: 1.5-3M
- Memory: 1.5-2.5 GB VRAM

---

### Mobile (High-End)

**Target**: 30-60 FPS, Optimized for battery

```cpp
// Terrain Configuration
TerrainSize = 5000.0f;            // Small world
MaxLODLevel = 3;                  // Few LOD levels
MeshResolution = 32;              // Very low poly
LODDistanceMultiplier = 4.0f;     // Very aggressive LOD
UpdateFrequency = 5;              // Every 5 frames
MaxMeshGenerationsPerFrame = 2;   // Very conservative
bEnableAsyncGeneration = false;   // Simpler threading

// Generation Parameters
Octaves = 4;                      // Minimal octaves
HeightScale = 400.0f;
Frequency = 0.01f;

// Erosion
NumIterations = 10000;            // Light erosion
bUseGPUErosion = false;           // CPU only (mobile GPU limited)
ErosionRadius = 2;

// Foliage
VegetationDensity = 0.3f;         // Very sparse
bUseHISM = true;                  // Essential on mobile
CullingDistance = 3000.0f;
bEnableFrustumCulling = true;
bEnableDistanceCulling = true;
MaxInstancesPerCell = 50;         // Hard limit

// Mobile-specific
bUseSimplifiedShaders = true;
bDisableShadowsOnFoliage = true;
MaterialQuality = Low;
```

**Expected Performance:**
- FPS: 30-60
- Draw Calls: 100-200
- Vertices: 200K-500K
- Triangles: 300K-800K
- Memory: 512 MB - 1 GB

---

## 🎚️ LOD Optimization

### LOD Distance Configuration

**Formula**: `LODThreshold = NodeSize * LODDistanceMultiplier * (1 << LODLevel)`

```cpp
// Conservative (better quality, lower performance)
LODDistanceMultiplier = 1.5f;

// Balanced (recommended)
LODDistanceMultiplier = 2.0f;

// Aggressive (better performance, more visible transitions)
LODDistanceMultiplier = 3.0f;

// Very Aggressive (mobile/low-end)
LODDistanceMultiplier = 4.0f;
```

### Custom LOD Per Biome

```cpp
// Configure LOD for mountain biome (needs more detail)
FLODConfiguration MountainLOD;
MountainLOD.LODDistances = {0, 1500, 4000, 8000, 15000, 30000};
MountainLOD.MeshResolutions = {128, 64, 32, 16, 8, 4};
MountainLOD.CollisionEnabled = {true, true, false, false, false, false};

LODManager->SetLODConfiguration(TEXT("Mountain"), MountainLOD);

// Configure LOD for flat desert (can use aggressive LOD)
FLODConfiguration DesertLOD;
DesertLOD.LODDistances = {0, 3000, 7000, 15000, 30000};
DesertLOD.MeshResolutions = {64, 32, 16, 8, 4};
DesertLOD.CollisionEnabled = {true, false, false, false, false};

LODManager->SetLODConfiguration(TEXT("Desert"), DesertLOD);
```

### Mesh Resolution Guidelines

| LOD Level | Resolution | Triangles/Chunk | Use Case |
|-----------|-----------|-----------------|----------|
| 0 | 128x128 | 32,512 | Very close, detailed areas |
| 1 | 64x64 | 8,064 | Close range |
| 2 | 32x32 | 1,984 | Medium range |
| 3 | 16x16 | 480 | Far range |
| 4 | 8x8 | 112 | Very far range |
| 5 | 4x4 | 24 | Extremely far |

**Memory Impact**:
- 128x128: ~65KB per chunk
- 64x64: ~16KB per chunk
- 32x32: ~4KB per chunk

### LOD Transition Smoothing

```cpp
// Enable smooth LOD transitions
bSmoothLODTransitions = true;
LODTransitionDuration = 0.3f;  // Seconds

// Use morphing for smoother transitions
bUseLODMorphing = true;
MorphingRange = 0.2f;  // Percentage of LOD distance

// Geomorphing in vertex shader
// This smoothly transitions vertices between LOD levels
```

---

## 🖥️ GPU Compute Shaders

### When to Use GPU vs CPU

**Use GPU Compute Shaders When:**
- ✅ Large heightmaps (512x512+)
- ✅ High iteration counts (100K+ erosion droplets)
- ✅ Baking/preprocessing (not runtime)
- ✅ Multiple terrains to process
- ✅ Platform has strong GPU (PC, Console)

**Use CPU When:**
- ✅ Small heightmaps (<256x256)
- ✅ Runtime generation needed immediately
- ✅ Mobile platforms
- ✅ Low-end GPUs
- ✅ Need debugging/iteration

### GPU Erosion Performance

```cpp
// GPU Erosion - Fast!
FErosionParameters Params;
Params.NumDroplets = 1000000;  // 1 Million!

TArray<float> OutputMap;
FErosionGPUInterface::RunErosion(
    InputHeightMap,
    1024,  // 1024x1024
    OutputMap,
    Params.NumDroplets,
    &Params
);

// Typical timing:
// - RTX 3080:  8-12 seconds
// - RTX 2060:  15-25 seconds
// - GTX 1660:  25-40 seconds
```

```cpp
// CPU Erosion - Slower but flexible
UHydraulicErosion* Erosion = NewObject<UHydraulicErosion>();
Erosion->NumIterations = 100000;  // 100K
Erosion->Erode(HeightMap, 512, false);

// Typical timing:
// - Ryzen 9 / i9: 5-10 seconds
// - Ryzen 5 / i5: 10-20 seconds
// - Mobile: 30-60+ seconds
```

### Shader Dispatch Optimization

```cpp
// Compute shader dispatch size
// Balance between occupancy and overhead

// Good for large maps (1024x1024+)
ThreadGroupSize = {8, 8, 1};
NumThreadGroups = {MapSize / 8, MapSize / 8, 1};

// Better for medium maps (512x512)
ThreadGroupSize = {16, 16, 1};
NumThreadGroups = {MapSize / 16, MapSize / 16, 1};

// Ensure map size is multiple of thread group size!
// Use power-of-2 sizes: 256, 512, 1024, 2048
```

### GPU Memory Considerations

```cpp
// Monitor GPU memory usage
FTextureMemoryStats Stats;
RHIGetTextureMemoryStats(Stats);

UE_LOG(LogTemp, Log, TEXT("GPU Memory: %.2f MB / %.2f MB"),
    Stats.AllocatedMemorySize / 1024.0f / 1024.0f,
    Stats.TotalGraphicsMemory / 1024.0f / 1024.0f);

// Limit heightmap size based on available memory
const int32 MaxMapSize = FMath::Min(
    4096,
    FMath::FloorToInt(FMath::Sqrt(AvailableMemory / sizeof(float)))
);
```

---

## 🔄 Multi-threading

### Async Mesh Generation

```cpp
// Enable async generation
bEnableAsyncGeneration = true;
MaxMeshGenerationsPerFrame = 5;

// This spreads mesh generation across multiple frames
// Prevents frame spikes when generating many chunks

// Monitor generation queue
int32 QueueSize = UpdateQueue.Num();
if (QueueSize > 50)
{
    // Queue is backing up, reduce generation rate
    MaxMeshGenerationsPerFrame = FMath::Max(1, MaxMeshGenerationsPerFrame - 1);
}
else if (QueueSize < 5)
{
    // Queue is empty, can increase rate
    MaxMeshGenerationsPerFrame = FMath::Min(10, MaxMeshGenerationsPerFrame + 1);
}
```

### Task Graph Parallelization

```cpp
// Create async task for terrain generation
FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
    [this, Node]()
    {
        // This runs on task graph thread
        GenerateMeshDataForNode(Node);
    },
    TStatId(),
    nullptr,
    ENamedThreads::AnyBackgroundThreadNormalTask
);

// Continue on game thread when done
FFunctionGraphTask::CreateAndDispatchWhenReady(
    [this, Node]()
    {
        // This runs on game thread
        ApplyMeshDataToComponent(Node);
    },
    TStatId(),
    Task,  // Wait for previous task
    ENamedThreads::GameThread
);
```

### Parallel For Erosion

```cpp
void UHydraulicErosion::Erode(TArray<float>& HeightMap, int32 MapSize, bool bAsync)
{
    if (bAsync && NumIterations > 10000)
    {
        // Split into batches for parallel processing
        const int32 BatchSize = 1000;
        const int32 NumBatches = FMath::CeilToInt(NumIterations / (float)BatchSize);

        ParallelFor(NumBatches, [&](int32 BatchIndex)
        {
            FRandomStream LocalRandom(RandomSeed + BatchIndex);

            for (int32 i = 0; i < BatchSize; ++i)
            {
                SimulateDroplet(HeightMap, MapSize, LocalRandom);
            }
        });
    }
    else
    {
        // Single-threaded for small iterations
        for (int32 i = 0; i < NumIterations; ++i)
        {
            SimulateDroplet(HeightMap, MapSize, RandomStream);
        }
    }
}
```

### Thread Pool Configuration

```cpp
// In DefaultEngine.ini
[/Script/Engine.Engine]
; Adjust thread pool size based on platform
TaskGraph.NumBackgroundThreads=4  ; High-end PC
; TaskGraph.NumBackgroundThreads=2  ; Mid-range
; TaskGraph.NumBackgroundThreads=1  ; Mobile

; Task priorities
TaskGraph.bEnableThreadPriorities=true
```

---

## 💾 Memory Management

### Mesh Component Pooling

```cpp
class UQuadTreeTerrain
{
private:
    // Pool of reusable mesh components
    TArray<UProceduralMeshComponent*> MeshComponentPool;
    const int32 MaxPoolSize = 100;

public:
    UProceduralMeshComponent* GetOrCreateMeshComponent(TSharedPtr<FQuadTreeNode> Node)
    {
        UProceduralMeshComponent* Mesh = nullptr;

        if (!MeshComponentPool.IsEmpty())
        {
            // Reuse from pool
            Mesh = MeshComponentPool.Pop();
            Mesh->SetVisibility(true);
            Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        else
        {
            // Create new if pool empty
            Mesh = NewObject<UProceduralMeshComponent>(GetOwner());
            Mesh->RegisterComponent();
            Mesh->AttachToComponent(GetOwner()->GetRootComponent(),
                FAttachmentTransformRules::KeepRelativeTransform);
        }

        return Mesh;
    }

    void ReturnMeshComponentToPool(UProceduralMeshComponent* Mesh)
    {
        if (MeshComponentPool.Num() < MaxPoolSize)
        {
            // Clear mesh data
            Mesh->ClearAllMeshSections();
            Mesh->SetVisibility(false);
            Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            // Return to pool
            MeshComponentPool.Add(Mesh);
        }
        else
        {
            // Pool full, destroy
            Mesh->DestroyComponent();
        }
    }
};
```

### Heightmap Memory Optimization

```cpp
// Use 16-bit height values instead of 32-bit floats
// Reduces memory by 50%

TArray<uint16> CompressHeightMap(const TArray<float>& FloatHeightMap,
                                  float MinHeight, float MaxHeight)
{
    TArray<uint16> CompressedMap;
    CompressedMap.Reserve(FloatHeightMap.Num());

    float Range = MaxHeight - MinHeight;

    for (float Height : FloatHeightMap)
    {
        float Normalized = (Height - MinHeight) / Range;
        uint16 Compressed = FMath::Clamp(
            FMath::RoundToInt(Normalized * 65535.0f),
            0,
            65535
        );
        CompressedMap.Add(Compressed);
    }

    return CompressedMap;
}

float DecompressHeight(uint16 Compressed, float MinHeight, float MaxHeight)
{
    float Normalized = Compressed / 65535.0f;
    return MinHeight + Normalized * (MaxHeight - MinHeight);
}
```

### Memory Budget Management

```cpp
class FTerrainMemoryManager
{
private:
    SIZE_T MaxMemoryBudget;     // In bytes
    SIZE_T CurrentMemoryUsage;

public:
    FTerrainMemoryManager()
    {
        // Set budget based on platform
        if (IsRunningOnConsole())
        {
            MaxMemoryBudget = 512 * 1024 * 1024;  // 512 MB
        }
        else if (IsRunningOnMobile())
        {
            MaxMemoryBudget = 256 * 1024 * 1024;  // 256 MB
        }
        else
        {
            MaxMemoryBudget = 2048 * 1024 * 1024; // 2 GB
        }
    }

    bool CanAllocate(SIZE_T Size)
    {
        return (CurrentMemoryUsage + Size) <= MaxMemoryBudget;
    }

    void TrackAllocation(SIZE_T Size)
    {
        CurrentMemoryUsage += Size;

        // Log if approaching budget
        if (CurrentMemoryUsage > MaxMemoryBudget * 0.9f)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Terrain memory usage high: %.2f MB / %.2f MB"),
                CurrentMemoryUsage / 1024.0f / 1024.0f,
                MaxMemoryBudget / 1024.0f / 1024.0f);
        }
    }

    void TrackDeallocation(SIZE_T Size)
    {
        CurrentMemoryUsage -= Size;
    }

    float GetMemoryUsagePercent() const
    {
        return (CurrentMemoryUsage / (float)MaxMemoryBudget) * 100.0f;
    }
};
```

### Chunk Streaming

```cpp
// Stream chunks based on distance
void UQuadTreeTerrain::UpdateChunkStreaming(const FVector& ViewerLocation)
{
    const float StreamInDistance = TerrainSize * 0.6f;
    const float StreamOutDistance = TerrainSize * 0.8f;

    for (auto& Chunk : AllChunks)
    {
        float Distance = FVector::Dist2D(Chunk.Center, ViewerLocation);

        if (Distance < StreamInDistance && !Chunk.bIsLoaded)
        {
            // Stream in
            if (MemoryManager.CanAllocate(Chunk.EstimatedMemorySize))
            {
                LoadChunk(Chunk);
            }
        }
        else if (Distance > StreamOutDistance && Chunk.bIsLoaded)
        {
            // Stream out
            UnloadChunk(Chunk);
        }
    }
}
```

---

## 🎨 Mesh Generation

### Optimized Vertex Buffer Creation

```cpp
void UQuadTreeTerrain::GenerateMeshForNode(TSharedPtr<FQuadTreeNode> Node)
{
    // Pre-allocate arrays with exact size
    int32 Resolution = MeshResolution >> Node->LODLevel;
    int32 VertexCount = (Resolution + 1) * (Resolution + 1);
    int32 TriangleCount = Resolution * Resolution * 6;  // 2 triangles per quad

    TArray<FVector> Vertices;
    Vertices.Reserve(VertexCount);

    TArray<int32> Triangles;
    Triangles.Reserve(TriangleCount);

    TArray<FVector> Normals;
    Normals.Reserve(VertexCount);

    TArray<FVector2D> UVs;
    UVs.Reserve(VertexCount);

    // Generate with pre-allocated space (no reallocation!)
    // ... mesh generation code ...

    // Create mesh section
    Node->MeshComponent->CreateMeshSection_LinearColor(
        0,
        Vertices,
        Triangles,
        Normals,
        UVs,
        TArray<FLinearColor>(),  // Empty vertex colors
        TArray<FProcMeshTangent>(),  // Empty tangents (calculated in shader)
        bEnableCollision
    );
}
```

### Normal Calculation Optimization

```cpp
// Fast normal calculation using cross product
FVector CalculateNormalFast(const FVector& V0, const FVector& V1, const FVector& V2)
{
    FVector Edge1 = V1 - V0;
    FVector Edge2 = V2 - V0;
    return (Edge1 ^ Edge2).GetSafeNormal();
}

// Smooth normals (averaged from adjacent triangles)
void CalculateSmoothNormals(TArray<FVector>& Vertices,
                           const TArray<int32>& Triangles,
                           TArray<FVector>& Normals)
{
    // Initialize normals to zero
    Normals.SetNumZeroed(Vertices.Num());

    // Accumulate normals from all triangles
    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        FVector Normal = CalculateNormalFast(
            Vertices[Triangles[i]],
            Vertices[Triangles[i + 1]],
            Vertices[Triangles[i + 2]]
        );

        Normals[Triangles[i]] += Normal;
        Normals[Triangles[i + 1]] += Normal;
        Normals[Triangles[i + 2]] += Normal;
    }

    // Normalize
    for (FVector& Normal : Normals)
    {
        Normal.Normalize();
    }
}
```

### Index Buffer Optimization

```cpp
// Use triangle strips for better cache coherency
// This reduces vertex fetches by ~40%

void GenerateTriangleStrip(int32 Resolution,
                          TArray<int32>& Triangles)
{
    Triangles.Empty();
    Triangles.Reserve(Resolution * Resolution * 6);

    for (int32 Y = 0; Y < Resolution; ++Y)
    {
        for (int32 X = 0; X < Resolution; ++X)
        {
            int32 i0 = Y * (Resolution + 1) + X;
            int32 i1 = i0 + 1;
            int32 i2 = i0 + (Resolution + 1);
            int32 i3 = i2 + 1;

            // Alternate winding order for better cache coherency
            if ((X + Y) % 2 == 0)
            {
                Triangles.Append({i0, i2, i1});
                Triangles.Append({i1, i2, i3});
            }
            else
            {
                Triangles.Append({i0, i2, i3});
                Triangles.Append({i0, i3, i1});
            }
        }
    }
}
```

---

## 🌳 Foliage Optimization

### Hierarchical Instanced Static Mesh (HISM)

```cpp
// ALWAYS use HISM for foliage, never individual actors!

UHierarchicalInstancedStaticMeshComponent* CreateFoliageHISM(UStaticMesh* Mesh)
{
    UHierarchicalInstancedStaticMeshComponent* HISM =
        NewObject<UHierarchicalInstancedStaticMeshComponent>(this);

    HISM->SetStaticMesh(Mesh);
    HISM->SetMobility(EComponentMobility::Static);

    // Performance settings
    HISM->SetCullDistances(3000, 10000);  // Start/End cull distance
    HISM->bUseAsOccluder = false;         // Foliage shouldn't occlude
    HISM->CastShadow = true;              // But should cast shadows
    HISM->bCastDynamicShadow = false;     // Static shadows only
    HISM->bAffectDistanceFieldLighting = true;
    HISM->bAffectDynamicIndirectLighting = false;

    // Culling
    HISM->InstanceStartCullDistance = 3000;
    HISM->InstanceEndCullDistance = 10000;
    HISM->bEnableAutoLODGeneration = true;

    // Collision
    HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Enable collision only for important vegetation

    HISM->RegisterComponent();
    return HISM;
}
```

### Density-Based LOD

```cpp
// Reduce foliage density at distance

float CalculateFoliageDensity(float Distance, float MaxDistance)
{
    if (Distance < MaxDistance * 0.3f)
    {
        return 1.0f;  // Full density
    }
    else if (Distance < MaxDistance * 0.6f)
    {
        return 0.5f;  // Half density
    }
    else if (Distance < MaxDistance)
    {
        return 0.25f;  // Quarter density
    }
    else
    {
        return 0.0f;  // No foliage
    }
}

void SpawnFoliageWithLOD(const FVector& Position, float Distance)
{
    float DensityMultiplier = CalculateFoliageDensity(Distance, 10000.0f);
    int32 InstanceCount = FMath::RoundToInt(BaseInstanceCount * DensityMultiplier);

    for (int32 i = 0; i < InstanceCount; ++i)
    {
        // Spawn instance
    }
}
```

### Frustum Culling

```cpp
// Implement custom frustum culling for foliage cells

bool IsFoliageCellVisible(const FBox& CellBounds, const FConvexVolume& ViewFrustum)
{
    return ViewFrustum.IntersectBox(CellBounds.GetCenter(), CellBounds.GetExtent());
}

void UpdateFoliageVisibility(const FConvexVolume& ViewFrustum)
{
    for (auto& Cell : FoliageCells)
    {
        bool bVisible = IsFoliageCellVisible(Cell.Bounds, ViewFrustum);

        if (bVisible != Cell.bWasVisible)
        {
            Cell.HISM->SetVisibility(bVisible);
            Cell.bWasVisible = bVisible;
        }
    }
}
```

### Grass System Optimization

```cpp
// Use UE's runtime virtual texturing for grass

UPROPERTY(EditAnywhere, Category = "Grass")
URuntimeVirtualTexture* GrassDensityRVT;

// In material, sample RVT for grass density
// This allows dynamic grass placement without CPU overhead

// Alternative: Use compute shader for grass placement
void GenerateGrassGPU(const FBox& Bounds, URuntimeVirtualTexture* DensityRVT)
{
    // Dispatch compute shader
    // Read density from RVT
    // Output instance transforms
    // Much faster than CPU placement
}
```

---

## 📊 Profiling Tools

### Built-in Console Commands

```
# FPS and frame time
stat fps
stat unit
stat unitgraph

# GPU profiling
stat gpu
profilegpu

# Rendering stats
stat scenerendering
stat rhi
stat d3d12 (or stat d3d11, stat opengl, stat vulkan)

# Memory
stat memory
stat streaming

# LOD stats
stat lodinfo
stat levels

# Foliage
stat foliage
stat grass

# Threading
stat taskgraph
stat threading

# Specific systems
stat terrain
stat proceduralmap (if custom stat group created)
```

### Custom Stat Groups

```cpp
// In your module
DECLARE_STATS_GROUP(TEXT("ProceduralMap"), STATGROUP_ProceduralMap, STATCAT_Advanced);

// Individual stats
DECLARE_CYCLE_STAT(TEXT("Generate Mesh"), STAT_GenerateMesh, STATGROUP_ProceduralMap);
DECLARE_CYCLE_STAT(TEXT("Update LOD"), STAT_UpdateLOD, STATGROUP_ProceduralMap);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Chunks"), STAT_ActiveChunks, STATGROUP_ProceduralMap);
DECLARE_MEMORY_STAT(TEXT("Terrain Memory"), STAT_TerrainMemory, STATGROUP_ProceduralMap);

// Use in code
void UQuadTreeTerrain::GenerateMeshForNode(TSharedPtr<FQuadTreeNode> Node)
{
    SCOPE_CYCLE_COUNTER(STAT_GenerateMesh);

    // ... mesh generation code ...
}

void UQuadTreeTerrain::UpdateLOD(const FVector& CameraPosition)
{
    SCOPE_CYCLE_COUNTER(STAT_UpdateLOD);

    INC_DWORD_STAT(STAT_ActiveChunks);

    // ... LOD update code ...
}
```

### GPU Profiler Analysis

```cpp
// Add GPU profiler events
SCOPED_GPU_STAT(RHICmdList, TerrainGeneration);

// In render thread
ENQUEUE_RENDER_COMMAND(GenerateTerrainGPU)(
    [](FRHICommandListImmediate& RHICmdList)
    {
        SCOPED_DRAW_EVENT(RHICmdList, TerrainComputeShader);
        SCOPED_GPU_STAT(RHICmdList, TerrainCompute);

        // Dispatch compute shader
        DispatchComputeShader(RHICmdList, ...);
    }
);
```

### Unreal Insights

```
# Launch with tracing
UnrealEditor.exe -trace=cpu,gpu,frame,bookmark

# Or in game
Trace.Start

# Stop tracing
Trace.Stop

# Analyze in Unreal Insights application
```

---

## 🚨 Common Bottlenecks

### 1. Too Many Draw Calls

**Symptom**: Low GPU usage, CPU bound

**Solutions**:
```cpp
// Reduce mesh sections
// - Don't create separate mesh section per material
// - Use material indices instead

// Use instancing
// - HISM for foliage
// - Instanced materials

// Increase LOD distances
LODDistanceMultiplier = 3.0f;  // More aggressive

// Reduce max LOD levels
MaxLODLevel = 4;  // Instead of 6
```

### 2. Mesh Generation Spikes

**Symptom**: Frame time spikes when moving

**Solutions**:
```cpp
// Enable async generation
bEnableAsyncGeneration = true;

// Limit per frame
MaxMeshGenerationsPerFrame = 3;

// Increase update frequency (update less often)
UpdateFrequency = 3;  // Every 3 frames

// Pre-generate meshes
PreGenerateMeshesAroundPosition(PlayerStartLocation, Radius);
```

### 3. Memory Leaks

**Symptom**: Memory usage increasing over time

**Solutions**:
```cpp
// Use mesh component pooling (see earlier)
// Don't forget to clean up!

void UQuadTreeTerrain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Clean up all nodes
    CleanupTerrain();

    // Clear pools
    for (UProceduralMeshComponent* Mesh : MeshComponentPool)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    MeshComponentPool.Empty();
}
```

### 4. Shader Compilation Stutters

**Symptom**: Hitches when new materials/shaders compile

**Solutions**:
```
# Cook shaders beforehand
# In DefaultEngine.ini
[/Script/Engine.Engine]
bPrecompileShadersForDevelopment=true

# Use shader caching
r.ShaderDevelopmentMode=0
r.Shaders.Optimize=1

# PSO caching (UE5)
r.ShaderPipelineCache.Enabled=1
r.ShaderPipelineCache.LogPSO=1
```

### 5. Collision Performance

**Symptom**: Physics queries are slow

**Solutions**:
```cpp
// Disable collision on distant LODs
if (Node->LODLevel > 2)
{
    Node->MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Use simple collision
Node->MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
CollisionComplexity = ECollisionTraceFlag::CTF_UseSimpleAsComplex;

// Async collision cooking
bUseAsyncCooking = true;
```

---

## 🎯 Platform-Specific Tips

### PC Optimizations

```cpp
// Use all available threads
TaskGraph.NumBackgroundThreads=8  // For 8+ core CPUs

// Enable DX12/Vulkan for better multi-threading
-dx12
-vulkan

// Use GPU compute shaders extensively
bUseGPUErosion = true;
bUseGPUBiomes = true;
bUseGPUNormals = true;

// High resolution textures
r.Streaming.PoolSize=4096  // 4 GB texture pool
```

### Console Optimizations

```cpp
// Use async compute
bEnableAsyncCompute = true;

// Dynamic resolution scaling
r.DynamicRes.OperationMode=2
r.DynamicRes.TargetedGPUHeadRoomPercentage=10

// Optimize for 60 FPS
t.MaxFPS=60

// Reduce shadow quality if needed
r.Shadow.MaxResolution=2048
r.ShadowQuality=3

// Use temporal upsampling
r.TemporalAA.Upsampling=1
```

### Mobile Optimizations

```cpp
// Aggressive culling
r.MobileMSAA=0  // Disable MSAA
r.MobileContentScaleFactor=0.8  // Render at lower res

// Simplified shaders
r.Mobile.Forward.EnableLocalLights=0
r.Mobile.EnableStaticAndCSMShadowReceivers=1

// Limit draw distance
r.ViewDistanceScale=0.5

// No async
bEnableAsyncGeneration=false
bUseGPUCompute=false

// Minimal foliage
VegetationDensity=0.2
bDisableFoliageShadows=true
```

### VR Optimizations

```cpp
// High framerate is critical!
t.MaxFPS=90  // Or 120 for high-end VR

// Instanced stereo rendering
vr.InstancedStereo=1

// Reduce LOD transitions (jarring in VR)
LODDistanceMultiplier=1.5  // Less aggressive

// Shorter view distance
r.ViewDistanceScale=0.7

// Fixed foveated rendering (if supported)
vr.foveation.Level=2
```

---

## 📈 Performance Targets

### PC

| Quality | Resolution | FPS | Settings |
|---------|-----------|-----|----------|
| Ultra   | 4K        | 60+ | Max everything |
| High    | 1440p     | 60+ | High settings, aggressive LOD |
| Medium  | 1080p     | 60+ | Medium settings, moderate LOD |
| Low     | 1080p     | 60+ | Low settings, very aggressive LOD |

### Console

| Platform | Resolution | FPS | Notes |
|----------|-----------|-----|-------|
| PS5/XSX  | 4K        | 60  | Use DRS, async compute |
| PS5/XSX  | 1080p     | 120 | Performance mode |
| PS4/XB1  | 1080p     | 30  | Aggressive optimizations |

### Mobile

| Device    | Resolution | FPS | Notes |
|-----------|-----------|-----|-------|
| High-end  | 1080p     | 60  | Snapdragon 888+, A14+ |
| Mid-range | 720p      | 30  | Snapdragon 765, A12 |
| Low-end   | 540p      | 30  | Minimal features |

---

<div align="center">

**Monitor your performance and adjust settings based on target platform!**

[⬆ Back to Top](#-performance-optimization-guide) | [README](./README.md) | [Documentation](./DOCUMENTATION.md)

</div>
