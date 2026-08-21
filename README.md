# winFOAM

**Windows-native OpenFOAM GUI** — A modern, ImGui-based desktop application for Computational Fluid Dynamics (CFD) pre-processing, solving, and post-processing on Windows with WSL2 backend.

## Features

- **Native Windows UI** — Built with Dear ImGui + ImPlot for responsive, GPU-accelerated interface
- **WSL2 Integration** — Seamless execution of OpenFOAM solvers (`blockMesh`, `simpleFoam`, `snappyHexMesh`, etc.) via WSL
- **Case Management** — Visual setup of `controlDict`, `fvSchemes`, `fvSolution`, boundary conditions
- **Live Residual Plotting** — Real-time convergence monitoring with ImPlot
- **3D Mesh Viewport** — VTK-powered mesh visualization and inspection
- **Dictionary Templates** — Built-in OpenFOAM dictionary templates for rapid case setup

## Architecture

```
winFOAM/
├── include/core/      # Backend: WSL bridge, case I/O, threading
├── include/gui/       # Frontend: ImGui panels, viewport, charts
├── src/               # Implementation
├── vendor/            # ImGui, ImPlot, GLFW, stb (git submodules)
├── assets/            # Fonts, icons, themes
├── resources/         # OpenFOAM dictionary templates
├── scripts/           # WSL setup & utility wrappers
└── docs/              # Technical documentation
```

## Requirements

- **Windows 10/11** with **WSL2** (Ubuntu 22.04+ recommended)
- **OpenFOAM v10+** installed inside WSL
- **CMake 3.20+**, **Ninja**, **C++20 compiler** (MSVC 19.35+ / GCC 11+ / Clang 14+)
- **VTK 9+** (for 3D viewport)
- **GPU** with OpenGL 3.3+ support

## Building

```bash
# Clone with submodules
git clone --recursive https://github.com/yourorg/winFOAM.git
cd winFOAM

# Configure (Debug)
cmake --preset debug

# Build
cmake --build --preset build-debug

# Run
./build/debug/bin/winFOAM.exe
```

### Using CMake Presets

| Preset | Description |
|--------|-------------|
| `debug` | Debug with ASan/UBSan |
| `release` | Optimized `-O3` |
| `relwithdebinfo` | Optimized with debug symbols |

## First Run Setup

1. **Install WSL2 & Ubuntu**: `wsl --install -d Ubuntu-22.04`
2. **Install OpenFOAM in WSL**:
   ```bash
   # Inside WSL
   sudo apt update && sudo apt install -y openfoam10
   echo "source /opt/openfoam10/etc/bashrc" >> ~/.bashrc
   source ~/.bashrc
   ```
3. **Configure winFOAM**: On first launch, set WSL distro name and OpenFOAM version in *Settings → Backend*.

## Project Structure

| Directory | Purpose |
|-----------|---------|
| `include/core/` | Backend headers (ThreadPool, WslBridge, CaseExporter, CaseParser) |
| `include/gui/` | Frontend headers (Application, Workbench, PreProcessing, Solutions, PostProcessing, Viewport) |
| `src/` | `.cpp` implementations matching headers |
| `vendor/` | Git submodules: imgui, implot, glfw, stb |
| `assets/themes/` | ImGui style presets (Dark, Light, Dracula, etc.) |
| `resources/templates/` | OpenFOAM dictionary templates |
| `scripts/` | `setup_env.bat` (Windows), `wsl_deploy.sh` (WSL), `foam_utils.sh` (solver wrappers) |

## Key Classes

| Class | File | Responsibility |
|-------|------|----------------|
| `ThreadPool` | `core/ThreadPool.hpp` | Async task queue for long-running WSL commands |
| `WslBridge` | `core/WslBridge.hpp` | Execute commands in WSL, capture stdout/stderr |
| `CaseExporter` | `core/CaseExporter.hpp` | Write OpenFOAM dictionaries from UI state |
| `CaseParser` | `core/CaseParser.hpp` | Parse log files, extract residuals, mesh stats |
| `Application` | `gui/Application.hpp` | Main loop, GLFW/ImGui initialization |
| `Workbench` | `gui/Workbench.hpp` | Dockspace, menu bar, panel layout |
| `PreProcessing` | `gui/PreProcessing.hpp` | Mesh generation, boundary condition panels |
| `Solutions` | `gui/Solutions.hpp` | Solver selection, fvSchemes/fvSolution editors |
| `PostProcessing` | `gui/PostProcessing.hpp` | Residual charts (ImPlot), surface/volume data |
| `Viewport` | `gui/Viewport.hpp` | VTK 3D mesh rendering widget |

## License

MIT License — see [LICENSE](LICENSE) for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes with clang-format compliance
4. Run tests: `cmake --build --preset build-debug --target test`
5. Submit PR

## Roadmap

- [ ] snappyHexMesh visual configuration
- [ ] Parallel run monitoring (decomposePar / reconstructPar)
- [ ] Python scripting API for automation
- [ ] Cloud HPC integration (Azure/AWS batch)
- [ ] Plugin system for custom solvers