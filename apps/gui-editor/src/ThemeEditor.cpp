#include <ThemeEditor.hpp>
#include <nfd.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <imgui.h>
#include <unordered_map>
#include <toml++/toml.h>
#include <string>

namespace breadbin::gui {

    static bool edit_float(toml::table& tbl, const char* key, float min, float max, bool& dirty) {
        float value = tbl[key].value_or(0.0f);
        if (ImGui::SliderFloat(key, &value, min, max)) {
            tbl.insert_or_assign(key, value);
            dirty = true;
            return true;
        }
        return false;
    }

    static bool edit_vec2(toml::table& tbl, const char* key, float min, float max, bool& dirty) {
        float v[2] = {0.0f, 0.0f};

        if (auto arr = tbl[key].as_array(); arr && arr->size() == 2) {
            v[0] = (*arr)[0].value_or(0.0f);
            v[1] = (*arr)[1].value_or(0.0f);
        }

        if (ImGui::SliderFloat2(key, v, min, max)) {
            tbl.insert_or_assign(key, toml::array{v[0], v[1]});
            dirty = true;
            return true;
        }
        return false;
    }

    static bool edit_color(toml::table& tbl, const char* key, bool& dirty) {
        float col[4] = {0, 0, 0, 1};

        if (auto arr = tbl[key].as_array(); arr && arr->size() == 4) {
            for (int i = 0; i < 4; ++i) {
                col[i] = (*arr)[i].value_or(col[i]);
            }
        }

        if (ImGui::ColorEdit4(key, col, ImGuiColorEditFlags_AlphaBar)) {
            tbl.insert_or_assign(key, toml::array{col[0], col[1], col[2], col[3]});
            dirty = true;
            return true;
        }
        return false;
    }

    static std::filesystem::path themes_dir() {
        return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin" / "themes";
    }

    ThemeEditor::ThemeEditor(bool& dirty_flag, breadbin::core::ThemeFile& active_theme, std::optional<std::filesystem::path>& theme_path, breadbin::core::ReloadManager& reload_mgr) : m_dirty(dirty_flag), m_theme(active_theme), m_path(theme_path), m_reload_mgr(reload_mgr) {
        std::snprintf(m_name_buf, sizeof(m_name_buf), "%s", themes_dir().c_str());
    }

    bool ThemeEditor::is_active() const {
        return true;
    }

    bool ThemeEditor::is_dirty() const {
        return m_dirty;
    }

    bool ThemeEditor::save() {
        if (!m_path) {
            return save_as();
        }

        if (m_theme.save_to_file(*m_path)) {
            m_dirty = false;
            m_reload_mgr.notify_internal_change(*m_path);
            return true;
        }

        return false;
    }

    bool ThemeEditor::save_as() {
        std::filesystem::create_directories(themes_dir());

        NFD::UniquePath outPath;
        nfdfilteritem_t filter[] = {{"Theme files", "toml"}};

        if (NFD::SaveDialog(outPath, filter, 1, themes_dir().string().c_str()) == NFD_OKAY) {
            std::filesystem::path save_path = outPath.get();
            if (save_path.extension() != ".toml") {
                save_path += ".toml";
            }

            m_path = save_path;
            return save();
        }

        return false;
    }

    void ThemeEditor::render(bool* p_open, uint32_t dockspace_id) {
        if (!p_open || !*p_open) {
            return;
        }

        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("The Bread Bin - Theme Editor", p_open, flags);

        bool save_requested = ImGui::MenuItem("Save", "Ctrl+S") || (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_S));

        if (save_requested) {
            if (save() && m_on_saved && m_path) {
                m_on_saved(*m_path);
            }
        }

        if (!m_dirty && std::string(m_name_buf) != m_theme.name) {
            std::snprintf(m_name_buf, sizeof(m_name_buf), "%s", m_theme.name.c_str());
        }

        if (ImGui::InputText("Theme Name", m_name_buf, sizeof(m_name_buf))) {
            m_theme.name = m_name_buf;
            m_dirty = true;
        }

        ImGui::Separator();

        auto& theme_tbl = m_theme.theme;

        if (!theme_tbl.contains("style")) {
            theme_tbl.insert("style", toml::table{});
        }

        if (!theme_tbl.contains("colors")) {
            theme_tbl.insert("colors", toml::table{});
        }

        auto* style_tbl  = theme_tbl["style"].as_table();
        auto* colors_tbl = theme_tbl["colors"].as_table();

        if (ImGui::CollapsingHeader("Style", ImGuiTreeNodeFlags_DefaultOpen)) {
            edit_float(*style_tbl, "FrameRounding", 0.0f, 20.0f, m_dirty);
            edit_float(*style_tbl, "WindowRounding", 0.0f, 20.0f, m_dirty);
            edit_float(*style_tbl, "GrabRounding", 0.0f, 20.0f, m_dirty);
            edit_float(*style_tbl, "ScrollbarRounding", 0.0f, 20.0f, m_dirty);

            edit_vec2(*style_tbl, "WindowPadding", 0.0f, 30.0f, m_dirty);
            edit_vec2(*style_tbl, "FramePadding", 0.0f, 30.0f, m_dirty);
            edit_vec2(*style_tbl, "ItemSpacing", 0.0f, 30.0f, m_dirty);
        }

        if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& [key, value] : *colors_tbl) {
                if (value.is_array() && value.as_array()->size() == 4) {
                    edit_color(*colors_tbl, key.str().data(), m_dirty);
                }
            }
        }

        ImGui::End();
    }
} // namespace breadbin::gui