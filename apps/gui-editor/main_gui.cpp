#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <breadbin/LoafFile.hpp>
#include <ThemeLoader.hpp>
#include <nfd.hpp>

#include <cstdio>
#include <iostream>
#include <optional>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <map>

#include <LoafBrowser.hpp>

std::optional<std::filesystem::path> current_loaf_path;
bool show_browser = false;
bool loaf_dirty = false;
breadbin::core::LoafFile my_loaf;
breadbin::gui::LoafBrowser browser(loaf_dirty, my_loaf);

static std::map<std::string, std::string> installed_apps;
static std::vector<std::string> installed_app_names;

static std::filesystem::path loafs_dir() {
    return std::filesystem::path(getenv("HOME")) / ".config" / "the-bread-bin" / "loafs";
}

bool save_loaf_to_path(breadbin::core::LoafFile& loaf, const std::filesystem::path& path) {
    if (loaf.save_to_file(path)) {
        loaf_dirty = false;
        return true;
    }
    std::cerr << "Save failed! Cannot open file: " << path << std::endl;
    return false;
}

void save_loaf_as(breadbin::core::LoafFile& loaf, std::optional<std::filesystem::path>& path) {
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

std::map<std::string, std::string> get_installed_apps() {
    std::map<std::string, std::string> apps;

    std::vector<std::filesystem::path> paths = {
        "/usr/share/applications",
        std::filesystem::path(getenv("HOME")) / ".local/share/applications"
    };

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
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "The Bread Bin - Editor", nullptr, nullptr);

    if (!window) {
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    breadbin::theme::ApplyTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (NFD::Init() != NFD_OKAY) {
        std::cerr << "NFD init failed: " << NFD::GetError() << std::endl;
        return -1;
    }

    refresh_installed_apps();

    breadbin::core::LoafFile current_loaf;
    current_loaf.name = "Untitled Loaf";
    static char name_buf[128];
    std::snprintf(name_buf, sizeof(name_buf), "%s", current_loaf.name.c_str());

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Open Loaf Browser", "Ctrl + B")) {
                    show_browser = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_B)) {
            show_browser = !show_browser;
        }

        float menu_height = ImGui::GetFrameHeight();
        float sidebar_width = 0.0f;

        if (show_browser) {
            sidebar_width = 300.0f;

            ImGui::SetNextWindowPos(ImVec2(0, menu_height), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(sidebar_width, ImGui::GetIO().DisplaySize.y - menu_height), ImGuiCond_FirstUseEver);
            ImVec2 pos = ImGui::GetWindowPos();
            if (pos.x > 10.0f) {
                sidebar_width = 0.0f;
            }

            browser.render(&show_browser);
        }

        ImGui::SetNextWindowPos(ImVec2(sidebar_width, menu_height), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - sidebar_width, ImGui::GetIO().DisplaySize.y - menu_height), ImGuiCond_Always);

        ImGui::Begin("Bread Bin Editor", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoSavedSettings);

        if (!loaf_dirty && std::string(name_buf) != current_loaf.name) {
            std::snprintf(name_buf, sizeof(name_buf), "%s", current_loaf.name.c_str());
        }

        if (ImGui::InputText("Loaf Name", name_buf, sizeof(name_buf))) {
            current_loaf.name = name_buf;
            loaf_dirty = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Load Loaf")) {
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

            if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
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
                    for (auto& name : installed_app_names)
                    {
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

        ImGui::Separator();

        ImGui::BeginDisabled(!loaf_dirty);
        if (ImGui::Button("Save")) {
            if (current_loaf_path) {
                save_loaf_to_path(current_loaf, *current_loaf_path);
            }
            else {
                save_loaf_as(current_loaf, current_loaf_path);
            }
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Save As")) {
            save_loaf_as(current_loaf, current_loaf_path);
        }

        ImGui::End();

        ImGui::Render();
        int w,h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    NFD::Quit();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
