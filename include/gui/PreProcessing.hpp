#pragma once

#include "core/Common.hpp"
#include "core/CaseExporter.hpp"
#include "core/WslBridge.hpp"

namespace winfoam::gui {

class PreProcessing {
public:
    PreProcessing();
    ~PreProcessing();

    void render();
    void set_case_path(const std::string& path);
    const std::string& case_path() const noexcept { return case_path_; }

private:
    std::string case_path_;
    std::string mesh_tool_ = "blockMesh";
    std::string block_mesh_dict_content_;
    bool show_block_mesh_editor_ = false;
    bool show_snappy_hex_mesh_ = false;
    bool show_boundary_editor_ = false;

    struct BoundaryPanelState {
        std::string selected_patch;
        core::BoundaryCondition editing_bc;
        bool is_editing = false;
    } boundary_state_;

    void render_mesh_generation();
    void render_boundary_conditions();
    void render_block_mesh_editor();
    void render_snappy_hex_mesh();
    void load_block_mesh_dict();
    bool save_block_mesh_dict();
    void run_block_mesh();
    void run_snappy_hex_mesh();
};

} // namespace winfoam::gui