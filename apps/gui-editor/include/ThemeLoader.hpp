#pragma once

#include <filesystem>
#include <vector>

namespace breadbin::theme {
    void ApplyDefaultTheme();
    bool LoadThemeFromFile(const std::filesystem::path& path);
    void SaveActiveTheme();
    void LoadPersistedTheme();

    const std::filesystem::path& GetActiveTheme();
    std::vector<std::filesystem::path> DiscoverThemes();

} // namespace breadbin::theme
