#include <LoafEditor.hpp>
#include <imgui.h>

namespace breadbin::gui {
    LoafEditor::LoafEditor(bool& dirty_flag, breadbin::core::LoafFile& active_loaf) : m_dirty(dirty_flag), m_loaf(active_loaf) {
        std::snprintf(m_name_buf, sizeof(m_name_buf), "%s", m_loaf.name.c_str());
    }

    void LoafEditor::update_apps(const std::map<std::string, std::string>& apps, const std::vector<std::string>& app_names) {
        m_installed_apps = apps;
        m_app_names = app_names;
    }

    void LoafEditor::render(bool* p_open, uint32_t dockspace_id) {
        if (!p_open) {
            return;
        }

        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("Bread Bin Editor", p_open, flags);

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

                const char* type_labels[] = {"App","File","Link"};
                int type_index = static_cast<int>(action.type);

                std::string header = type_labels[type_index] + std::string(": ") + (action.target.empty() ? "<unset>" : action.target);

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
