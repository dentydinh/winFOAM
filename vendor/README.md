# Vendor Libraries

This directory contains third-party libraries as Git submodules.

## Initialization

```bash
git submodule update --init --recursive
```

## Required Submodules

| Library | Repository | Version/Tag | Path |
|---------|------------|-------------|------|
| Dear ImGui | https://github.com/ocornut/imgui | docking branch | `vendor/imgui` |
| ImPlot | https://github.com/epezent/implot | latest | `vendor/implot` |
| GLFW | https://github.com/glfw/glfw | 3.4+ | `vendor/glfw` |
| stb | https://github.com/nothings/stb | master | `vendor/stb` |

## Manual Setup (if not using submodules)

### ImGui (docking branch)
```bash
cd vendor
git clone -b docking https://github.com/ocornut/imgui.git
```

### ImPlot
```bash
cd vendor
git clone https://github.com/epezent/implot.git
```

### GLFW
```bash
cd vendor
git clone https://github.com/glfw/glfw.git
cd glfw
git checkout 3.4
```

### stb
```bash
cd vendor
git clone https://github.com/nothings/stb.git
```

## Directory Structure After Init

```
vendor/
├── imgui/
│   ├── imgui.h
│   ├── imgui.cpp
│   ├── imgui_draw.cpp
│   ├── imgui_tables.cpp
│   ├── imgui_widgets.cpp
│   ├── backends/
│   │   ├── imgui_impl_glfw.cpp
│   │   └── imgui_impl_opengl3.cpp
│   └── ...
├── implot/
│   ├── implot.h
│   ├── implot.cpp
│   ├── implot_items.cpp
│   └── ...
├── glfw/
│   ├── include/GLFW/glfw3.h
│   ├── src/
│   ├── CMakeLists.txt
│   └── ...
└── stb/
    ├── stb_image.h
    ├── stb_image_write.h
    └── ...
```

## CMake Integration

The main `CMakeLists.txt` includes these as source files directly (not as separate targets) for simplicity. GLFW is added as a subdirectory since it has its own CMake build.

## Licenses

- **ImGui**: MIT License
- **ImPlot**: MIT License
- **GLFW**: zlib/libpng License
- **stb**: Public Domain / MIT License