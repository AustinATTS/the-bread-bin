#include <LoafEditor.hpp>
#include <nfd.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace breadbin::gui {

    static std::filesystem::path loafs_dir() {
        return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin" / "loafs";
    }

    LoafEditor::LoafEditor(bool& dirty_flag, breadbin::core::LoafFile& active_loaf, std::optional<std::filesystem::path>& loaf_path, breadbin::core::ReloadManager& reload_mgr) : m_dirty(dirty_flag), m_loaf(active_loaf), m_path(loaf_path), m_reload_mgr(reload_mgr) {
        std::snprintf(m_name_buf, sizeof(m_name_buf), "%s", m_loaf.name.c_str());
    }

    bool LoafEditor::is_active() const {
        return true;
    }

    bool LoafEditor::is_dirty() const {
        return m_dirty;
    }

    bool LoafEditor::save() {
        if (!m_path) {
            return save_as();
        }

        if (m_loaf.save_to_file(*m_path)) {
            m_dirty = false;
            m_reload_mgr.notify_internal_change(*m_path);
            return true;
        }

        return false;
    }

    bool LoafEditor::save_as() {
        std::filesystem::create_directories(loafs_dir());

        NFD::UniquePath outPath;
        nfdfilteritem_t filter[] = {{"Loaf files", "loaf"}};

        if (NFD::SaveDialog(outPath, filter, 1, loafs_dir().string().c_str()) == NFD_OKAY) {
            std::filesystem::path save_path = outPath.get();
            if (save_path.extension() != ".loaf") {
                save_path += ".loaf";
            }

            m_path = save_path;
            return save();
        }

        return false;
    }

    void LoafEditor::update_apps(const std::map<std::string, std::string>& apps, const std::vector<std::string>& app_names) {
        m_installed_apps = apps;
        m_app_names = app_names;
    }

    void LoafEditor::refresh_installed_apps() {
        m_installed_apps.clear();
        m_app_names.clear();

        std::vector<std::filesystem::path> paths = {
            "/usr/share/applications",
            std::filesystem::path(std::getenv("HOME")) / ".local/share/applications"
        };

        for (const auto& dir : paths) {
            if (!std::filesystem::exists(dir)) {
                continue;
            }

            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.path().extension() != ".desktop") {
                    continue;
                }

                std::ifstream file(entry.path());
                std::string line, name, exec;

                while (std::getline(file, line)) {
                    if (line.rfind("Name=", 0) == 0) {
                        name = line.substr(5);
                    }
                    else {
                        if (line.rfind("Exec=", 0) == 0) {
                            exec = line.substr(5);
                            auto pos = exec.find(' ');
                            if (pos != std::string::npos) {
                                exec = exec.substr(0, pos);
                            }
                        }
                    }

                    if (!name.empty() && !exec.empty()) {
                        break;
                    }
                }

                if (!name.empty() && !exec.empty()) {
                    m_installed_apps[name] = exec;
                }
            }
        }

        for (const auto& [name, _] : m_installed_apps) {
            m_app_names.push_back(name);
        }
    }

    void LoafEditor::render(bool* p_open, uint32_t dockspace_id) {
        if (!p_open || !*p_open) {
            return;
        }

        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("The Bread Bin - Loaf Editor", p_open, flags);

        if (!m_dirty && std::string(m_name_buf) != m_loaf.name) {
            std::snprintf(m_name_buf, sizeof(m_name_buf), "%s", m_loaf.name.c_str());
        }

        if (ImGui::InputText("Loaf Name", m_name_buf, sizeof(m_name_buf))) {
            m_loaf.name = m_name_buf;
            m_dirty = true;
        }

        ImGui::Separator();
        render_actions();

        ImGui::End();
    }

    void LoafEditor::render_actions() {
        if (ImGui::Button("+ Add Action")) {
            m_loaf.actions.push_back({breadbin::core::ActionType::App, "", {}});
            m_dirty = true;
        }

        for (size_t i = 0; i < m_loaf.actions.size(); ++i) {
            auto& action = m_loaf.actions[i];
            ImGui::PushID(static_cast<int>(i));

            const char* type_labels[] = {"App", "File", "Link"};
            int type_index = static_cast<int>(action.type);

            std::string header = std::string(type_labels[type_index]) + ": " + (action.target.empty() ? "<unset>" : action.target);

            if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::Combo("Type", &type_index, type_labels, IM_ARRAYSIZE(type_labels))) {
                    action.type = static_cast<breadbin::core::ActionType>(type_index);
                    m_dirty = true;
                }

                std::string current_label = "<unset>";
                for (auto& [name, exec] : m_installed_apps) {
                    if (exec == action.target) {
                        current_label = name;
                        break;
                    }
                }

                if (ImGui::BeginCombo("Select App", current_label.c_str())) {
                    for (auto& name : m_app_names) {
                        bool selected = m_installed_apps[name] == action.target;
                        if (ImGui::Selectable(name.c_str(), selected)) {
                            action.target = m_installed_apps[name];
                            m_dirty = true;
                        }
                        if (selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Separator();
                ImGui::Text("Arguments");

                if (ImGui::Button("+ Add Argument")) {
                    action.args.emplace_back();
                    m_dirty = true;
                }

                for (size_t a = 0; a < action.args.size(); ++a) {
                    ImGui::PushID(static_cast<int>(a));

                    char arg_buf[128];
                    std::snprintf(arg_buf, sizeof(arg_buf), "%s", action.args[a].c_str());

                    if (ImGui::InputText("##arg", arg_buf, sizeof(arg_buf))) {
                        action.args[a] = arg_buf;
                        m_dirty = true;
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Remove")) {
                        action.args.erase(action.args.begin() + a);
                        m_dirty = true;
                        ImGui::PopID();
                        break;
                    }

                    ImGui::PopID();
                }

                ImGui::Separator();
                if (ImGui::Button("Remove Action")) {
                    m_loaf.actions.erase(m_loaf.actions.begin() + i);
                    m_dirty = true;
                    ImGui::PopID();
                    break;
                }
            }

            ImGui::PopID();
        }
    }

} // namespace breadbin::gui
