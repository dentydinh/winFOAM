#include "gui/Solutions.hpp"
#include "core/WslBridge.hpp"
#include "core/CaseExporter.hpp"
#include "core/CaseParser.hpp"

#include <imgui.h>
#include <fstream>
#include <filesystem>

namespace winfoam::gui {

Solutions::Solutions() = default;

Solutions::~Solutions() = default;

void Solutions::render() {
    ImGui::Begin("Solutions");
    ImGui::BeginTabBar("SolutionsTabs");

    if (ImGui::BeginTabItem("Solver Setup")) {
        render_solver_selection();
        render_time_controls();
        render_parallel_controls();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Discretization (fvSchemes)")) {
        render_discretization_schemes();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Solution Controls (fvSolution)")) {
        render_solution_controls();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Physical Properties")) {
        render_transport_properties();
        render_turbulence_properties();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
}

void Solutions::set_case_path(const std::string& path) {
    case_path_ = path;
    load_case_dicts();
}

void Solutions::render_solver_selection() {
    ImGui::Text("Case: %s", case_path_.empty() ? "Not set" : case_path_.c_str());
    ImGui::Separator();

    const char* solvers[] = {
        "simpleFoam", "pimpleFoam", "pisoFoam", "icoFoam",
        "rhoSimpleFoam", "rhoPimpleFoam", "buoyantSimpleFoam",
        "buoyantPimpleFoam", "chtMultiRegionFoam", "Custom..."
    };
    int current = 0;
    for (int i = 0; i < IM_ARRAYSIZE(solvers); ++i) {
        if (solver_settings_.solver == solvers[i]) { current = i; break; }
    }
    if (ImGui::Combo("Solver", &current, solvers, IM_ARRAYSIZE(solvers))) {
        solver_settings_.solver = solvers[current];
    }

    ImGui::Separator();
    ImGui::Text("Execution:");
    if (ImGui::Button("Run Solver")) run_solver();
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        // TODO: kill process
    }
    ImGui::SameLine();
    if (ImGui::Button("Decompose")) run_decompose_par();
    ImGui::SameLine();
    if (ImGui::Button("Reconstruct")) run_reconstruct_par();
}

void Solutions::render_time_controls() {
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Time Control", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputDouble("Start Time", &solver_settings_.start_time, 0.1, 1.0, "%.3f");
        ImGui::InputDouble("End Time", &solver_settings_.end_time, 1.0, 10.0, "%.3f");
        ImGui::InputDouble("Delta T", &solver_settings_.delta_t, 0.01, 0.1, "%.4f");
        ImGui::InputDouble("Write Interval", &solver_settings_.write_interval, 1.0, 10.0, "%.3f");

        const char* write_control[] = {"timeStep", "runTime", "adjustableRunTime", "cpuTime"};
        int wc = 0;
        if (ImGui::Combo("Write Control", &wc, write_control, IM_ARRAYSIZE(write_control))) {}
    }
}

void Solutions::render_discretization_schemes() {
    if (ImGui::Button("Load fvSchemes")) load_case_dicts();
    ImGui::SameLine();
    if (ImGui::Button("Save fvSchemes")) save_case_dicts();

    ImGui::Separator();
    static char fv_schemes_text[16384] = "";
    ImGui::InputTextMultiline("##fvSchemes", fv_schemes_text, sizeof(fv_schemes_text),
                              ImVec2(-1, -1), ImGuiInputTextFlags_AllowTabInput);
}

void Solutions::render_solution_controls() {
    ImGui::Separator();
    static char fv_solution_text[16384] = "";
    ImGui::InputTextMultiline("##fvSolution", fv_solution_text, sizeof(fv_solution_text),
                              ImVec2(-1, -1), ImGuiInputTextFlags_AllowTabInput);
}

void Solutions::render_transport_properties() {
    if (ImGui::CollapsingHeader("Transport Properties")) {
        ImGui::Text("nu (kinematic viscosity):");
        static double nu = 1.5e-5;
        ImGui::InputDouble("##nu", &nu, 0.00001, 0.0001, "%.6f");
    }
}

void Solutions::render_turbulence_properties() {
    if (ImGui::CollapsingHeader("Turbulence Properties")) {
        const char* models[] = {"kEpsilon", "kOmegaSST", "realizableKE", "RNGkEpsilon", "SpalartAllmaras", "Laminar"};
        static int model = 0;
        ImGui::Combo("Turbulence Model", &model, models, IM_ARRAYSIZE(models));
    }
}

void Solutions::render_parallel_controls() {
    if (ImGui::CollapsingHeader("Parallel Execution")) {
        ImGui::Checkbox("Run in Parallel", &solver_settings_.run_parallel);
        if (solver_settings_.run_parallel) {
            ImGui::InputInt("Processors", &solver_settings_.n_processors, 1, 4);
            const char* methods[] = {"scotch", "hierarchical", "simple", "metis", "manual"};
            int method = 0;
            ImGui::Combo("Decomposition Method", &method, methods, IM_ARRAYSIZE(methods));
            solver_settings_.decompose_method = methods[method];
        }
    }
}

void Solutions::load_case_dicts() {
    if (case_path_.empty()) return;

    std::filesystem::path case_path(case_path_);
    auto control_opt = core::CaseParser::parse_control_dict(case_path / "system" / "controlDict");
    if (control_opt) solver_settings_ = *control_opt;

    auto schemes_opt = core::CaseParser::parse_dict_file(case_path / "system" / "fvSchemes");
    if (schemes_opt) fv_schemes_ = *schemes_opt;

    auto solution_opt = core::CaseParser::parse_dict_file(case_path / "system" / "fvSolution");
    if (solution_opt) fv_solution_ = *solution_opt;
}

bool Solutions::save_case_dicts() {
    if (case_path_.empty()) return false;

    bool ok = true;
    ok &= core::CaseExporter::write_control_dict(case_path_, solver_settings_);
    ok &= core::CaseExporter::write_fv_schemes(case_path_, fv_schemes_);
    ok &= core::CaseExporter::write_fv_solution(case_path_, fv_solution_);
    return ok;
}

void Solutions::run_solver() {
    if (case_path_.empty()) return;
    core::WslBridge bridge;
    std::string cmd = solver_settings_.run_parallel ?
        "mpirun -np " + std::to_string(solver_settings_.n_processors) + " " + solver_settings_.solver + " -parallel" :
        solver_settings_.solver;
    auto result = bridge.execute(cmd, case_path_, std::chrono::seconds(3600));
}

void Solutions::run_decompose_par() {
    if (case_path_.empty()) return;
    core::WslBridge bridge;
    auto result = bridge.execute("decomposePar -force", case_path_, std::chrono::seconds(120));
}

void Solutions::run_reconstruct_par() {
    if (case_path_.empty()) return;
    core::WslBridge bridge;
    auto result = bridge.execute("reconstructPar -latestTime", case_path_, std::chrono::seconds(120));
}

} // namespace winfoam::gui