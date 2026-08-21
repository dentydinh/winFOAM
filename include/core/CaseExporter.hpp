#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <optional>
#include <variant>

namespace winfoam::core {

enum class DictEntryType {
    Scalar,
    Vector,
    Tensor,
    List,
    Dict,
    Word,
    String
};

struct DictEntry {
    DictEntryType type = DictEntryType::Word;
    std::string value;
    std::unordered_map<std::string, DictEntry> sub_dict;
    std::vector<DictEntry> list;
    int dimensions[7] = {0,0,0,0,0,0,0};
};

using Dictionary = std::unordered_map<std::string, DictEntry>;

struct BoundaryCondition {
    std::string name;
    std::string type;
    Dictionary coefficients;
    int patch_index = -1;
};

struct MeshInfo {
    std::size_t n_cells = 0;
    std::size_t n_faces = 0;
    std::size_t n_points = 0;
    std::size_t n_boundary_patches = 0;
    std::vector<std::string> patch_names;
    std::vector<std::size_t> patch_nfaces;
    std::vector<std::string> patch_types;
};

struct SolverSettings {
    std::string solver = "simpleFoam";
    std::string start_from = "latestTime";
    double start_time = 0.0;
    double end_time = 1000.0;
    double delta_t = 1.0;
    double write_interval = 100.0;
    int write_format = 1;
    std::string time_format = "general";
    double time_precision = 6;
    bool run_parallel = false;
    int n_processors = 4;
    std::string decompose_method = "scotch";
};

struct ResidualData {
    std::vector<double> iterations;
    std::unordered_map<std::string, std::vector<double>> residuals;
    std::unordered_map<std::string, std::vector<double>> initial_residuals;
    std::unordered_map<std::string, std::vector<double>> final_residuals;
};

class CaseExporter {
public:
    static bool write_control_dict(const std::filesystem::path& case_dir, const SolverSettings& settings);
    static bool write_fv_schemes(const std::filesystem::path& case_dir, const Dictionary& schemes);
    static bool write_fv_solution(const std::filesystem::path& case_dir, const Dictionary& solution);
    static bool write_boundary_file(const std::filesystem::path& case_dir,
                                    const std::string& field_name,
                                    const std::vector<BoundaryCondition>& bcs);
    static bool write_transport_properties(const std::filesystem::path& case_dir,
                                           const Dictionary& props);
    static bool write_turbulence_properties(const std::filesystem::path& case_dir,
                                            const Dictionary& props);

    static Dictionary create_default_control_dict(const SolverSettings& settings);
    static Dictionary create_default_fv_schemes();
    static Dictionary create_default_fv_solution();

private:
    static void write_dict_entry(std::ostream& os, const std::string& key, const DictEntry& entry, int indent);
    static std::string format_dimensions(const int dims[7]);
};

} // namespace winfoam::core