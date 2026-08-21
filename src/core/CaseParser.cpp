#include "core/CaseParser.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <cctype>

namespace winfoam::core {

std::optional<Dictionary> CaseParser::parse_dict_file(const std::filesystem::path& file) {
    std::ifstream ifs(file);
    if (!ifs) return std::nullopt;
    return parse_dict_stream(ifs);
}

std::optional<MeshInfo> CaseParser::parse_check_mesh(const std::filesystem::path& log_file) {
    std::ifstream ifs(log_file);
    if (!ifs) return std::nullopt;

    MeshInfo info;
    std::string line;
    std::regex cells_regex(R"(cells:\s+(\d+))");
    std::regex faces_regex(R"(faces:\s+(\d+))");
    std::regex points_regex(R"(points:\s+(\d+))");
    std::regex patches_regex(R"(boundary patches:\s+(\d+))");
    std::regex patch_regex(R"(\s*(\w+)\s+(\w+)\s+(\d+))");

    bool in_patches = false;
    while (std::getline(ifs, line)) {
        std::smatch match;
        if (std::regex_search(line, match, cells_regex)) info.n_cells = std::stoull(match[1]);
        else if (std::regex_search(line, match, faces_regex)) info.n_faces = std::stoull(match[1]);
        else if (std::regex_search(line, match, points_regex)) info.n_points = std::stoull(match[1]);
        else if (std::regex_search(line, match, patches_regex)) info.n_boundary_patches = std::stoull(match[1]);
        else if (line.find("patch name") != std::string::npos) in_patches = true;
        else if (in_patches && std::regex_search(line, match, patch_regex)) {
            info.patch_names.push_back(match[1]);
            info.patch_types.push_back(match[2]);
            info.patch_nfaces.push_back(std::stoull(match[3]));
        } else if (in_patches && line.empty()) {
            in_patches = false;
        }
    }
    return info;
}

std::optional<ResidualData> CaseParser::parse_solver_log(const std::filesystem::path& log_file) {
    std::ifstream ifs(log_file);
    if (!ifs) return std::nullopt;

    ResidualData data;
    std::string line;
    std::regex iter_regex(R"(Time = ([\d.]+))");
    std::regex residual_regex(R"(Solving for (\w+).*Initial residual = ([\deE\.-]+).*Final residual = ([\deE\.-]+))");
    std::regex simple_residual_regex(R"((\w+)\s+([\deE\.-]+)\s+([\deE\.-]+))");

    double current_time = 0.0;
    while (std::getline(ifs, line)) {
        std::smatch match;
        if (std::regex_search(line, match, iter_regex)) {
            current_time = std::stod(match[1]);
            data.iterations.push_back(current_time);
        } else if (std::regex_search(line, match, residual_regex)) {
            std::string field = match[1];
            double initial = std::stod(match[2]);
            double final = std::stod(match[3]);
            data.initial_residuals[field].push_back(initial);
            data.final_residuals[field].push_back(final);
            data.residuals[field].push_back(final);
        } else if (std::regex_search(line, match, simple_residual_regex) && line.find("Solving") == std::string::npos) {
            std::string field = match[1];
            double initial = std::stod(match[2]);
            double final = std::stod(match[3]);
            data.initial_residuals[field].push_back(initial);
            data.final_residuals[field].push_back(final);
            data.residuals[field].push_back(final);
        }
    }
    return data;
}

std::vector<BoundaryCondition> CaseParser::parse_boundary_file(const std::filesystem::path& file) {
    std::vector<BoundaryCondition> bcs;
    auto dict_opt = parse_dict_file(file);
    if (!dict_opt) return bcs;

    const auto& dict = *dict_opt;
    auto it = dict.find("boundaryField");
    if (it == dict.end() || it->second.type != DictEntryType::Dict) return bcs;

    for (const auto& [patch_name, patch_entry] : it->second.sub_dict) {
        if (patch_entry.type != DictEntryType::Dict) continue;
        BoundaryCondition bc;
        bc.name = patch_name;
        for (const auto& [key, val] : patch_entry.sub_dict) {
            if (key == "type" && val.type == DictEntryType::Word) {
                bc.type = val.value;
            } else {
                bc.coefficients[key] = val;
            }
        }
        bcs.push_back(bc);
    }
    return bcs;
}

std::optional<SolverSettings> CaseParser::parse_control_dict(const std::filesystem::path& file) {
    auto dict_opt = parse_dict_file(file);
    if (!dict_opt) return std::nullopt;

    const auto& dict = *dict_opt;
    SolverSettings settings;

    auto get_word = [&](const std::string& key, std::string& out) {
        auto it = dict.find(key);
        if (it != dict.end() && it->second.type == DictEntryType::Word) out = it->second.value;
    };
    auto get_scalar = [&](const std::string& key, double& out) {
        auto it = dict.find(key);
        if (it != dict.end() && it->second.type == DictEntryType::Scalar) out = std::stod(it->second.value);
    };

    get_word("application", settings.solver);
    get_word("startFrom", settings.start_from);
    get_scalar("startTime", settings.start_time);
    get_scalar("endTime", settings.end_time);
    get_scalar("deltaT", settings.delta_t);
    get_scalar("writeInterval", settings.write_interval);

    return settings;
}

std::optional<Dictionary> CaseParser::parse_dict_stream(std::istream& is) {
    Dictionary dict;
    skip_whitespace_and_comments(is);

    char c;
    while (is.get(c)) {
        if (c == '/' && is.peek() == '/') {
            is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            skip_whitespace_and_comments(is);
            continue;
        }
        if (c == '/' && is.peek() == '*') {
            is.get();
            while (is.get(c)) {
                if (c == '*' && is.peek() == '/') { is.get(); break; }
            }
            skip_whitespace_and_comments(is);
            continue;
        }
        if (std::isspace(c)) { skip_whitespace_and_comments(is); continue; }
        if (c == '}') break;

        is.unget();
        std::string key = read_word(is);
        skip_whitespace_and_comments(is);
        DictEntry entry = parse_value(is);
        if (!key.empty()) dict[key] = entry;
        skip_whitespace_and_comments(is);
    }
    return dict;
}

DictEntry CaseParser::parse_value(std::istream& is) {
    DictEntry entry;
    skip_whitespace_and_comments(is);

    char c;
    if (!is.get(c)) return entry;

    if (c == '(') {
        entry.type = DictEntryType::List;
        while (is.get(c)) {
            if (c == ')') break;
            if (std::isspace(c)) continue;
            is.unget();
            entry.list.push_back(parse_value(is));
        }
    } else if (c == '{') {
        entry.type = DictEntryType::Dict;
        while (is.get(c)) {
            if (c == '}') break;
            if (std::isspace(c)) continue;
            if (c == '/' && is.peek() == '/') { is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); continue; }
            if (c == '/' && is.peek() == '*') { is.get(); while (is.get(c)) { if (c == '*' && is.peek() == '/') { is.get(); break; } } continue; }
            is.unget();
            std::string key = read_word(is);
            skip_whitespace_and_comments(is);
            entry.sub_dict[key] = parse_value(is);
        }
    } else if (c == '"') {
        entry.type = DictEntryType::String;
        entry.value = read_quoted_string(is);
    } else {
        is.unget();
        std::string word = read_word(is);
        if (std::regex_match(word, std::regex(R"(^[\d\.\-eE]+$)"))) {
            entry.type = DictEntryType::Scalar;
        } else {
            entry.type = DictEntryType::Word;
        }
        entry.value = word;
    }
    return entry;
}

void CaseParser::skip_whitespace_and_comments(std::istream& is) {
    char c;
    while (is.get(c)) {
        if (std::isspace(c)) continue;
        if (c == '/' && is.peek() == '/') {
            is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (c == '/' && is.peek() == '*') {
            is.get();
            while (is.get(c)) {
                if (c == '*' && is.peek() == '/') { is.get(); break; }
            }
            continue;
        }
        is.unget();
        break;
    }
}

std::string CaseParser::read_word(std::istream& is) {
    std::string word;
    char c;
    while (is.get(c)) {
        if (std::isspace(c) || c == ';' || c == '(' || c == ')' || c == '{' || c == '}') {
            is.unget();
            break;
        }
        word += c;
    }
    return word;
}

std::string CaseParser::read_quoted_string(std::istream& is) {
    std::string str;
    char c;
    while (is.get(c)) {
        if (c == '"') break;
        if (c == '\\' && is.peek() == '"') { is.get(); str += '"'; }
        else str += c;
    }
    return str;
}

} // namespace winfoam::core