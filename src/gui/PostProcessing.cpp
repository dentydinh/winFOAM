#include "gui/PostProcessing.hpp"
#include "core/CaseParser.hpp"
#include "core/WslBridge.hpp"

#include <imgui.h>
#include <implot.h>
#include <filesystem>
#include <chrono>
#include <fstream>

namespace winfoam::gui {

PostProcessing::PostProcessing() = default;

PostProcessing::~PostProcessing() = default;

void PostProcessing::render() {
    ImGui::Begin("Post-Processing");
    ImGui::BeginTabBar("PostProcessingTabs");

    if (ImGui::BeginTabItem("Residuals")) {
        render_residual_chart();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Log Monitor")) {
        render_log_monitor();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Field Operations")) {
        render_field_operations();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
}

void PostProcessing::set_case_path(const std::string& path) {
    case_path_ = path;
    log_file_path_ = (std::filesystem::path(path) / "log.solver").string();
    parse_log_file();
}

void PostProcessing::render_residual_chart() {
    ImGui::Checkbox("Auto Refresh", &auto_refresh_);
    ImGui::SameLine();
    ImGui::DragFloat("Interval (s)", &refresh_interval_sec_, 0.1f, 0.5f, 30.0f);
    ImGui::SameLine();
    if (ImGui::Button("Refresh Now")) parse_log_file();

    ImGui::Separator();

    static bool log_scale = true;
    ImGui::Checkbox("Log Y Scale", &log_scale);
    // ImPlotAxisFlags_LogScale may not be available in this version
    y_axis_flags_ = log_scale ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_AutoFit;

    if (ImPlot::BeginPlot("Residuals", ImVec2(-1, -1), ImPlotFlags_NoTitle)) {
        ImPlot::SetupAxes("Iteration", "Residual", x_axis_flags_, y_axis_flags_);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, residuals_.iterations.empty() ? 100 : residuals_.iterations.back() * 1.1);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 1e-12, 1e1, ImGuiCond_Once);

        for (const auto& [field, values] : residuals_.residuals) {
            if (!values.empty() && !residuals_.iterations.empty()) {
                ImPlot::PlotLine(field.c_str(), residuals_.iterations.data(), values.data(), values.size());
            }
        }

        if (ImPlot::BeginLegendPopup("Residuals")) {
            for (const auto& [field, values] : residuals_.residuals) {
                ImPlot::PlotLine(field.c_str(), residuals_.iterations.data(), values.data(), values.size());
            }
            ImPlot::EndLegendPopup();
        }
        ImPlot::EndPlot();
    }
}

void PostProcessing::render_log_monitor() {
    ImGui::Text("Log File: %s", log_file_path_.c_str());
    ImGui::Separator();

    static char log_buffer[65536] = "";
    static size_t last_size = 0;

    std::filesystem::path log_path(log_file_path_);
    if (std::filesystem::exists(log_path)) {
        auto fsize = std::filesystem::file_size(log_path);
        if (fsize != last_size || auto_refresh_) {
            std::ifstream ifs(log_path, std::ios::binary | std::ios::ate);
            if (ifs) {
                size_t sz = std::min<size_t>(ifs.tellg(), sizeof(log_buffer) - 1);
                ifs.seekg(0);
                ifs.read(log_buffer, sz);
                log_buffer[sz] = '\0';
                last_size = fsize;
            }
        }
    }

    ImGui::InputTextMultiline("##log", log_buffer, sizeof(log_buffer), ImVec2(-1, -1),
                              ImGuiInputTextFlags_ReadOnly);
}

void PostProcessing::render_field_operations() {
    if (case_path_.empty()) {
        ImGui::TextColored(ImVec4(1,0.5,0,1), "No case selected");
        return;
    }

    if (ImGui::Button("Generate VTK (foamToVTK)")) {
        core::WslBridge bridge;
        auto result = bridge.execute("foamToVTK", case_path_, std::chrono::seconds(120));
    }

    ImGui::SameLine();
    if (ImGui::Button("Sample (postProcess)")) {
        core::WslBridge bridge;
        auto result = bridge.execute("postProcess -func sample -latestTime", case_path_, std::chrono::seconds(120));
    }

    ImGui::Separator();
    ImGui::Text("Available Fields:");
}

void PostProcessing::parse_log_file() {
    if (log_file_path_.empty()) return;
    auto data_opt = core::CaseParser::parse_solver_log(log_file_path_);
    if (data_opt) {
        residuals_ = *data_opt;
    }
}

void PostProcessing::update_residuals() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now.time_since_epoch()).count();
    if (elapsed - last_refresh_time_ > refresh_interval_sec_) {
        last_refresh_time_ = elapsed;
        parse_log_file();
    }
}

} // namespace winfoam::gui