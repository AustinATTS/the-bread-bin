#include "AppDiscovery.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace BreadBin {

    AppDiscovery::AppDiscovery ( ) {

    }

    AppDiscovery::~AppDiscovery ( ) {

    }

    size_t AppDiscovery::ScanSystem ( ) {
        clear();

    #ifdef __linux__
        ScanLinuxApplications();
    #elif defined(_WIN32)
        ScanWindowsApplications();
    #elif defined(__APPLE__)
        ScanMacOSApplications();
    #endif

        ScanPathEnvironment();

        return applications_.size();
    }

    void AppDiscovery::ScanLinuxApplications ( ) {
        std::vector<std::string> app_dirs = {
            "/usr/share/applications",
            "/usr/local/share/applications",
            std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/share/applications"
        };

        for (const auto& dir : app_dirs) {
            if (!std::filesystem::exists(dir)) {
                continue;
            }

            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.path().extension() == ".desktop") {
                    AppInfo info = ParseDesktopFile(entry.path().string());
                    if (!info.name.empty() && !info.executable.empty()) {
                        applications_.push_back(info);
                    }
                }
            }
        }
    }

    AppInfo AppDiscovery::ParseDesktopFile (const std::string& filepath) {
        AppInfo info;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return info;
        }

        std::string line;
        bool in_desktop_entry = false;

        while (std::getline(file, line)) {
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) {
                line.clear();
            }
            else {
                if (start > 0) {
                    line.erase(0, start);
                }
                size_t end = line.find_last_not_of(" \t\r\n");
                if (end != std::string::npos && end + 1 < line.length()) {
                    line.erase(end + 1);
                }
            }

            if (line.empty() || line[0] == '#') {
                continue;
            }

            if (line == "[Desktop Entry]") {
                in_desktop_entry = true;
                continue;
            }
            else {
                if (line[0] == '[') {
                    in_desktop_entry = false;
                    continue;
                }
            }

            if (!in_desktop_entry) {
                continue;
            }

            size_t pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "Name") {
                info.name = value;
            }
            else {
                if (key == "Exec") {
                    size_t percent = value.find('%');
                    if (percent != std::string::npos) {
                        value = value.substr(0, percent);
                    }
                    if (!value.empty()) {
                        size_t end = value.find_last_not_of(" \t");
                        if (end != std::string::npos && end + 1 < value.length()) {
                            value.erase(end + 1);
                        }
                        else {
                            if (end == std::string::npos) {
                                value.clear();
                            }
                        }
                    }
                    info.executable = value;
                }
                else {
                    if (key == "Comment" || key == "GenericName") {
                        if (info.description.empty()) {
                            info.description = value;
                        }
                    }
                    else {
                        if (key == "Icon") {
                            info.icon_path = value;
                        }
                        else {
                            if (key == "Categories") {
                                size_t semicolon = value.find(';');
                                if (semicolon != std::string::npos) {
                                    info.category = value.substr(0, semicolon);
                                }
                                else {
                                    info.category = value;
                                }
                            }
                        }
                    }
                }
            }
        }

        return info;
    }

    void AppDiscovery::ScanWindowsApplications() {
    #ifdef _WIN32
        std::vector<std::string> menu_dirs;

        CHAR path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, path))) {
            menu_dirs.push_back(path);
        }
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAMS, NULL, 0, path))) {
            menu_dirs.push_back(path);
        }

        for (const auto& dir : menu_dirs) {
            if (!std::filesystem::exists(dir)) {
                continue;
            }

            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (entry.path().extension() == ".lnk") {
                    AppInfo info;
                    info.name = entry.path().stem().string();
                    info.executable = entry.path().string();
                    info.category = "Application";
                    applications_.push_back(info);
                }
            }
        }
    #endif
    }

    void AppDiscovery::ScanMacOSApplications ( ) {
    #ifdef __APPLE__
        std::vector<std::string> app_dirs = {
            "/Applications",
            std::string(getenv("HOME") ? getenv("HOME") : "") + "/Applications"
        };

        for (const auto& dir : app_dirs) {
            if (!std::filesystem::exists(dir)) {
                continue;
            }

            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.path().extension() == ".app") {
                    AppInfo info;
                    info.name = entry.path().stem().string();
                    info.executable = entry.path().string();
                    info.category = "Application";
                    applications_.push_back(info);
                }
            }
        }
    #endif
    }

    void AppDiscovery::ScanPathEnvironment ( ) {
        const char* path_environment = std::getenv("PATH");
        if (!path_environment) {
            return;
        }

        std::string path_string(path_environment);
        std::istringstream iss(path_string);
        std::string dir;

    #ifdef _WIN32
        char delimiter = ';';
    #else
        char delimiter = ':';
    #endif

        while (std::getline(iss, dir, delimiter)) {
            if (!std::filesystem::exists(dir)) {
                continue;
            }

            try {
                for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                    if (!entry.is_regular_file()) {
                        continue;
                    }

                    auto perms = entry.status().permissions();
                    bool is_executable = (perms & std::filesystem::perms::owner_exec) != std::filesystem::perms::none;

                    if (is_executable) {
                        std::string filename = entry.path().filename().string();

                        bool already_exists = false;
                        for (const auto& app : applications_) {
                            if (app.executable.find(filename) != std::string::npos) {
                                already_exists = true;
                                break;
                            }
                        }

                        if (!already_exists) {
                            AppInfo info;
                            info.name = filename;
                            info.executable = entry.path().string();
                            info.category = "Command-Line";
                            applications_.push_back(info);
                        }
                    }
                }
            } catch (...) {

            }
        }
    }

    const std::vector<AppInfo>& AppDiscovery::GetApplications ( ) const {
        return applications_;
    }

    std::vector<AppInfo> AppDiscovery::SearchApplications (const std::string& query) const {
        std::vector<AppInfo> results;

        if (query.empty()) {
            return applications_;
        }

        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

        for (const auto& app : applications_) {
            std::string lower_name = app.name;
            std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

            if (lower_name.find(lower_query) != std::string::npos) {
                results.push_back(app);
            }
        }

        return results;
    }

    std::vector<AppInfo> AppDiscovery::GetApplicationsByCategory (const std::string& category) const {
        std::vector<AppInfo> results;

        for (const auto& app : applications_) {
            if (app.category == category) {
                results.push_back(app);
            }
        }

        return results;
    }

    std::vector<std::string> AppDiscovery::GetCategories ( ) const {
        std::vector<std::string> categories;

        for (const auto& app : applications_) {
            if (!app.category.empty()) {
                auto it = std::find(categories.begin(), categories.end(), app.category);
                if (it == categories.end()) {
                    categories.push_back(app.category);
                }
            }
        }

        std::sort(categories.begin(), categories.end());
        return categories;
    }

    void AppDiscovery::clear ( ) {
        applications_.clear();
    }

    bool AppDiscovery::SaveCache (const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        for (const auto& app : applications_) {
            file << "NAME:" << app.name << "\n";
            file << "EXEC:" << app.executable << "\n";
            file << "DESC:" << app.description << "\n";
            file << "ICON:" << app.icon_path << "\n";
            file << "CATEGORY:" << app.category << "\n";
            file << "---\n";
        }

        return true;
    }

    bool AppDiscovery::LoadCache (const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        clear();

        AppInfo current_app;
        std::string line;

        while (std::getline(file, line)) {
            if (line == "---") {
                if (!current_app.name.empty()) {
                    applications_.push_back(current_app);
                    current_app = AppInfo();
                }
                continue;
            }

            size_t pos = line.find(':');
            if (pos == std::string::npos) {
                continue;
            }

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "NAME") {
                current_app.name = value;
            }
            else {
                if (key == "EXEC") {
                    current_app.executable = value;
                }
                else {
                    if (key == "DESC") {
                        current_app.description = value;
                    }
                    else {
                        if (key == "ICON") {
                            current_app.icon_path = value;
                        }
                        else {
                            if (key == "CATEGORY") {
                                current_app.category = value;
                            }
                        }
                    }
                }
            }
        }

        if (!current_app.name.empty()) {
            applications_.push_back(current_app);
        }

        return true;
    }

} // namespace BreadBin