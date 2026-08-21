#pragma once

#include "core/Common.hpp"
#include "core/CaseParser.hpp"

#include <implot.h>

namespace winfoam::gui {

class PostProcessing {
public:
    PostProcessing();
    ~PostProcessing();

    void render();
    void set_case_path(const std::string& path);
    const std::string& case_path() const noexcept { return case_path_; }
    const core::ResidualData& residuals() const noexcept { return residuals_; }

private:
    std::string case_path_;
    core::ResidualData residuals_;
    std::string log_file_path_;
    bool auto_refresh_ = true;
    float refresh_interval_sec_ = 2.0f;
    double last_refresh_time_ = 0.0;

    ImPlotAxisFlags x_axis_flags_ = ImPlotAxisFlags_AutoFit;
    ImPlotAxisFlags y_axis_flags_ = ImPlotAxisFlags_AutoFit;

    void render_residual_chart();
    void render_log_monitor();
    void render_field_operations();
    void parse_log_file();
    void update_residuals();
};

} // namespace winfoam::gui