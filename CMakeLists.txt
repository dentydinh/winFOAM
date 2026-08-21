cmake_minimum_required(VERSION 3.20)
project(winFOAM VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Vendor paths
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)
set(IMGUI_DIR ${VENDOR_DIR}/imgui)
set(IMPLOT_DIR ${VENDOR_DIR}/implot)
set(GLFW_DIR ${VENDOR_DIR}/glfw)
set(STB_DIR ${VENDOR_DIR}/stb)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${IMGUI_DIR})
include_directories(${IMPLOT_DIR})
include_directories(${GLFW_DIR}/include)
include_directories(${STB_DIR})

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive- /MP)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -fPIC)
endif()

# Find packages
find_package(OpenGL REQUIRED)

# VTK 9+ uses new component names (without vtk prefix)
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    CommonExecutionModel
    CommonMath
    CommonTransforms
    FiltersCore
    FiltersGeneral
    FiltersGeometry
    IOImage
    IOLegacy
    IOXML
    ImagingCore
    InteractionStyle
    RenderingCore
    RenderingOpenGL2
    RenderingFreeType
)

# ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImPlot source files
set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
    ${IMPLOT_DIR}/implot_demo.cpp
)

# winFOAM core sources
set(CORE_SOURCES
    src/core/ThreadPool.cpp
    src/core/WslBridge.cpp
    src/core/CaseExporter.cpp
    src/core/CaseParser.cpp
)

# winFOAM GUI sources
set(GUI_SOURCES
    src/gui/Application.cpp
    src/gui/Workbench.cpp
    src/gui/PreProcessing.cpp
    src/gui/Solutions.cpp
    src/gui/PostProcessing.cpp
    src/gui/Viewport.cpp
)

# Main executable
add_executable(winFOAM
    src/main.cpp
    ${CORE_SOURCES}
    ${GUI_SOURCES}
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
)

target_link_libraries(winFOAM PRIVATE
    ${OPENGL_LIBRARIES}
    ${VTK_LIBRARIES}
    glfw
)

# GLFW as subdirectory
add_subdirectory(${GLFW_DIR} EXCLUDE_FROM_ALL)

# Install
install(TARGETS winFOAM DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/winFOAM/assets)
install(DIRECTORY resources/ DESTINATION share/winFOAM/resources)

# Enable testing (only if tests directory exists)
if(EXISTS ${CMAKE_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()