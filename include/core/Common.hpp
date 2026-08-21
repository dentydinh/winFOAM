#pragma once

#include <string>

namespace winfoam::core {

inline const char* APP_NAME = "winFOAM";
inline const char* APP_VERSION = "1.0.0";
inline const char* ORG_NAME = "winFOAM";

enum class Theme { Dark, Light, Dracula, Classic };

struct AppSettings {
    Theme theme = Theme::Dark;
    bool auto_save = true;
    int auto_save_interval_sec = 30;
    std::string last_case_path;
    bool show_demo_window = false;
    bool show_metrics_window = false;
    float font_scale = 1.0f;
    std::string font_path;
};

} // namespace winfoam::core