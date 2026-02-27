#ifndef APPDISCOVERY_H
#define APPDISCOVERY_H

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

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
  AppInfo(std::string name, std::string executable)
      : name(std::move(name)), executable(std::move(executable)) {}
};

class AppDiscovery {
 public:
  AppDiscovery();
  ~AppDiscovery();

  size_t ScanSystem();
  [[nodiscard]] const std::vector<AppInfo>& GetApplications() const;
  [[nodiscard]] std::vector<AppInfo> SearchApplications(
      const std::string& query) const;
  [[nodiscard]] std::vector<AppInfo> GetApplicationsByCategory(
      const std::string& category) const;
  [[nodiscard]] std::vector<std::string> GetCategories() const;
  void clear();
  [[nodiscard]] bool SaveCache(const std::string& filepath) const;
  bool LoadCache(const std::string& filepath);

 private:
  void ScanLinuxApplications();
  void ScanWindowsApplications();
  void ScanMacOSApplications();
  void ScanPathEnvironment();
  static AppInfo ParseDesktopFile(const std::string& filepath);

  std::vector<AppInfo> applications_;
};
}  // namespace BreadBin

#endif  // APPDISCOVERY_H