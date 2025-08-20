# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

CarnivoresRenderer is a C++/OpenGL 3 renderer for Carnivores 1, 2, and Ice Age game files. It's a 3D game engine that loads classic Carnivores map and character files (.map, .rsc, .car) and renders them using modern OpenGL.

## Build Commands

This project uses CMake for build configuration:

### macOS Build
```bash
# Install dependencies
brew install glew glfw3

# Generate Xcode project
cmake -G "Xcode" -B build

# Build
cmake --build build
```

### Windows Build  
```bash
# Generate Visual Studio project (adjust VS version as needed)
mkdir build
cd build
cmake -G "Visual Studio 15" ..

# Build in Visual Studio or via command line
cmake --build . --config Release
```

### General Build
```bash
cmake -B build
cmake --build build
```

## Key Architecture Components

### Core Systems
- **LocalVideoManager**: Handles OpenGL rendering and window management
- **LocalAudioManager**: Manages OpenAL audio playback
- **LocalInputManager**: Processes keyboard/mouse input
- **TerrainRenderer**: Renders game terrain from .map/.rsc files

### Game Entity Controllers  
- **CELocalPlayerController**: Manages the local player's state and behavior
- **CERemotePlayerController**: Handles remote/AI character entities
- **CEAIGenericAmbientManager**: Manages ambient AI behavior systems

### File Format Loaders
- **C2MapFile/C2MapRscFile**: Loads Carnivores map and resource files
- **C2CarFile/C2CarFilePreloader**: Loads character/dinosaur models and animations
- **CEAnimation**: Handles character animation playback
- **CEGeometry/CETexture**: Manages 3D meshes and texture loading

### Rendering Components
- **CEShaderProgram**: OpenGL shader management
- **CEWorldModel**: 3D model representation and rendering
- **C2Sky**: Sky rendering system
- **CEShadowManager**: Static shadow mapping system for map objects

## Configuration

The runtime is configured via `runtime/config.json`:
- Set `basePath` to your runtime assets directory
- Configure `map.type` ("C1" or "C2") and map/rsc file paths  
- Add `spawns` array for placing characters with animations

Example:
```json
{
  "basePath": "runtime/cce/",
  "map": {
    "type": "C1", 
    "map": "game/c1/area2.map",
    "rsc": "game/c1/area2.rsc"
  },
  "spawns": [
    {
      "file": "runtime/cce/tirex.car",
      "animation": "Tx_see2", 
      "position": [200, 232]
    }
  ]
}
```

## Important Directories

- `src/`: C++ source code
- `runtime/`: Game assets and configuration
- `include/`: External library headers (OpenGL, GLFW, OpenAL, GLM)
- `lib/`: Static libraries for linking
- `cmake/`: CMake find modules for dependencies
- `shaders/`: GLSL shader files (in runtime/cce/shaders/)

## Dependencies

- **OpenGL 3+**: 3D graphics rendering
- **GLFW**: Window/input management  
- **OpenAL**: Audio playback
- **GLM**: Mathematics library for graphics
- **nlohmann/json**: JSON parsing (fetched via CMake)
- **GLAD**: OpenGL function loading

## Shadow System

The renderer includes a static shadow mapping system for map objects:

- **Shadow Generation**: Automatically generates 2048x2048 shadow maps at startup
- **Object Filtering**: Only objects with `objectDEFLIGHT` or `objectGRNDLIGHT` flags cast shadows
- **Caching**: Shadow maps are cached to `runtime/cache/shadows/` and reused on subsequent loads
- **Soft Shadows**: Uses PCF (Percentage-Closer Filtering) for realistic shadow edges
- **Performance**: Generated once per map and reused throughout the session
- remember how to build on macos: cmake --build build --target CarnivoresRenderer