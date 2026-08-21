#include "core/WslBridge.hpp"

#include <cstdio>
#include <cstdlib>
#include <array>
#include <memory>
#include <thread>
#include <future>
#include <sstream>
#include <algorithm>

namespace winfoam::core {

WslBridge::WslBridge(const WslConfig& config) : config_(config) {}

CommandResult WslBridge::execute(const std::string& command,
                                  const std::filesystem::path& working_dir,
                                  std::chrono::seconds timeout) const {
    std::string wsl_cmd = build_wsl_command(command, working_dir);
    CommandResult result;

    auto exec_task = [&]() -> CommandResult {
        CommandResult local_result;
        std::array<char, 4096> buffer;
        std::string full_cmd = "wsl.exe -d " + config_.distro_name + " -- " + wsl_cmd;

        FILE* pipe = _popen(full_cmd.c_str(), "r");
        if (!pipe) {
            local_result.exit_code = -1;
            local_result.stderr_output = "Failed to open pipe to WSL";
            return local_result;
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            local_result.stdout_output += buffer.data();
        }

        int exit_code = _pclose(pipe);
        local_result.exit_code = exit_code;
        return local_result;
    };

    if (timeout.count() > 0) {
        auto future = std::async(std::launch::async, exec_task);
        if (future.wait_for(timeout) == std::future_status::ready) {
            result = future.get();
        } else {
            result.timed_out = true;
            result.exit_code = -1;
            result.stderr_output = "Command timed out after " + std::to_string(timeout.count()) + " seconds";
        }
    } else {
        result = exec_task();
    }

    return result;
}

CommandResult WslBridge::execute_async(const std::string& command,
                                       const std::filesystem::path& working_dir) {
    return execute(command, working_dir, std::chrono::seconds(0));
}

bool WslBridge::is_wsl_available() const noexcept {
    auto result = execute("echo test", "", std::chrono::seconds(5));
    return result.exit_code == 0;
}

std::vector<std::string> WslBridge::list_distros() const {
    std::vector<std::string> distros;
    std::array<char, 4096> buffer;
    FILE* pipe = _popen("wsl.exe -l -q", "r");
    if (!pipe) return distros;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::string line = buffer.data();
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (!line.empty()) distros.push_back(line);
    }
    _pclose(pipe);
    return distros;
}

std::optional<std::string> WslBridge::get_openfoam_path() const {
    auto result = execute("which foamExec", "", std::chrono::seconds(10));
    if (result.exit_code == 0 && !result.stdout_output.empty()) {
        std::string path = result.stdout_output;
        path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());
        return path;
    }
    return std::nullopt;
}

std::string WslBridge::build_wsl_command(const std::string& cmd, const std::filesystem::path& cwd) const {
    std::ostringstream oss;
    oss << "source " << config_.openfoam_bashrc << " >/dev/null 2>&1";
    if (!cwd.empty()) {
        oss << " && cd " << escape_for_wsl(cwd.string());
    }
    oss << " && " << cmd;
    return oss.str();
}

std::string WslBridge::escape_for_wsl(const std::string& input) const {
    std::ostringstream oss;
    oss << "'";
    for (char c : input) {
        if (c == '\'') {
            oss << "'\\''";
        } else {
            oss << c;
        }
    }
    oss << "'";
    return oss.str();
}

} // namespace winfoam::core