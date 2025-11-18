#!/bin/bash

# Compilation Diagnostic Script for ProceduralMap
# This script attempts to compile the project and captures detailed error logs

echo "======================================"
echo "ProceduralMap Compilation Diagnostic"
echo "======================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Find Unreal Engine installation
echo "1. Finding Unreal Engine installation..."

UE_PATHS=(
    "$HOME/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh"
    "/opt/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh"
    "/usr/local/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh"
    "$HOME/Epic/UE_5.4/Engine/Build/BatchFiles/Linux/Build.sh"
    "/opt/Epic/UE_5.4/Engine/Build/BatchFiles/Linux/Build.sh"
)

UE_BUILD_SCRIPT=""
for path in "${UE_PATHS[@]}"; do
    if [ -f "$path" ]; then
        UE_BUILD_SCRIPT="$path"
        echo -e "${GREEN}✓${NC} Found Unreal Engine at: $path"
        break
    fi
done

if [ -z "$UE_BUILD_SCRIPT" ]; then
    echo -e "${RED}✗${NC} Could not find Unreal Engine Build.sh"
    echo ""
    echo "Please provide the path to your Unreal Engine installation:"
    echo "Example: /path/to/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh"
    echo ""
    echo "Or use Unreal Editor to compile:"
    echo "Right-click ProceduralMap.uproject → Generate Visual Studio Project Files"
    echo "Then open the .sln file and build from your IDE"
    exit 1
fi

# Project details
PROJECT_DIR="/home/user/ProceduralMap"
PROJECT_FILE="$PROJECT_DIR/ProceduralMap.uproject"
LOG_FILE="$PROJECT_DIR/CompilationLog.txt"

echo ""
echo "2. Project information:"
echo "   Project: $PROJECT_FILE"
echo "   Log file: $LOG_FILE"
echo ""

# Check if project file exists
if [ ! -f "$PROJECT_FILE" ]; then
    echo -e "${RED}✗${NC} Project file not found: $PROJECT_FILE"
    exit 1
fi

echo -e "${GREEN}✓${NC} Project file found"
echo ""

# Check Build.cs files
echo "3. Checking Build.cs files..."
if [ -f "$PROJECT_DIR/Source/ProceduralMap/ProceduralMap.Build.cs" ]; then
    echo -e "${GREEN}✓${NC} ProceduralMap.Build.cs exists"
else
    echo -e "${RED}✗${NC} ProceduralMap.Build.cs missing"
fi

if [ -f "$PROJECT_DIR/Source/ProceduralMapEditor/ProceduralMapEditor.Build.cs" ]; then
    echo -e "${GREEN}✓${NC} ProceduralMapEditor.Build.cs exists"
else
    echo -e "${YELLOW}⚠${NC} ProceduralMapEditor.Build.cs missing (optional for runtime)"
fi

echo ""

# Check module files
echo "4. Checking module implementation files..."
if [ -f "$PROJECT_DIR/Source/ProceduralMap/ProceduralMap.h" ]; then
    echo -e "${GREEN}✓${NC} ProceduralMap.h exists"
else
    echo -e "${RED}✗${NC} ProceduralMap.h missing"
fi

if [ -f "$PROJECT_DIR/Source/ProceduralMap/ProceduralMap.cpp" ]; then
    echo -e "${GREEN}✓${NC} ProceduralMap.cpp exists"
else
    echo -e "${RED}✗${NC} ProceduralMap.cpp missing"
fi

echo ""

# Count source files
echo "5. Source file count:"
H_COUNT=$(find "$PROJECT_DIR/Source" -name "*.h" 2>/dev/null | wc -l)
CPP_COUNT=$(find "$PROJECT_DIR/Source" -name "*.cpp" 2>/dev/null | wc -l)
echo "   Header files (.h): $H_COUNT"
echo "   Source files (.cpp): $CPP_COUNT"
echo ""

# Attempt compilation
echo "6. Attempting compilation..."
echo "   This may take 5-15 minutes..."
echo "   Output will be saved to: $LOG_FILE"
echo ""

# Clean previous builds
echo "Cleaning previous build artifacts..."
rm -rf "$PROJECT_DIR/Intermediate" 2>/dev/null
rm -rf "$PROJECT_DIR/Binaries" 2>/dev/null
rm -rf "$PROJECT_DIR/Saved" 2>/dev/null

# Try to build
echo "Starting build..."
"$UE_BUILD_SCRIPT" ProceduralMapEditor Linux Development "$PROJECT_FILE" -waitmutex > "$LOG_FILE" 2>&1

BUILD_RESULT=$?

echo ""
echo "======================================"
echo "Build Result"
echo "======================================"
echo ""

if [ $BUILD_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ BUILD SUCCESSFUL!${NC}"
    echo ""
    echo "The project compiled successfully."
    echo "You can now open ProceduralMap.uproject in Unreal Editor."
else
    echo -e "${RED}✗ BUILD FAILED${NC}"
    echo ""
    echo "Compilation errors detected. Analyzing log..."
    echo ""

    # Extract key errors
    echo "===== COMPILATION ERRORS ====="
    grep -i "error" "$LOG_FILE" | head -20
    echo ""

    echo "===== COMMON ISSUES ====="

    # Check for specific error patterns
    if grep -q "undefined reference" "$LOG_FILE"; then
        echo -e "${YELLOW}⚠${NC} Linker errors detected (undefined references)"
        echo "   Possible causes:"
        echo "   - Missing module dependencies in Build.cs"
        echo "   - Missing function implementations"
    fi

    if grep -q "no matching function" "$LOG_FILE"; then
        echo -e "${YELLOW}⚠${NC} Function signature mismatch detected"
        echo "   Possible causes:"
        echo "   - API changes between UE versions"
        echo "   - Incorrect parameter types"
    fi

    if grep -q "cannot open source file" "$LOG_FILE"; then
        echo -e "${YELLOW}⚠${NC} Missing include files detected"
        echo "   Possible causes:"
        echo "   - Missing module in Build.cs"
        echo "   - Incorrect include path"
    fi

    echo ""
    echo "Full log available at: $LOG_FILE"
    echo ""
    echo "For detailed analysis, run:"
    echo "  less $LOG_FILE"
fi

echo ""
echo "======================================"
echo "Next Steps"
echo "======================================"
echo ""

if [ $BUILD_RESULT -eq 0 ]; then
    echo "1. Double-click ProceduralMap.uproject to open in Unreal Editor"
    echo "2. The project is ready to use!"
else
    echo "1. Review the compilation log: $LOG_FILE"
    echo "2. Share the error messages for assistance"
    echo "3. Consider using a minimal configuration if issues persist"
fi

echo ""
