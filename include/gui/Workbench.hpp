#pragma once

#include "core/Common.hpp"

namespace winfoam::gui {

class Workbench {
public:
    Workbench();
    ~Workbench();

    void render_menu_bar();
    void render_dockspace();
    void render_status_bar();
    void render();

    void set_theme(core::Theme theme);
    core::Theme current_theme() const noexcept { return theme_; }
    void apply_theme(core::Theme theme);

private:
    core::Theme theme_ = core::Theme::Dark;
    bool show_demo_ = false;
    bool show_metrics_ = false;
    bool show_about_ = false;
    void render_view_menu();
    void render_help_menu();
    void render_about_dialog();
};

} // namespace winfoam::gui