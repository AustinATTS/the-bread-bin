#pragma once
#include <filesystem>
#include <map>
#include <functional>

namespace breadbin::core {

    class ReloadManager {
        public:
            ReloadManager() = default;

            void watch_file(const std::filesystem::path& path, std::function<void()> callback);

            void watch_directory(const std::filesystem::path& path, std::function<void()> callback);

            void unwatch_file(const std::filesystem::path& path);

            void notify_internal_change(const std::filesystem::path& path);

            void update();

        private:
            struct WatchItem {
                std::filesystem::file_time_type last_modified;
                std::function<void()> callback;
                bool is_directory;
                size_t entry_count = 0;
            };

            std::map<std::filesystem::path, WatchItem> m_watches;

            size_t get_dir_signature(const std::filesystem::path& path);
    };

} // namespace breadbin::core