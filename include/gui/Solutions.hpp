#pragma once

#include "core/Common.hpp"
#include "core/CaseExporter.hpp"

namespace winfoam::gui {

class Solutions {
public:
    Solutions();
    ~Solutions();

    void render();
    void set_case_path(const std::string& path);
    const std::string& case_path() const noexcept { return case_path_; }
    const core::SolverSettings& solver_settings() const noexcept { return solver_settings_; }

private:
    std::string case_path_;
    core::SolverSettings solver_settings_;
    core::Dictionary fv_schemes_;
    core::Dictionary fv_solution_;
    bool show_fv_schemes_editor_ = false;
    bool show_fv_solution_editor_ = false;
    bool show_transport_properties_ = false;
    bool show_turbulence_properties_ = false;

    void render_solver_selection();
    void render_time_controls();
    void render_discretization_schemes();
    void render_solution_controls();
    void render_transport_properties();
    void render_turbulence_properties();
    void render_parallel_controls();
    void load_case_dicts();
    bool save_case_dicts();
    void run_solver();
    void run_decompose_par();
    void run_reconstruct_par();
};

} // namespace winfoam::gui