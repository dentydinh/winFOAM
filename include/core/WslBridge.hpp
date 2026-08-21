#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <filesystem>

namespace winfoam::core {

struct CommandResult {
    int exit_code = -1;
    std::string stdout_output;
    std::string stderr_output;
    bool timed_out = false;
};

struct WslConfig {
    std::string distro_name = "Ubuntu-22.04";
    std::string openfoam_version = "10";
    std::string openfoam_bashrc = "/opt/openfoam10/etc/bashrc";
    std::chrono::seconds default_timeout{300};
};

class WslBridge {
public:
    explicit WslBridge(const WslConfig& config = {});
    ~WslBridge() = default;

    WslBridge(const WslBridge&) = delete;
    WslBridge& operator=(const WslBridge&) = delete;
    WslBridge(WslBridge&&) noexcept = default;
    WslBridge& operator=(WslBridge&&) noexcept = default;

    CommandResult execute(const std::string& command,
                          const std::filesystem::path& working_dir = "",
                          std::chrono::seconds timeout = std::chrono::seconds(0)) const;

    CommandResult execute_async(const std::string& command,
                                const std::filesystem::path& working_dir = "");

    bool is_wsl_available() const noexcept;
    std::vector<std::string> list_distros() const;
    std::optional<std::string> get_openfoam_path() const;

    const WslConfig& config() const noexcept { return config_; }
    void set_config(const WslConfig& cfg) { config_ = cfg; }

private:
    WslConfig config_;
    std::string build_wsl_command(const std::string& cmd, const std::filesystem::path& cwd) const;
    std::string escape_for_wsl(const std::string& input) const;
};

} // namespace winfoam::core