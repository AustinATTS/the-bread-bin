#pragma once
#include <string>
#include <filesystem>

namespace breadbin::gui {
    class TextEditor {
        public:
            TextEditor() = default;

            void open_file(const std::filesystem::path& path);
            void save_file();
            void render(bool* p_open);

            bool is_open() const {
                return m_is_open;
            }
            void set_open(bool open) {
                m_is_open = open;
            }
            bool is_dirty() const {
                return m_dirty;
            }
            const std::filesystem::path& get_current_path() const {
                return m_current_path;
            }

        private:
            std::filesystem::path m_current_path;
            std::string m_buffer;
            bool m_is_open = false;
            bool m_dirty = false;
    };

} // namespace breadbin::gui