#include "gui/Application.hpp"
#include "gui/Workbench.hpp"
#include "gui/PreProcessing.hpp"
#include "gui/Solutions.hpp"
#include "gui/PostProcessing.hpp"
#include "gui/Viewport.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

namespace winfoam::gui {

Application* Application::instance_ = nullptr;

Application::Application() {
    instance_ = this;
}

Application::~Application() {
    shutdown();
    instance_ = nullptr;
}

bool Application::initialize(const char* title, int width, int height) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    window_ = std::unique_ptr<void, GLFWwindowDeleter>(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    init_imgui();
    init_style();
    load_fonts();

    workbench_ = std::make_unique<Workbench>();
    pre_processing_ = std::make_unique<PreProcessing>();
    solutions_ = std::make_unique<Solutions>();
    post_processing_ = std::make_unique<PostProcessing>();
    viewport_ = std::make_unique<Viewport>();

    running_ = true;
    return true;
}

void Application::run() {
    while (running_ && !glfwWindowShouldClose(static_cast<GLFWwindow*>(window_.get()))) {
        process_events();
        new_frame();
        render();
    }
}

void Application::shutdown() {
    if (imgui_ctx_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imgui_ctx_);
        imgui_ctx_ = nullptr;
    }
    if (window_) {
        glfwTerminate();
        window_.reset();
    }
    running_ = false;
}

void Application::init_imgui() {
    IMGUI_CHECKVERSION();
    imgui_ctx_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_ctx_);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = "imgui.ini";

    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window_.get()), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Application::init_style() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
}

void Application::load_fonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    io.Fonts->Build();
}

void Application::new_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::render() {
    workbench_->render_dockspace();

    if (workbench_->current_theme() == core::Theme::Dark) {
        workbench_->apply_theme(core::Theme::Dark);
    }

    workbench_->render_menu_bar();

    pre_processing_->render();
    solutions_->render();
    post_processing_->render();
    viewport_->render();

    workbench_->render_status_bar();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(window_.get()), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(static_cast<GLFWwindow*>(window_.get()));
}

void Application::process_events() {
    glfwPollEvents();
}

void Application::GLFWwindowDeleter::operator()(void* window) const {
    if (window) glfwDestroyWindow(static_cast<GLFWwindow*>(window));
}

} // namespace winfoam::gui