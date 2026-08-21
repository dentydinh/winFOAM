#include "core/CaseExporter.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace winfoam::core {

bool CaseExporter::write_control_dict(const std::filesystem::path& case_dir, const SolverSettings& settings) {
    std::filesystem::path file = case_dir / "system" / "controlDict";
    std::filesystem::create_directories(file.parent_path());

    Dictionary dict = create_default_control_dict(settings);

    std::ofstream ofs(file);
    if (!ofs) return false;

    ofs << "FoamFile\n{\n";
    ofs << "    version     2.0;\n";
    ofs << "    format      ascii;\n";
    ofs << "    class       dictionary;\n";
    ofs << "    location    \"system\";\n";
    ofs << "    object      controlDict;\n";
    ofs << "}\n\n";

    for (const auto& [key, entry] : dict) {
        write_dict_entry(ofs, key, entry, 0);
    }
    return true;
}

bool CaseExporter::write_fv_schemes(const std::filesystem::path& case_dir, const Dictionary& schemes) {
    std::filesystem::path file = case_dir / "system" / "fvSchemes";
    std::filesystem::create_directories(file.parent_path());

    std::ofstream ofs(file);
    if (!ofs) return false;

    ofs << "FoamFile\n{\n";
    ofs << "    version     2.0;\n";
    ofs << "    format      ascii;\n";
    ofs << "    class       dictionary;\n";
    ofs << "    location    \"system\";\n";
    ofs << "    object      fvSchemes;\n";
    ofs << "}\n\n";

    for (const auto& [key, entry] : schemes) {
        write_dict_entry(ofs, key, entry, 0);
    }
    return true;
}

bool CaseExporter::write_fv_solution(const std::filesystem::path& case_dir, const Dictionary& solution) {
    std::filesystem::path file = case_dir / "system" / "fvSolution";
    std::filesystem::create_directories(file.parent_path());

    std::ofstream ofs(file);
    if (!ofs) return false;

    ofs << "FoamFile\n{\n";
    ofs << "    version     2.0;\n";
    ofs << "    format      ascii;\n";
    ofs << "    class       dictionary;\n";
    ofs << "    location    \"system\";\n";
    ofs << "    object      fvSolution;\n";
    ofs << "}\n\n";

    for (const auto& [key, entry] : solution) {
        write_dict_entry(ofs, key, entry, 0);
    }
    return true;
}

bool CaseExporter::write_boundary_file(const std::filesystem::path& case_dir,
                                       const std::string& field_name,
                                       const std::vector<BoundaryCondition>& bcs) {
    std::filesystem::path file = case_dir / "0" / field_name;
    std::filesystem::create_directories(file.parent_path());

    std::ofstream ofs(file);
    if (!ofs) return false;

    ofs << "FoamFile\n{\n";
    ofs << "    version     2.0;\n";
    ofs << "    format      ascii;\n";
    ofs << "    class       volScalarField;\n";
    ofs << "    location    \"0\";\n";
    ofs << "    object      " << field_name << ";\n";
    ofs << "}\n\n";

    ofs << "dimensions      [0 0 0 0 0 0 0];\n\n";
    ofs << "internalField   uniform 0;\n\n";
    ofs << "boundaryField\n{\n";

    for (const auto& bc : bcs) {
        ofs << "    " << bc.name << "\n    {\n";
        ofs << "        type            " << bc.type << ";\n";
        for (const auto& [key, entry] : bc.coefficients) {
            write_dict_entry(ofs, key, entry, 2);
        }
        ofs << "    }\n";
    }
    ofs << "}\n";
    return true;
}

bool CaseExporter::write_transport_properties(const std::filesystem::path& case_dir, const Dictionary& props) {
    std::filesystem::path file = case_dir / "constant" / "transportProperties";
    std::filesystem::create_directories(file.parent_path());

    std::ofstream ofs(file);
    if (!ofs) return false;

    ofs << "FoamFile\n{\n";
    ofs << "    version     2.0;\n";
    ofs << "    format      ascii;\n";
    ofs << "    class       dictionary;\n";
    ofs << "    location    \"constant\";\n";
    ofs << "    object      transportProperties;\n";
    ofs << "}\n\n";

    for (const auto& [key, entry] : props) {
        write_dict_entry(ofs, key, entry, 0);
    }
    return true;
}

bool CaseExporter::write_turbulence_properties(const std::filesystem::path& case_dir, const Dictionary& props) {
    std::filesystem::path file = case_dir / "constant" / "turbulenceProperties";
    std::filesystem::create_directories(file.parent_path());

    std::ofstream ofs(file);
    if (!ofs) return false;

    ofs << "FoamFile\n{\n";
    ofs << "    version     2.0;\n";
    ofs << "    format      ascii;\n";
    ofs << "    class       dictionary;\n";
    ofs << "    location    \"constant\";\n";
    ofs << "    object      turbulenceProperties;\n";
    ofs << "}\n\n";

    for (const auto& [key, entry] : props) {
        write_dict_entry(ofs, key, entry, 0);
    }
    return true;
}

Dictionary CaseExporter::create_default_control_dict(const SolverSettings& settings) {
    Dictionary dict;
    dict["application"] = DictEntry{DictEntryType::Word, settings.solver};
    dict["startFrom"] = DictEntry{DictEntryType::Word, settings.start_from};
    dict["startTime"] = DictEntry{DictEntryType::Scalar, std::to_string(settings.start_time)};
    dict["stopAt"] = DictEntry{DictEntryType::Word, "endTime"};
    dict["endTime"] = DictEntry{DictEntryType::Scalar, std::to_string(settings.end_time)};
    dict["deltaT"] = DictEntry{DictEntryType::Scalar, std::to_string(settings.delta_t)};
    dict["writeControl"] = DictEntry{DictEntryType::Word, "timeStep"};
    dict["writeInterval"] = DictEntry{DictEntryType::Scalar, std::to_string(settings.write_interval)};
    dict["purgeWrite"] = DictEntry{DictEntryType::Scalar, "0"};
    dict["writeFormat"] = DictEntry{DictEntryType::Word, settings.write_format == 1 ? "ascii" : "binary"};
    dict["writePrecision"] = DictEntry{DictEntryType::Scalar, std::to_string(static_cast<int>(settings.time_precision))};
    dict["writeCompression"] = DictEntry{DictEntryType::Word, "off"};
    dict["timeFormat"] = DictEntry{DictEntryType::Word, settings.time_format};
    dict["timePrecision"] = DictEntry{DictEntryType::Scalar, std::to_string(static_cast<int>(settings.time_precision))};
    dict["runTimeModifiable"] = DictEntry{DictEntryType::Word, "true"};
    return dict;
}

Dictionary CaseExporter::create_default_fv_schemes() {
    Dictionary dict;
    Dictionary ddtSchemes{{"default", DictEntry{DictEntryType::Word, "steadyState"}}};
    dict["ddtSchemes"] = DictEntry{DictEntryType::Dict, "", ddtSchemes};

    Dictionary gradSchemes{{"default", DictEntry{DictEntryType::Word, "Gauss linear"}}};
    dict["gradSchemes"] = DictEntry{DictEntryType::Dict, "", gradSchemes};

    Dictionary divSchemes{
        {"default", DictEntry{DictEntryType::Word, "none"}},
        {"div(phi,U)", DictEntry{DictEntryType::Word, "Gauss upwind"}},
        {"div(phi,k)", DictEntry{DictEntryType::Word, "Gauss upwind"}},
        {"div(phi,epsilon)", DictEntry{DictEntryType::Word, "Gauss upwind"}},
        {"div(phi,omega)", DictEntry{DictEntryType::Word, "Gauss upwind"}},
        {"div((nuEff*dev2(T(grad(U)))))", DictEntry{DictEntryType::Word, "Gauss linear"}}
    };
    dict["divSchemes"] = DictEntry{DictEntryType::Dict, "", divSchemes};

    Dictionary laplacianSchemes{{"default", DictEntry{DictEntryType::Word, "Gauss linear corrected"}}};
    dict["laplacianSchemes"] = DictEntry{DictEntryType::Dict, "", laplacianSchemes};

    Dictionary interpolationSchemes{{"default", DictEntry{DictEntryType::Word, "linear"}}};
    dict["interpolationSchemes"] = DictEntry{DictEntryType::Dict, "", interpolationSchemes};

    Dictionary snGradSchemes{{"default", DictEntry{DictEntryType::Word, "corrected"}}};
    dict["snGradSchemes"] = DictEntry{DictEntryType::Dict, "", snGradSchemes};

    return dict;
}

Dictionary CaseExporter::create_default_fv_solution() {
    Dictionary dict;
    Dictionary solvers{
        {"p", DictEntry{DictEntryType::Dict, "", {
            {"solver", DictEntry{DictEntryType::Word, "GAMG"}},
            {"tolerance", DictEntry{DictEntryType::Scalar, "1e-06"}},
            {"relTol", DictEntry{DictEntryType::Scalar, "0.1"}},
            {"smoother", DictEntry{DictEntryType::Word, "GaussSeidel"}}
        }}},
        {"pFinal", DictEntry{DictEntryType::Dict, "", {
            {"$p", DictEntry{DictEntryType::Word, ""}},
            {"relTol", DictEntry{DictEntryType::Scalar, "0"}}
        }}},
        {"U", DictEntry{DictEntryType::Dict, "", {
            {"solver", DictEntry{DictEntryType::Word, "smoothSolver"}},
            {"smoother", DictEntry{DictEntryType::Word, "GaussSeidel"}},
            {"tolerance", DictEntry{DictEntryType::Scalar, "1e-05"}},
            {"relTol", DictEntry{DictEntryType::Scalar, "0.1"}}
        }}},
        {"UFinal", DictEntry{DictEntryType::Dict, "", {
            {"$U", DictEntry{DictEntryType::Word, ""}},
            {"relTol", DictEntry{DictEntryType::Scalar, "0"}}
        }}},
        {"k", DictEntry{DictEntryType::Dict, "", {
            {"solver", DictEntry{DictEntryType::Word, "smoothSolver"}},
            {"smoother", DictEntry{DictEntryType::Word, "GaussSeidel"}},
            {"tolerance", DictEntry{DictEntryType::Scalar, "1e-05"}},
            {"relTol", DictEntry{DictEntryType::Scalar, "0.1"}}
        }}},
        {"epsilon", DictEntry{DictEntryType::Dict, "", {
            {"solver", DictEntry{DictEntryType::Word, "smoothSolver"}},
            {"smoother", DictEntry{DictEntryType::Word, "GaussSeidel"}},
            {"tolerance", DictEntry{DictEntryType::Scalar, "1e-05"}},
            {"relTol", DictEntry{DictEntryType::Scalar, "0.1"}}
        }}}
    };
    dict["solvers"] = DictEntry{DictEntryType::Dict, "", solvers};

    Dictionary SIMPLE{
        {"nNonOrthogonalCorrectors", DictEntry{DictEntryType::Scalar, "0"}},
        {"consistent", DictEntry{DictEntryType::Word, "yes"}}
    };
    dict["SIMPLE"] = DictEntry{DictEntryType::Dict, "", SIMPLE};

    dict["relaxationFactors"] = DictEntry{DictEntryType::Dict, "", {
        {"fields", DictEntry{DictEntryType::Dict, "", {
            {"p", DictEntry{DictEntryType::Scalar, "0.3"}}
        }}},
        {"equations", DictEntry{DictEntryType::Dict, "", {
            {"U", DictEntry{DictEntryType::Scalar, "0.7"}},
            {"k", DictEntry{DictEntryType::Scalar, "0.7"}},
            {"epsilon", DictEntry{DictEntryType::Scalar, "0.7"}}
        }}}
    }};

    return dict;
}

void CaseExporter::write_dict_entry(std::ostream& os, const std::string& key, const DictEntry& entry, int indent) {
    std::string ind(indent * 4, ' ');
    switch (entry.type) {
        case DictEntryType::Scalar:
        case DictEntryType::Word:
            os << ind << key << " " << entry.value << ";\n";
            break;
        case DictEntryType::Vector:
            os << ind << key << " " << entry.value << ";\n";
            break;
        case DictEntryType::Tensor:
            os << ind << key << " " << entry.value << ";\n";
            break;
        case DictEntryType::List:
            os << ind << key << " (\n";
            for (const auto& item : entry.list) {
                os << ind << "    " << item.value << "\n";
            }
            os << ind << ");\n";
            break;
        case DictEntryType::Dict:
            os << ind << key << "\n" << ind << "{\n";
            for (const auto& [k, v] : entry.sub_dict) {
                write_dict_entry(os, k, v, indent + 1);
            }
            os << ind << "}\n";
            break;
        case DictEntryType::String:
            os << ind << key << " \"" << entry.value << "\";\n";
            break;
    }
}

std::string CaseExporter::format_dimensions(const int dims[7]) {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < 7; ++i) {
        oss << dims[i];
        if (i < 6) oss << " ";
    }
    oss << "]";
    return oss.str();
}

} // namespace winfoam::core