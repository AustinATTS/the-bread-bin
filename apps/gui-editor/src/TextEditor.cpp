// TextEditor.cpp
#include <TextEditor.hpp>
#include <imgui.h>
#include <nfd.hpp>
#include <cstdlib>

namespace breadbin::gui {

    static std::filesystem::path base_dir() {
        return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin";
    }

    TextEditor::TextEditor(
        bool& dirty,
        breadbin::core::TextFile& file,
        std::optional<std::filesystem::path>& path,
        breadbin::core::ReloadManager& reload_mgr
    )
        : m_dirty(dirty)
        , m_file(file)
        , m_path(path)
        , m_reload_mgr(reload_mgr) {}

    bool TextEditor::is_dirty() const {
        return m_dirty;
    }

    bool TextEditor::save() {
        if (!m_path) return save_as();

        if (m_file.save_to_file(*m_path)) {
            m_dirty = false;
            m_reload_mgr.notify_internal_change(*m_path);
            return true;
        }
        return false;
    }

    bool TextEditor::save_as() {
        std::filesystem::create_directories(base_dir());

        NFD::UniquePath out;
        nfdfilteritem_t filter[] = {
            {"Loaf files", "loaf"},
            {"Toml files", "toml"}
        };

        if (NFD::SaveDialog(out, filter, 2, base_dir().string().c_str()) == NFD_OKAY) {
            m_path = out.get();
            m_reload_mgr.notify_internal_change(*m_path);
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

    void TextEditor::reload_from_disk() {
        if (!m_path || m_dirty) return;
        m_file.load_from_file(*m_path);
        m_dirty = false;
    }

    void TextEditor::render(bool* p_open, uint32_t dockspace_id) {
        if (!p_open || !*p_open) return;

        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::Begin("Text Editor", p_open,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar);

        bool save_requested = ImGui::MenuItem("Save", "Ctrl+S") || (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_S));

        if (save_requested) {
            if (save() && m_on_saved && m_path) {
                m_on_saved(*m_path);
            }
        }

        if (ImGui::InputTextMultiline(
            "##text",
            m_file.contents.data(),
            m_file.contents.capacity() + 1,
            ImVec2(-FLT_MIN, -FLT_MIN),
            ImGuiInputTextFlags_AllowTabInput |
            ImGuiInputTextFlags_CallbackResize,
            [](ImGuiInputTextCallbackData* data) {
                auto* s = static_cast<std::string*>(data->UserData);
                s->resize(data->BufTextLen);
                data->Buf = s->data();
                return 0;
            },
            &m_file.contents))
        {
            m_dirty = true;
        }

        ImGui::End();
    }

}
