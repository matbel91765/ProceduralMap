# ⚡ Quick Start Guide

> **Get your first procedural terrain running in 5 minutes!**

---

## 🎯 Prerequisites

- ✅ Unreal Engine **5.4** or higher installed
- ✅ Basic familiarity with Unreal Editor
- ✅ Project with ProceduralMap plugin enabled

---

## 📦 Step 1: Installation (2 minutes)

### From Marketplace
```
1. Open Epic Games Launcher
2. Go to Marketplace → Library → Vault
3. Find "Ultimate Procedural Map"
4. Click "Install to Engine" (select UE 5.4+)
5. Open your project
6. Edit → Plugins → Search "Procedural Map"
7. Enable the plugin ✅
8. Restart Unreal Editor
```

### Manual Installation
```
1. Copy ProceduralMap folder to YourProject/Plugins/
2. Right-click YourProject.uproject → Generate Visual Studio Files
3. Build project in Visual Studio
4. Launch Unreal Editor
```

---

## 🌍 Step 2: Create Your First Terrain (2 minutes)

### Method A: Blueprint Actor (Easiest)

1. **Open Content Browser**
   ```
   Content Browser → Plugins/ProceduralMap Content/Blueprints/Examples
   ```

2. **Drag BP_QuadTreeTerrain into Level**
   ```
   - Position: (0, 0, 0)
   - This creates a basic terrain actor
   ```

3. **Configure in Details Panel**
   ```
   🔧 Terrain Configuration:
   - Terrain Size: 10000 (10km x 10km)
   - Max LOD Level: 4
   - LOD Distance Multiplier: 2.0
   - Mesh Resolution: 64

   🎨 Terrain Generation:
   - Seed: 12345 (try different numbers!)
   - Frequency: 0.005
   - Octaves: 6
   - Lacunarity: 2.0
   - Gain: 0.5
   - Height Scale: 500
   - Height Offset: 0

   🎨 Terrain Rendering:
   - Terrain Material: (assign a material)
   - Enable Collision: ✅
   ```

4. **Press Play!**
   ```
   - Click Play (Alt+P)
   - Fly around to see LOD system in action
   - Watch chunks load/unload dynamically
   ```

### Method B: C++ (Advanced)

1. **Create Terrain Actor**
   ```cpp
   // In your actor header
   UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
   UQuadTreeTerrain* TerrainComponent;
   ```

2. **Initialize in Constructor**
   ```cpp
   AMyTerrainActor::AMyTerrainActor()
   {
       TerrainComponent = CreateDefaultSubobject<UQuadTreeTerrain>(TEXT("Terrain"));

       // Configure
       TerrainComponent->TerrainSize = 10000.0f;
       TerrainComponent->MaxLODLevel = 4;
       TerrainComponent->MeshResolution = 64;

       TerrainComponent->GenerationParams.Seed = 12345;
       TerrainComponent->GenerationParams.Frequency = 0.005f;
       TerrainComponent->GenerationParams.Octaves = 6;
       TerrainComponent->GenerationParams.HeightScale = 500.0f;
   }
   ```

3. **Initialize in BeginPlay**
   ```cpp
   void AMyTerrainActor::BeginPlay()
   {
       Super::BeginPlay();
       TerrainComponent->InitializeTerrain();
   }
   ```

---

## 🎨 Step 3: Basic Material Setup (1 minute)

### Quick Material

1. **Create Material**
   ```
   Content Browser → Right-click → Material → M_Terrain
   ```

2. **Add Basic Nodes**
   ```
   Material Graph:

   [World Position] → [/10000] → [Noise] → [*0.5+0.5] → [Base Color]
                                                      ↓
                                                  [Lerp] → [Material Output]
                                                      ↑
   [World Normal.Z] → [Power 2] ─────────────────────┘

   This creates a simple height-based color gradient.
   ```

3. **Assign to Terrain**
   ```
   Select terrain actor → Details → Terrain Material → M_Terrain
   ```

### Using Provided Material

```
1. Content Browser → Plugins/ProceduralMap Content/Materials
2. Find M_TerrainMaster
3. Assign to your terrain
4. Customize material parameters in Details panel
```

---

## 🚀 Step 4: Advanced Features (Optional)

### Add Biomes

```cpp
// Blueprint
1. Add BP_BiomeGenerator to level
2. Configure biomes in Details
3. Reference in terrain actor

// C++
ABiomeGenerator* BiomeGen = GetWorld()->SpawnActor<ABiomeGenerator>();
BiomeGen->WorldSizeX = 20000.0f;
BiomeGen->WorldSizeY = 20000.0f;
```

### Apply Erosion

```cpp
// In terrain generation
UHydraulicErosion* Erosion = NewObject<UHydraulicErosion>();
Erosion->NumIterations = 50000;
Erosion->ErosionRadius = 3;
Erosion->Inertia = 0.05f;
Erosion->ErodeSpeed = 0.3f;
Erosion->DepositSpeed = 0.3f;

// Apply to heightmap
Erosion->Erode(HeightMapArray, MapSize, false);
```

### Add Rivers

```cpp
// Spawn river generator
ARiverGenerator* River = GetWorld()->SpawnActor<ARiverGenerator>();

River->SourceLocation = FVector(0, 0, 1000);
River->bAutoFindDestination = true;
River->Width = 500.0f;
River->Depth = 100.0f;
River->DownhillBias = 5.0f;

River->GenerateRiver();
```

### Generate Caves

```cpp
// Spawn cave generator
ACaveGenerator* Cave = GetWorld()->SpawnActor<ACaveGenerator>();

// Configure cellular automata
Cave->CellularAutomata->GridWidth = 100;
Cave->CellularAutomata->GridHeight = 100;
Cave->CellularAutomata->FillPercent = 0.45f;
Cave->CellularAutomata->NumIterations = 5;

// Generate
Cave->MeshMode = ECaveMeshMode::MarchingCubes3D;
Cave->VoxelSize = 100.0f;
Cave->GenerateCave();
```

---

## 📋 Common Configurations

### 🏔️ Mountain Terrain
```
Seed: 42
Frequency: 0.003
Octaves: 8
Lacunarity: 2.5
Gain: 0.6
Height Scale: 2000
Height Offset: 0

Apply Erosion:
- NumIterations: 100000
- ErosionRadius: 4
- ErodeSpeed: 0.4
```

### 🏜️ Desert Terrain
```
Seed: 777
Frequency: 0.008
Octaves: 4
Lacunarity: 2.0
Gain: 0.4
Height Scale: 200
Height Offset: 0

Apply Thermal Erosion:
- NumIterations: 15
- TalusAngle: 35
- ErosionRate: 0.6
```

### 🌊 Island Terrain
```
Seed: 1234
Frequency: 0.005
Octaves: 6
Lacunarity: 2.2
Gain: 0.5
Height Scale: 800
Height Offset: -200

Add Gradient Mask:
- Circular falloff from center
- Ocean level: -100
```

### 🌲 Forest Terrain
```
Seed: 9999
Frequency: 0.006
Octaves: 5
Lacunarity: 2.0
Gain: 0.5
Height Scale: 400
Height Offset: 0

Add Foliage:
- Tree Density: High
- Grass Density: Very High
- Biome: Temperate Forest
```

---

## 🎮 Example Presets

### Load Preset in Editor

1. **Open Preset Manager**
   ```
   Tools → Procedural Map → Preset Manager
   ```

2. **Select Preset**
   ```
   - Mountain Preset
   - Desert Preset
   - Island Preset
   - Forest Preset
   ```

3. **Apply to Terrain**
   ```
   Right-click preset → Apply to Selected Terrain
   ```

### Create Custom Preset

1. **Configure Your Terrain**
   ```
   Adjust all parameters until satisfied
   ```

2. **Save as Preset**
   ```
   Content Browser → Right-click → Misc → Terrain Preset
   Name: MyCustomTerrain
   Copy parameters from current terrain
   ```

3. **Reuse Later**
   ```
   Right-click preset → Apply to Terrain
   ```

---

## 🛠️ Editor Tools

### Custom Editor Mode

```
1. Window → Modes → Procedural Terrain
2. Select tool (keyboard 1-5):
   [1] Paint   - Raise/lower terrain
   [2] Sculpt  - Artistic sculpting
   [3] Smooth  - Smooth rough areas
   [4] Flatten - Create flat areas
   [5] Generate- Apply noise patterns

3. Adjust brush:
   [ ] Decrease brush size
   ] Increase brush size
   Shift+Drag: Adjust strength

4. Paint on terrain
```

### Real-Time Preview

```
1. Window → Procedural Map → Preview Window
2. Adjust parameters on left panel
3. See instant preview on right
4. Switch between 3D and 2D views
5. Click "Apply to Terrain" when satisfied
```

### Project Settings

```
Edit → Project Settings → Plugins → Procedural Map

🎚️ Performance Presets:
- Low (Mobile)
- Medium (PS4/Xbox One)
- High (PS5/Xbox Series X)
- Ultra (High-End PC)
- Custom

📁 Asset Paths:
- Default Material
- Texture Library
- Preset Storage

⚙️ Editor Preferences:
- Auto-save
- Preview Quality
- Update Frequency
```

---

## ⚡ Performance Tips

### For Best Performance

```
✅ Use GPU compute shaders for large terrains
✅ Enable async mesh generation
✅ Limit Max Mesh Generations Per Frame (3-5)
✅ Use appropriate LOD Distance Multiplier (2.0-3.0)
✅ Enable frustum culling
✅ Use HISM for foliage (not individual actors)
```

### For Best Quality

```
✅ High Octaves (6-8) for detail
✅ Apply hydraulic + thermal erosion
✅ Use multi-biome blending
✅ Enable smooth normals
✅ Use high mesh resolution at LOD 0 (64-128)
✅ Apply triplanar mapping for materials
```

### Platform-Specific

**High-End PC:**
```
Terrain Size: 20000+
Max LOD: 6
Mesh Resolution: 128
Update Frequency: 1
GPU Shaders: ✅
```

**Console (PS5/XSX):**
```
Terrain Size: 15000
Max LOD: 5
Mesh Resolution: 64
Update Frequency: 2
GPU Shaders: ✅
```

**Mobile:**
```
Terrain Size: 8000
Max LOD: 3
Mesh Resolution: 32
Update Frequency: 5
GPU Shaders: ❌ (use CPU)
```

---

## 🔧 Troubleshooting

### Terrain Not Appearing

```
❌ Problem: Nothing shows up after pressing Play

✅ Solutions:
1. Check TerrainMaterial is assigned
2. Verify InitializeTerrain() was called
3. Check camera is within view distance
4. Ensure HeightScale > 0
5. Try regenerating: Call RegenerateTerrain()
```

### Low Frame Rate

```
❌ Problem: FPS is very low

✅ Solutions:
1. Reduce Max LOD Level (try 3-4)
2. Decrease Mesh Resolution (try 32-48)
3. Increase Update Frequency (try 3-5)
4. Lower Max Mesh Generations Per Frame (try 2-3)
5. Enable Async Generation
6. Check profiler: Stat GPU, Stat Unit
```

### Terrain Looks Blocky

```
❌ Problem: Terrain has visible LOD transitions

✅ Solutions:
1. Decrease LOD Distance Multiplier (try 1.5)
2. Increase Mesh Resolution
3. Enable Smooth Normals
4. Add more LOD levels
5. Use normal maps in material
```

### Seams Between Chunks

```
❌ Problem: Visible gaps between terrain chunks

✅ Solutions:
1. Enable vertex welding
2. Check mesh generation overlaps edges
3. Ensure consistent LOD calculation
4. Verify no floating-point precision issues
```

---

## 📚 Next Steps

### Learn More

- 📖 [Full Documentation](./DOCUMENTATION.md) - In-depth technical details
- 🎯 [API Reference](./API_REFERENCE.md) - Complete API documentation
- ⚡ [Performance Guide](./PERFORMANCE_GUIDE.md) - Optimization strategies
- 📝 [Examples](./Content/Examples/) - Sample maps and setups

### Join Community

- 💬 **Discord**: Share your creations
- 🎮 **Forums**: Get help from community
- 📺 **YouTube**: Video tutorials
- 🐛 **GitHub**: Report issues (if open source)

### Advanced Tutorials

1. **Multi-Biome Open World**
   - Create seamless biome transitions
   - Add weather systems
   - Implement streaming

2. **Procedural Dungeon**
   - Use WFC for room generation
   - Add corridors with pathfinding
   - Spawn loot and enemies

3. **River Network System**
   - Generate realistic river networks
   - Add tributaries
   - Integrate with landscape

4. **Cave Explorer**
   - 3D cave generation
   - Add lighting
   - Place resources and hazards

---

## 🎉 Success!

**Congratulations! You now have a procedural terrain system running.**

### What You've Learned

✅ Install and configure the plugin
✅ Create basic terrain
✅ Configure generation parameters
✅ Apply materials
✅ Use presets
✅ Optimize performance

### Ready for More?

- 🏔️ Try different configurations
- 🌿 Add biomes and foliage
- 💧 Apply erosion for realism
- 🌊 Generate rivers and lakes
- 🕳️ Create cave systems
- 🎨 Customize materials

---

<div align="center">

**Happy Terrain Generation! 🌍**

[⬆ Back to Top](#-quick-start-guide) | [Full Docs](./DOCUMENTATION.md) | [API Reference](./API_REFERENCE.md)

</div>
