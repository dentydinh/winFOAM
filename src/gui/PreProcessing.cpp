#include "gui/PreProcessing.hpp"
#include "core/WslBridge.hpp"
#include "core/CaseExporter.hpp"

#include <imgui.h>
#include <implot.h>
#include <fstream>
#include <filesystem>
#include <cstring>

namespace winfoam::gui {

PreProcessing::PreProcessing() = default;

PreProcessing::~PreProcessing() = default;

void PreProcessing::render() {
    ImGui::Begin("Pre-Processing");
    ImGui::BeginTabBar("PreProcessingTabs");

    if (ImGui::BeginTabItem("Mesh Generation")) {
        render_mesh_generation();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Boundary Conditions")) {
        render_boundary_conditions();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("blockMeshDict")) {
        render_block_mesh_editor();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("snappyHexMesh")) {
        render_snappy_hex_mesh();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
}

void PreProcessing::set_case_path(const std::string& path) {
    case_path_ = path;
    load_block_mesh_dict();
}

void PreProcessing::render_mesh_generation() {
    ImGui::Text("Case Path: %s", case_path_.empty() ? "Not set" : case_path_.c_str());
    ImGui::Separator();

    if (ImGui::Button("Select Case Directory")) {
        // TODO: File dialog
    }

    ImGui::Separator();
    ImGui::Text("Mesh Tool:");
    static int mesh_tool_idx = 0;
    const char* mesh_tools[] = {"blockMesh", "snappyHexMesh", "cfMesh"};
    if (ImGui::Combo("##mesh_tool", &mesh_tool_idx, mesh_tools, IM_ARRAYSIZE(mesh_tools))) {
        mesh_tool_ = mesh_tools[mesh_tool_idx];
    }

    if (mesh_tool_ == "blockMesh") {
        if (ImGui::Button("Run blockMesh")) run_block_mesh();
        ImGui::SameLine();
        if (ImGui::Button("Check Mesh")) {
            // TODO: run checkMesh
        }
    } else if (mesh_tool_ == "snappyHexMesh") {
        if (ImGui::Button("Run snappyHexMesh")) run_snappy_hex_mesh();
    }
}

void PreProcessing::render_boundary_conditions() {
    if (case_path_.empty()) {
        ImGui::TextColored(ImVec4(1,0.5,0,1), "No case selected");
        return;
    }

    ImGui::Text("Boundary Patches:");
    ImGui::Separator();

    static std::vector<core::BoundaryCondition> patch_list;

    if (ImGui::Button("Refresh from 0/")) {
        // Parse boundary files
    }

    if (ImGui::BeginTable("BoundaryTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Patch Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Field");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < patch_list.size(); ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", patch_list[i].name.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", patch_list[i].type.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("U, p, k, epsilon...");
            ImGui::TableSetColumnIndex(3);
            if (ImGui::SmallButton(("Edit##" + std::to_string(i)).c_str())) {
                boundary_state_.selected_patch = patch_list[i].name;
                boundary_state_.editing_bc = patch_list[i];
                boundary_state_.is_editing = true;
            }
        }
    }

    if (boundary_state_.is_editing) {
        ImGui::Separator();
        ImGui::Text("Editing: %s", boundary_state_.selected_patch.c_str());
        static char type_buf[256];
        std::strncpy(type_buf, boundary_state_.editing_bc.type.c_str(), sizeof(type_buf) - 1);
        type_buf[sizeof(type_buf) - 1] = '\0';
        if (ImGui::InputText("Type", type_buf, sizeof(type_buf))) {
            boundary_state_.editing_bc.type = type_buf;
        }
        // Coefficients editor
        if (ImGui::Button("Save")) {
            boundary_state_.is_editing = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) boundary_state_.is_editing = false;
    }
}

void PreProcessing::render_block_mesh_editor() {
    if (ImGui::Button("Load blockMeshDict")) load_block_mesh_dict();
    ImGui::SameLine();
    if (ImGui::Button("Save blockMeshDict")) save_block_mesh_dict();

    ImGui::Separator();
    static char dict_buf[16384];
    std::strncpy(dict_buf, block_mesh_dict_content_.c_str(), sizeof(dict_buf) - 1);
    dict_buf[sizeof(dict_buf) - 1] = '\0';
    if (ImGui::InputTextMultiline("##blockMeshDict", dict_buf, sizeof(dict_buf),
                                  ImVec2(-1, -1), ImGuiInputTextFlags_AllowTabInput)) {
        block_mesh_dict_content_ = dict_buf;
    }
}

void PreProcessing::render_snappy_hex_mesh() {
    ImGui::Text("snappyHexMeshDict configuration");
    ImGui::Separator();
    ImGui::TextWrapped("Configure castellatedMesh, snap, addLayers settings here.");
}

void PreProcessing::load_block_mesh_dict() {
    if (case_path_.empty()) return;
    std::filesystem::path file = std::filesystem::path(case_path_) / "system" / "blockMeshDict";
    std::ifstream ifs(file);
    if (ifs) {
        block_mesh_dict_content_ = std::string(std::istreambuf_iterator<char>(ifs), {});
    }
}

bool PreProcessing::save_block_mesh_dict() {
    if (case_path_.empty()) return false;
    std::filesystem::path file = std::filesystem::path(case_path_) / "system" / "blockMeshDict";
    std::ofstream ofs(file);
    if (ofs) {
        ofs << block_mesh_dict_content_;
        return true;
    }
    return false;
}

void PreProcessing::run_block_mesh() {
    if (case_path_.empty()) return;
    core::WslBridge bridge;
    auto result = bridge.execute("blockMesh", case_path_, std::chrono::seconds(300));
}

void PreProcessing::run_snappy_hex_mesh() {
    if (case_path_.empty()) return;
    core::WslBridge bridge;
    auto result = bridge.execute("snappyHexMesh -overwrite", case_path_, std::chrono::seconds(600));
}

} // namespace winfoam::gui