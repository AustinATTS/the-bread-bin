#include <TextEditor.hpp>
#include <nfd.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace breadbin::gui {

    static std::filesystem::path file_dir() {
        return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin";
    }

    TextEditor::TextEditor(bool& dirty_flag, breadbin::core::TextFile& active_file, std::optional<std::filesystem::path>& file_path, breadbin::core::ReloadManager& reload_mgr) : m_dirty(dirty_flag), m_file(active_file), m_path(file_path), m_reload_mgr(reload_mgr) {
        std::snprintf(m_name_buf, sizeof(m_name_buf), "%s", file_dir().string().c_str());
    }

    bool TextEditor::is_active() const {
        return true;
    }

    bool TextEditor::is_dirty() const {
        return m_dirty;
    }

    bool TextEditor::save() {
        if (!m_path) {
            return save_as();
        }

        if (m_file.save_to_file(*m_path)) {
            m_dirty = false;
            m_reload_mgr.notify_internal_change(*m_path);
            return true;
        }

        return false;
    }

    bool TextEditor::save_as() {
        std::filesystem::create_directories(file_dir());

        NFD::UniquePath outPath;
        nfdfilteritem_t filter[] = {
            {"Loaf files", "loaf"},
            {"Toml files", "toml"}
        };

        if (NFD::SaveDialog(outPath, filter, 2, file_dir().string().c_str()) == NFD_OKAY) {
            m_path = std::filesystem::path(outPath.get());
            return save();
        }

        return false;
    }

    void TextEditor::open_file(const std::filesystem::path& path) {
        if (m_file.load_from_file(path)) {
            m_path = path;
            m_dirty = false;
        }
    }

    void TextEditor::render(bool* p_open, uint32_t dockspace_id) {
        if (!p_open || !*p_open) {
            return;
        }

        if (m_path && m_path != m_last_loaded_path) {
            m_file.load_from_file(*m_path);
            m_dirty = false;
            m_last_loaded_path = m_path;
        }

        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("The Bread Bin - Text Editor", p_open, window_flags);

        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_AllowTabInput;

        if (ImGui::InputTextMultiline(
        "##source",
        m_file.contents.data(),
        m_file.contents.capacity() + 1,
        ImVec2(-FLT_MIN, -FLT_MIN),
        ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize,
        [](ImGuiInputTextCallbackData* data) -> int {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                auto* str = static_cast<std::string*>(data->UserData);
                str->resize(data->BufTextLen);
                data->Buf = str->data();
            }
            return 0;
        },
        &m_file.contents))
        {
            m_dirty = true;
        }


        ImGui::End();

    }

} // namespace breadbin::gui
