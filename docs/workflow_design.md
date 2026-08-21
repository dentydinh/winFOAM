# winFOAM Workflow Design Document

## Overview

winFOAM is a native Windows GUI application for OpenFOAM CFD workflows. It uses **Dear ImGui** for the UI, **WSL2** as the OpenFOAM execution backend, and **VTK** for 3D visualization.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      winFOAM Application                     │
├─────────────────────────────────────────────────────────────┤
│  GUI Layer (ImGui)              │  Core Layer (C++)         │
│  ┌─────────────────────────┐    │  ┌─────────────────────┐  │
│  │ Workbench (Dockspace)   │    │  │ ThreadPool          │  │
│  │ ├─ PreProcessing        │    │  │ WslBridge           │  │
│  │ ├─ Solutions            │───▶│  │ CaseExporter        │  │
│  │ ├─ PostProcessing       │    │  │ CaseParser          │  │
│  │ └─ Viewport (VTK)       │    │  └─────────────────────┘  │
│  └─────────────────────────┘    └───────────────────────────┘
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      WSL2 Ubuntu                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ OpenFOAM v10+                                        │   │
│  │ blockMesh, snappyHexMesh, simpleFoam, etc.          │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. ThreadPool (`core/ThreadPool.hpp`)
- **Purpose**: Async execution of long-running WSL commands
- **Design**: Fixed-size thread pool with task queue
- **API**: `enqueue(F&&, Args&&...) -> std::future<Result>`
- **Usage**: All WSL commands run async to keep UI responsive

### 2. WslBridge (`core/WslBridge.hpp`)
- **Purpose**: Execute commands in WSL, capture stdout/stderr/exit code
- **Key Methods**:
  - `execute(cmd, cwd, timeout)` - Synchronous with timeout
  - `execute_async(cmd, cwd)` - Fire-and-forget
  - `is_wsl_available()` - Health check
  - `list_distros()` - Discover WSL distributions
- **Implementation**: Uses `wsl.exe -d <distro> -- <cmd>` via `_popen`
- **Timeout**: Implemented via `std::future::wait_for`

### 3. CaseExporter (`core/CaseExporter.hpp`)
- **Purpose**: Write OpenFOAM dictionary files from UI state
- **Output Files**:
  - `system/controlDict` - Solver, time controls
  - `system/fvSchemes` - Discretization schemes
  - `system/fvSolution` - Linear solvers, SIMPLE controls
  - `0/<field>` - Boundary conditions
  - `constant/transportProperties`, `turbulenceProperties`
- **Format**: OpenFOAM dictionary syntax with proper nesting

### 4. CaseParser (`core/CaseParser.hpp`)
- **Purpose**: Parse OpenFOAM dictionaries and log files
- **Parsers**:
  - `parse_dict_file()` - Generic dictionary parser (recursive descent)
  - `parse_control_dict()` - Extract solver settings
  - `parse_boundary_file()` - Extract BCs from 0/ files
  - `parse_check_mesh()` - Mesh statistics
  - `parse_solver_log()` - Residual history for plotting

## GUI Components

### Application (`gui/Application.hpp`)
- **Role**: Main entry point, GLFW/ImGui lifecycle
- **Responsibilities**:
  - GLFW window creation (OpenGL 3.3 core)
  - ImGui context initialization
  - Main loop: events → new frame → render → swap
  - Owns all panel instances

### Workbench (`gui/Workbench.hpp`)
- **Role**: Application shell - menu bar, dockspace, status bar
- **Features**:
  - Persistent dockspace layout (imgui.ini)
  - Theme switching (Dark/Light/Dracula/Classic)
  - Menu bar: File, Edit, View, Tools, Help
  - About dialog

### PreProcessing (`gui/PreProcessing.hpp`)
- **Tabs**:
  1. **Mesh Generation**: Tool selector (blockMesh/snappyHexMesh), run buttons
  2. **Boundary Conditions**: Patch table with inline editing
  3. **blockMeshDict Editor**: Syntax-highlighted text editor
  4. **snappyHexMesh**: Configuration panel (TODO)
- **Workflow**: Load case → Edit blockMeshDict → Run blockMesh → Edit BCs

### Solutions (`gui/Solutions.hpp`)
- **Tabs**:
  1. **Solver Setup**: Solver combo, time controls, parallel options, Run/Stop buttons
  2. **fvSchemes**: Text editor for discretization schemes
  3. **fvSolution**: Text editor for linear solvers
  4. **Physical Properties**: Transport & turbulence model config
- **Workflow**: Select solver → Configure time/parallel → Edit schemes/solution → Run

### PostProcessing (`gui/PostProcessing.hpp`)
- **Tabs**:
  1. **Residuals**: ImPlot chart with log-scale Y, auto-refresh
  2. **Log Monitor**: Tail -f style log viewer with filtering
  3. **Field Operations**: foamToVTK, sample, probe buttons
- **Data Flow**: Parse log → Update ResidualData → Plot

### Viewport (`gui/Viewport.hpp`)
- **Role**: Embedded VTK 3D view
- **Current State**: Basic VTK pipeline setup (placeholder for ImGui embedding)
- **Future**: `vtkExternalOpenGLCoreRenderWindow` for zero-copy ImGui texture sharing
- **Features**: Representation modes, camera reset, background color

## Data Flow

```
User Action (UI)
       │
       ▼
┌──────────────────┐
│  Panel Handler   │  (e.g., Solutions::run_solver())
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  CaseExporter    │  Write dictionaries from UI state
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  WslBridge       │  Execute in WSL
│  .execute()      │  Async via ThreadPool
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  WSL / OpenFOAM  │  blockMesh, simpleFoam, etc.
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  CaseParser      │  Parse log files
│  .parse_log()    │  Update ResidualData
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  PostProcessing  │  Plot residuals (ImPlot)
│  .render()       │  Show log tail
└──────────────────┘
```

## Threading Model

```
Main Thread (UI)          Worker Threads (ThreadPool)
┌─────────────────┐       ┌─────────────────────────┐
│ ImGui Frame     │       │ WslBridge::execute()    │
│ 1. Process UI   │──────▶│ 1. Build wsl command    │
│ 2. Enqueue task │       │ 2. _popen()             │
│ 3. Check futures│       │ 3. Read stdout/stderr   │
│ 4. Update state │       │ 4. Return CommandResult │
└─────────────────┘       └─────────────────────────┘
       │                         │
       │   std::future           │
       └─────────────────────────┘
```

- UI never blocks on WSL commands
- Results polled each frame via `future.wait_for(0s)`
- Long output streamed to log panel incrementally

## Configuration

### WSL Config (`WslConfig`)
```cpp
struct WslConfig {
    std::string distro_name = "Ubuntu-22.04";
    std::string openfoam_version = "10";
    std::string openfoam_bashrc = "/opt/openfoam10/etc/bashrc";
    std::chrono::seconds default_timeout{300};
};
```

### App Settings (`AppSettings`)
```cpp
struct AppSettings {
    Theme theme = Theme::Dark;
    bool auto_save = true;
    int auto_save_interval_sec = 30;
    std::string last_case_path;
    float font_scale = 1.0f;
};
```

## Build System

### CMake Targets
- `winFOAM` - Main executable
- `vendor/glfw` - Static library (subdirectory)

### Dependencies
| Library | Version | Purpose |
|---------|---------|---------|
| ImGui | docking branch | UI |
| ImPlot | latest | Residual charts |
| GLFW | 3.3+ | Window/OpenGL context |
| VTK | 9+ | 3D viewport |
| OpenGL | 3.3+ | Rendering |
| C++ | 20 | Language standard |

### Presets
| Preset | CMAKE_BUILD_TYPE | Flags |
|--------|------------------|-------|
| debug | Debug | `-g -O0 -fsanitize=address,undefined` |
| release | Release | `-O3 -DNDEBUG` |
| relwithdebinfo | RelWithDebInfo | `-O2 -g` |

## File Structure Conventions

```
case/
├── 0/                    # Initial conditions
│   ├── U                 # Velocity field + BCs
│   ├── p                 # Pressure field + BCs
│   ├── k                 # Turbulence k + BCs
│   └── epsilon           # Turbulence epsilon + BCs
├── constant/
│   ├── transportProperties
│   ├── turbulenceProperties
│   └── polyMesh/         # Generated mesh
│       ├── points
│       ├── faces
│       ├── owner
│       ├── neighbour
│       └── boundary
├── system/
│   ├── controlDict
│   ├── fvSchemes
│   ├── fvSolution
│   ├── blockMeshDict
│   └── snappyHexMeshDict
└── log.*                 # Solver logs (generated)
```

## Error Handling Strategy

| Layer | Strategy |
|-------|----------|
| WslBridge | Return `CommandResult` with exit_code, stdout, stderr, timeout flag |
| CaseExporter | Return `bool`, log errors to stderr |
| CaseParser | Return `std::optional<T>`, empty on parse failure |
| UI | Show toast notifications, log panel for command output |

## Future Extensions

1. **Python Scripting**: Embed Python for automation
2. **Plugin System**: Dynamic loading of custom panels/solvers
3. **Cloud HPC**: Azure Batch / AWS ParallelCluster integration
4. **Remote WSL**: SSH to Linux HPC instead of local WSL
5. **Live VTK**: Zero-copy texture sharing for 60fps viewport
6. **Case Templates**: Wizard for common cases (airfoil, pipe, room)

## Testing Strategy

- **Unit Tests**: Core parsers, exporters (GoogleTest)
- **Integration Tests**: WslBridge with mock WSL
- **UI Tests**: ImGui test engine (manual)
- **CI**: GitHub Actions with Windows + WSL runners