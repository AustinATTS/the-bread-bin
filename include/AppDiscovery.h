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
        std::string icon_path;
        std::string category;
        std::vector<std::string> common_flags;
        std::map<std::string, std::string> metadata;

        AppInfo() = default;
        AppInfo(const std::string& name, const std::string& executable)
            : name(name), executable(executable) {}
    };

    class AppDiscovery {
        public:
            AppDiscovery ( );
            ~AppDiscovery ( );

            size_t ScanSystem ( );

            const std::vector<AppInfo>& GetApplications ( ) const;

            std::vector<AppInfo> SearchApplications (const std::string& query) const;

            std::vector<AppInfo> GetApplicationsByCategory (const std::string& category) const;

            std::vector<std::string> GetCategories ( ) const;

            void clear ( );

            bool SaveCache (const std::string& filepath) const;

            bool LoadCache (const std::string& filepath);

        private:
            void ScanLinuxApplications ( );
            void ScanWindowsApplications ( );
            void ScanMacOSApplications ( );
            void ScanPathEnvironment ( );

            AppInfo ParseDesktopFile (const std::string& filepath);

            std::vector<AppInfo> applications_;
    };

} // namespace BreadBin

#endif // APPDISCOVERY_H