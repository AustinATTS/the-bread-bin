#include <breadbin/ReloadManager.hpp>
#include <system_error>

namespace breadbin::core {
    void ReloadManager::watch_file(const std::filesystem::path& path, std::function<void()> callback) {
        if (path.empty()) {
            return;
        }

        std::error_code ec;
        if (std::filesystem::exists(path, ec)) {
            m_watches[path] = {
                std::filesystem::last_write_time(path, ec),
                callback,
                false,
                0
            };
        }
    }

    void ReloadManager::watch_directory(const std::filesystem::path& path, std::function<void()> callback) {
        if (path.empty()) {
            return;
        }

        std::error_code ec;
        if (std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec)) {
            m_watches[path] = {
                std::filesystem::last_write_time(path, ec),
                callback,
                true,
                get_dir_signature(path)
            };
        }
    }

    void ReloadManager::unwatch_file(const std::filesystem::path& path) {
        m_watches.erase(path);
    }

    void ReloadManager::notify_internal_change(const std::filesystem::path& path) {
        auto it = m_watches.find(path);
        if (it != m_watches.end()) {
            std::error_code ec;
            if (std::filesystem::exists(path, ec)) {
                it->second.last_modified = std::filesystem::last_write_time(path, ec);
            }
        }
    }

    void ReloadManager::update() {
        std::error_code ec;

        for (auto& [path, item] : m_watches) {
            if (!std::filesystem::exists(path, ec)) {
                item.callback();
                continue;
            }

            if (item.is_directory) {
                size_t current_sig = get_dir_signature(path);
                if (current_sig != item.entry_count) {
                    item.entry_count = current_sig;
                    item.callback();
                }
            }
            else {
                auto current_time = std::filesystem::last_write_time(path, ec);
                if (!ec && current_time > item.last_modified) {
                    item.last_modified = current_time;
                    item.callback();
                }
            }
        }
    }

    size_t ReloadManager::get_dir_signature(const std::filesystem::path& path) {
        std::error_code ec;
        size_t count = 0;
        if (std::filesystem::exists(path, ec)) {
            for (auto const& entry : std::filesystem::directory_iterator(path, ec)) {
                count++;
            }
        }
        return count;
    }

} // namespace breadbin::core