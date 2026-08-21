#pragma once

#include "core/Common.hpp"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnstructuredGridReader.h>

namespace winfoam::gui {

class Viewport {
public:
    Viewport();
    ~Viewport();

    Viewport(const Viewport&) = delete;
    Viewport& operator=(const Viewport&) = delete;

    void render();
    void set_case_path(const std::string& path);
    const std::string& case_path() const noexcept { return case_path_; }

    void load_mesh(const std::string& vtk_file);
    void reset_camera();
    void set_representation(int mode); // 0=surface, 1=wireframe, 2=points

private:
    std::string case_path_;
    std::string current_vtk_file_;

    vtkSmartPointer<vtkRenderer> renderer_;
    vtkSmartPointer<vtkRenderWindow> render_window_;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor_;
    vtkSmartPointer<vtkActor> mesh_actor_;
    vtkSmartPointer<vtkPolyDataMapper> mesh_mapper_;

    int representation_mode_ = 0;
    bool show_edges_ = true;
    float background_color_[3] = {0.15f, 0.15f, 0.15f};

    void init_vtk();
    void create_gl_context();
    void render_mesh();
};

} // namespace winfoam::gui