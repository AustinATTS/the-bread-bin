#pragma once
#include <filesystem>
#include <optional>
#include <breadbin/ScriptFile.hpp>
#include <breadbin/ReloadManager.hpp>

namespace breadbin::gui {

    class LoafScripter {
    public:
        LoafScripter(
            bool& dirty,
            breadbin::core::ScriptFile& file,
            std::optional<std::filesystem::path>& path,
            breadbin::core::ReloadManager& reload_mgr
        );

        void set_on_saved(std::function<void(const std::filesystem::path&)> cb);

        void render(bool* p_open, uint32_t dockspace_id);

        bool save();
        bool save_as();
        void open_file(const std::filesystem::path& path);
        void reload_from_disk();

        bool is_dirty() const;

    private:
        bool& m_dirty;
        std::function<void(const std::filesystem::path&)> m_on_saved;
        breadbin::core::ScriptFile& m_file;
        std::optional<std::filesystem::path>& m_path;
        breadbin::core::ReloadManager& m_reload_mgr;
    };

}
