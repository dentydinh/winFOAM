#include "gui/Workbench.hpp"
#include "gui/Application.hpp"
#include "core/Common.hpp"

#include <imgui.h>

namespace winfoam::gui {

Workbench::Workbench() {
    apply_theme(theme_);
}

Workbench::~Workbench() = default;

void Workbench::render_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Case", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Case...", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save Case", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save Case As...", "Ctrl+Shift+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Import OpenFOAM Case...")) {}
            if (ImGui::MenuItem("Export to OpenFOAM")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                Application::get().request_exit();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Preferences", "Ctrl+,")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            render_view_menu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Run blockMesh")) {}
            if (ImGui::MenuItem("Run snappyHexMesh")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Run Solver")) {}
            if (ImGui::MenuItem("Decompose / Reconstruct")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            render_help_menu();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Workbench::render_dockspace() {
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, window_flags);
    if (!opt_padding) ImGui::PopStyleVar();
    if (opt_fullscreen) { ImGui::PopStyleVar(2); }

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    ImGui::End();
}

void Workbench::render_status_bar() {
    ImGui::Begin("StatusBar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                         ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Ready");
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Text("winFOAM %s", core::APP_VERSION);
    ImGui::End();
}

void Workbench::render() {
    render_menu_bar();
    render_dockspace();
    render_status_bar();
}

void Workbench::set_theme(core::Theme theme) {
    theme_ = theme;
    apply_theme(theme);
}

void Workbench::apply_theme(core::Theme theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    switch (theme) {
        case core::Theme::Dark:
            ImGui::StyleColorsDark();
            break;
        case core::Theme::Light:
            ImGui::StyleColorsLight();
            break;
        case core::Theme::Dracula:
            colors[ImGuiCol_Text] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
            colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
            colors[ImGuiCol_Border] = ImVec4(0.44f, 0.44f, 0.44f, 0.50f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.33f, 0.36f, 0.43f, 1.00f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.47f, 0.56f, 1.00f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
            colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.16f, 0.19f, 1.00f);
            colors[ImGuiCol_Button] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.80f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_CheckMark] = ImVec4(0.96f, 0.61f, 0.07f, 1.00f);
            colors[ImGuiCol_SliderGrab] = ImVec4(0.96f, 0.61f, 0.07f, 1.00f);
            colors[ImGuiCol_SliderGrabActive] = ImVec4(0.96f, 0.61f, 0.07f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.80f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_Separator] = ImVec4(0.44f, 0.44f, 0.44f, 0.50f);
            colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.78f);
            colors[ImGuiCol_SeparatorActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            colors[ImGuiCol_ResizeGrip] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.80f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
            colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);
            colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.36f, 0.43f, 1.00f);
            colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
            colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.28f, 0.30f, 0.35f, 1.00f);
            colors[ImGuiCol_PlotLines] = ImVec4(0.96f, 0.61f, 0.07f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.96f, 0.61f, 0.07f, 1.00f);
            colors[ImGuiCol_PlotHistogram] = ImVec4(0.96f, 0.61f, 0.07f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.96f, 0.61f, 0.07f, 1.00f);
            colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
            colors[ImGuiCol_TableBorderStrong] = ImVec4(0.44f, 0.44f, 0.44f, 0.50f);
            colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.50f);
            colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
            colors[ImGuiCol_TextSelectedBg] = ImVec4(0.96f, 0.61f, 0.07f, 0.35f);
            colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
            break;
        case core::Theme::Classic:
            ImGui::StyleColorsClassic();
            break;
    }
}

void Workbench::render_view_menu() {
    ImGui::MenuItem("Show Demo Window", nullptr, &show_demo_);
    ImGui::MenuItem("Show Metrics", nullptr, &show_metrics_);
    ImGui::Separator();
    if (ImGui::BeginMenu("Theme")) {
        if (ImGui::MenuItem("Dark", nullptr, theme_ == core::Theme::Dark)) set_theme(core::Theme::Dark);
        if (ImGui::MenuItem("Light", nullptr, theme_ == core::Theme::Light)) set_theme(core::Theme::Light);
        if (ImGui::MenuItem("Dracula", nullptr, theme_ == core::Theme::Dracula)) set_theme(core::Theme::Dracula);
        if (ImGui::MenuItem("Classic", nullptr, theme_ == core::Theme::Classic)) set_theme(core::Theme::Classic);
        ImGui::EndMenu();
    }
    if (show_demo_) ImGui::ShowDemoWindow(&show_demo_);
    if (show_metrics_) ImGui::ShowMetricsWindow(&show_metrics_);
}

void Workbench::render_help_menu() {
    if (ImGui::MenuItem("Documentation")) {}
    if (ImGui::MenuItem("Report Issue")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("About")) show_about_ = true;
    if (show_about_) render_about_dialog();
}

void Workbench::render_about_dialog() {
    ImGui::OpenPopup("About winFOAM");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("About winFOAM", &show_about_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s v%s", core::APP_NAME, core::APP_VERSION);
        ImGui::Separator();
        ImGui::Text("Windows-native OpenFOAM GUI");
        ImGui::Text("Built with Dear ImGui, ImPlot, GLFW, VTK");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            show_about_ = false;
        }
        ImGui::EndPopup();
    }
}

} // namespace winfoam::gui