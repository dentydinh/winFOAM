#include "gui/Viewport.hpp"

#include <imgui.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>

namespace winfoam::gui {

Viewport::Viewport() {
    init_vtk();
}

Viewport::~Viewport() = default;

void Viewport::init_vtk() {
    renderer_ = vtkSmartPointer<vtkRenderer>::New();
    render_window_ = vtkSmartPointer<vtkRenderWindow>::New();
    render_window_->AddRenderer(renderer_);
    interactor_ = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor_->SetRenderWindow(render_window_);

    mesh_mapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();
    mesh_actor_ = vtkSmartPointer<vtkActor>::New();
    mesh_actor_->SetMapper(mesh_mapper_);
    mesh_actor_->GetProperty()->SetEdgeVisibility(true);
    mesh_actor_->GetProperty()->SetEdgeColor(0.2, 0.2, 0.2);
    mesh_actor_->GetProperty()->SetColor(0.6, 0.7, 0.9);
    mesh_actor_->GetProperty()->SetOpacity(1.0);

    renderer_->AddActor(mesh_actor_);
    renderer_->SetBackground(background_color_[0], background_color_[1], background_color_[2]);
    renderer_->ResetCamera();
}

void Viewport::create_gl_context() {
    // VTK will use its own OpenGL context; for embedding in ImGui we'd need vtkExternalOpenGLCoreRenderWindow
    // This is a placeholder for proper VTK-ImGui integration
}

void Viewport::render() {
    ImGui::Begin("3D Viewport");

    ImGui::Text("Case: %s", case_path_.empty() ? "Not set" : case_path_.c_str());
    ImGui::Separator();

    const char* reps[] = {"Surface", "Wireframe", "Surface with Edges", "Points"};
    if (ImGui::Combo("Representation", &representation_mode_, reps, IM_ARRAYSIZE(reps))) {
        set_representation(representation_mode_);
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset Camera")) reset_camera();

    ImGui::SameLine();
    ImGui::ColorEdit3("Background", background_color_);
    renderer_->SetBackground(background_color_[0], background_color_[1], background_color_[2]);

    ImGui::Separator();

    // Render VTK into ImGui via texture (requires vtkExternalOpenGLCoreRenderWindow or offscreen rendering)
    // Placeholder: show VTK render window info
    ImGui::Text("VTK Render Window: %p", static_cast<void*>(render_window_.GetPointer()));
    ImGui::Text("Renderer: %p", static_cast<void*>(renderer_.GetPointer()));
    ImGui::Text("Actors: %d", renderer_->GetActors()->GetNumberOfItems());

    if (ImGui::Button("Load Mesh (VTK)")) {
        // TODO: file dialog for .vtk/.vtu
    }

    ImGui::End();
}

void Viewport::set_case_path(const std::string& path) {
    case_path_ = path;
}

void Viewport::load_mesh(const std::string& vtk_file) {
    current_vtk_file_ = vtk_file;
    vtkSmartPointer<vtkUnstructuredGridReader> reader = vtkSmartPointer<vtkUnstructuredGridReader>::New();
    reader->SetFileName(vtk_file.c_str());
    reader->Update();

    vtkSmartPointer<vtkDataSetSurfaceFilter> surface = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surface->SetInputConnection(reader->GetOutputPort());
    surface->Update();

    mesh_mapper_->SetInputConnection(surface->GetOutputPort());
    renderer_->ResetCamera();
    render_window_->Render();
}

void Viewport::reset_camera() {
    renderer_->ResetCamera();
    render_window_->Render();
}

void Viewport::set_representation(int mode) {
    representation_mode_ = mode;
    switch (mode) {
        case 0: // Surface
            mesh_actor_->GetProperty()->SetRepresentationToSurface();
            mesh_actor_->GetProperty()->SetEdgeVisibility(false);
            break;
        case 1: // Wireframe
            mesh_actor_->GetProperty()->SetRepresentationToWireframe();
            break;
        case 2: // Surface with Edges
            mesh_actor_->GetProperty()->SetRepresentationToSurface();
            mesh_actor_->GetProperty()->SetEdgeVisibility(true);
            break;
        case 3: // Points
            mesh_actor_->GetProperty()->SetRepresentationToPoints();
            mesh_actor_->GetProperty()->SetPointSize(3);
            break;
    }
    render_window_->Render();
}

} // namespace winfoam::gui