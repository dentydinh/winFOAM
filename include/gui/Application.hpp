#pragma once

#include <memory>

#include "imgui.h"

namespace winfoam::gui {

class Workbench;
class PreProcessing;
class Solutions;
class PostProcessing;
class Viewport;

class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool initialize(const char* title = "winFOAM", int width = 1600, int height = 900);
    void run();
    void shutdown();

    static Application& get() { return *instance_; }

    Workbench& workbench() { return *workbench_; }
    PreProcessing& pre_processing() { return *pre_processing_; }
    Solutions& solutions() { return *solutions_; }
    PostProcessing& post_processing() { return *post_processing_; }
    Viewport& viewport() { return *viewport_; }

    bool is_running() const noexcept { return running_; }
    void request_exit() { running_ = false; }

    ImGuiContext* imgui_context() const noexcept { return imgui_ctx_; }

private:
    static Application* instance_;

    bool running_ = false;
    ImGuiContext* imgui_ctx_ = nullptr;

    struct GLFWwindowDeleter {
        void operator()(void* window) const;
    };
    std::unique_ptr<void, GLFWwindowDeleter> window_;

    std::unique_ptr<Workbench> workbench_;
    std::unique_ptr<PreProcessing> pre_processing_;
    std::unique_ptr<Solutions> solutions_;
    std::unique_ptr<PostProcessing> post_processing_;
    std::unique_ptr<Viewport> viewport_;

    void init_imgui();
    void init_style();
    void load_fonts();
    void new_frame();
    void render();
    void process_events();
};

} // namespace winfoam::gui