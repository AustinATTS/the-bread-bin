#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ThemeLoader.hpp>
#include <LoafBrowser.hpp>
#include <breadbin/LoafFile.hpp>
#include <nfd.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <map>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

static bool show_editor = true;
static bool show_browser = true;
static bool loaf_dirty = false;

static std::optional<std::filesystem::path> current_loaf_path;

static std::map<std::string, std::string> installed_apps;
static std::vector<std::string> installed_app_names;
static breadbin::core::LoafFile current_loaf;

static std::filesystem::path loafs_dir() {
    return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin" / "loafs";
}

static bool save_loaf_to_path(breadbin::core::LoafFile& loaf, const std::filesystem::path & path) {
    if (loaf.save_to_file(path)) {
        loaf_dirty = false;
        return true;
    }
    return false;
}

static void save_loaf_as(breadbin::core::LoafFile& loaf, std::optional<std::filesystem::path>& path) {
    std::filesystem::create_directories(loafs_dir());

    NFD::UniquePath outPath;
    nfdfilteritem_t filter[] = {{"Loaf files", "loaf"}};

    if (NFD::SaveDialog(outPath, filter, 1, loafs_dir().string().c_str()) == NFD_OKAY) {

        std::filesystem::path save_path = outPath.get();
        if (save_path.extension() != ".loaf") {
            save_path += ".loaf";
        }

        path = save_path;
        save_loaf_to_path(loaf, save_path);
    }
}

static void glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

std::map<std::string, std::string> get_installed_apps() {
    std::map<std::string, std::string> apps;

    std::vector<std::filesystem::path> paths = {"/usr/share/applications", std::filesystem::path(getenv("HOME")) / ".local/share/applications"};

    for (auto& dir : paths) {
        if (!std::filesystem::exists(dir)) {
            continue;
        }
        for (auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() != ".desktop") {
                continue;
            }
            std::ifstream file(entry.path());
            std::string line, name, exec;

            while (std::getline(file, line)) {
                if (line.rfind("Name=") == 0) {
                    name = line.substr(5);
                }
                else {
                    if (line.find("Exec=") == 0) {
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
                apps[name] = exec;
            }
        }
    }
    return apps;
}

void refresh_installed_apps() {
    installed_apps = get_installed_apps();
    installed_app_names.clear();
    for (auto& [name, exec] : installed_apps) {
        installed_app_names.push_back(name);
    }
}


int main() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1400, 900, "The Bread Bin", nullptr, nullptr);

    if (!window) {
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL()) {
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    breadbin::theme::ApplyTheme();

    if (NFD::Init() != NFD_OKAY) {
        return 1;
    }

    refresh_installed_apps();

    current_loaf.name = "Untitled Loaf";

    static char name_buf[128];
    std::snprintf(name_buf, sizeof(name_buf), "%s", current_loaf.name.c_str());

    breadbin::gui::LoafBrowser loaf_browser(loaf_dirty, current_loaf);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags root_flags =ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_MenuBar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("##BreadBinRoot", nullptr, root_flags);
        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("BreadBinDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0));

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load Loaf")) {
                    std::filesystem::create_directories(loafs_dir());

                    NFD::UniquePath inPath;
                    nfdfilteritem_t filter[] = {{"Loaf files", "loaf"}};

                    if (NFD::OpenDialog(inPath, filter, 1, loafs_dir().string().c_str()) == NFD_OKAY) {
                        if (current_loaf.load_from_file(inPath.get())) {
                            current_loaf_path = inPath.get();
                            std::snprintf(name_buf, sizeof(name_buf), "%s", current_loaf.name.c_str());
                            loaf_dirty = false;
                        }
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save", nullptr, false, loaf_dirty)) {
                    if (current_loaf_path) {
                        save_loaf_to_path(current_loaf, *current_loaf_path);
                    }
                    else {
                        save_loaf_as(current_loaf, current_loaf_path);
                    }
                }

                if (ImGui::MenuItem("Save As")) {
                    save_loaf_as(current_loaf, current_loaf_path);
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Editor", nullptr, &show_editor);
                ImGui::MenuItem("Loaf Browser", nullptr, &show_browser);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (show_editor) {
            ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

            ImGui::Begin("Bread Bin Editor", &show_editor);

            if (!loaf_dirty && std::string(name_buf) != current_loaf.name) {
                std::snprintf(name_buf, sizeof(name_buf), "%s", current_loaf.name.c_str());
            }

            if (ImGui::InputText("Loaf Name", name_buf, sizeof(name_buf))) {
                current_loaf.name = name_buf;
                loaf_dirty = true;
            }

            ImGui::Separator();

            if (ImGui::Button("+ Add Action")) {
                current_loaf.actions.push_back({breadbin::core::ActionType::App, "", {}});
                loaf_dirty = true;
            }

            for (size_t i = 0; i < current_loaf.actions.size(); ++i) {
            auto& action = current_loaf.actions[i];
            ImGui::PushID(static_cast<int>(i));

            const char* type_labels[] = {"App","File","Link"};
            int type_index = static_cast<int>(action.type);

            std::string header = type_labels[type_index] + std::string(": ") + (action.target.empty() ? "<unset>" : action.target);

            if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::Combo("Type", &type_index, type_labels, IM_ARRAYSIZE(type_labels))) {
                    action.type = static_cast<breadbin::core::ActionType>(type_index);
                    loaf_dirty = true;
                }

                std::string current_label = "<unset>";
                for (auto& [name, exec] : installed_apps) {
                    if (exec == action.target) {
                        current_label = name;
                        break;
                    }
                }

                if (ImGui::BeginCombo("Select App", current_label.c_str())) {
                    for (auto& name : installed_app_names) {
                        bool selected = installed_apps[name] == action.target;

                        if (ImGui::Selectable(name.c_str(), selected)) {
                            action.target = installed_apps[name];
                            loaf_dirty = true;
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
                    loaf_dirty = true;
                }

                for (size_t a = 0; a < action.args.size(); ++a) {
                    ImGui::PushID(static_cast<int>(a));

                    char arg_buf[128];
                    std::snprintf(arg_buf, sizeof(arg_buf), "%s", action.args[a].c_str());

                    if (ImGui::InputText("##arg", arg_buf, sizeof(arg_buf))) {
                        action.args[a] = arg_buf;
                        loaf_dirty = true;
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Remove")) {
                        action.args.erase(action.args.begin() + a);
                        loaf_dirty = true;
                        ImGui::PopID();
                        break;
                    }

                    ImGui::PopID();
                }

                ImGui::Separator();
                if (ImGui::Button("Remove Action")) {
                    current_loaf.actions.erase(current_loaf.actions.begin() + i);
                    loaf_dirty = true;
                    ImGui::PopID();
                    break;
                }
            }

            ImGui::PopID();
        }

            ImGui::End();
        }

        if (show_browser) {
            ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
            loaf_browser.render(&show_browser);
        }

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    NFD::Quit();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
