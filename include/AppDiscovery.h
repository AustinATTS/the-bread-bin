#ifndef APPDISCOVERY_H
#define APPDISCOVERY_H

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace BreadBin {

    struct AppInfo {
        std::string name;
        std::string executable;
        std::string description;
        std::string iconPath;
        std::string category;
        std::vector<std::string> commonFlags;
        std::map<std::string, std::string> metadata;

        AppInfo() = default;
        AppInfo(const std::string& n, const std::string& exec)
            : name(n), executable(exec) {}
    };

    class AppDiscovery {
        public:
            AppDiscovery();
            ~AppDiscovery();

            size_t scanSystem();

            const std::vector<AppInfo>& getApplications() const;

            std::vector<AppInfo> searchApplications(const std::string& query) const;

            std::vector<AppInfo> getApplicationsByCategory(const std::string& category) const;

            std::vector<std::string> getCategories() const;

            void clear();

            bool saveCache(const std::string& filepath) const;

            bool loadCache(const std::string& filepath);

        private:
            void scanLinuxApplications();
            void scanWindowsApplications();
            void scanMacOSApplications();
            void scanPathEnvironment();

            AppInfo parseDesktopFile(const std::string& filepath);

            std::vector<AppInfo> applications_;
    };

} // namespace BreadBin

#endif // APPDISCOVERY_H
