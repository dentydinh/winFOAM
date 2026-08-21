#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <optional>
#include <variant>

#include "CaseExporter.hpp"

namespace winfoam::core {

class CaseParser {
public:
    static std::optional<Dictionary> parse_dict_file(const std::filesystem::path& file);
    static std::optional<MeshInfo> parse_check_mesh(const std::filesystem::path& log_file);
    static std::optional<ResidualData> parse_solver_log(const std::filesystem::path& log_file);
    static std::vector<BoundaryCondition> parse_boundary_file(const std::filesystem::path& file);
    static std::optional<SolverSettings> parse_control_dict(const std::filesystem::path& file);

private:
    static std::optional<Dictionary> parse_dict_stream(std::istream& is);
    static DictEntry parse_value(std::istream& is);
    static void skip_whitespace_and_comments(std::istream& is);
    static std::string read_word(std::istream& is);
    static std::string read_quoted_string(std::istream& is);
};

} // namespace winfoam::core