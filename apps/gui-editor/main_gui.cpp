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
#include <TextEditor.hpp>

#include "LoafEditor.hpp"

static bool show_editor = true;
static bool show_browser = true;
static bool loaf_dirty = false;

static std::optional<std::filesystem::path> current_loaf_path;

static std::map<std::string, std::string> installed_apps;
static std::vector<std::string> installed_app_names;
static breadbin::core::LoafFile current_loaf;
static breadbin::gui::TextEditor raw_editor;

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

    if (!glfwInit()) {
        return 1;
    }

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

    breadbin::gui::LoafBrowser loaf_browser(loaf_dirty, current_loaf, raw_editor);
    breadbin::gui::LoafEditor loaf_editor(loaf_dirty, current_loaf);
    breadbin::gui::TextEditor text_editor;

    loaf_editor.update_apps(installed_apps, installed_app_names);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags root_flags = ImGuiWindowFlags_NoDocking |
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
            loaf_editor.render(&show_editor, dockspace_id);
        }

        if (show_browser) {
            ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
            loaf_browser.render(&show_browser);
        }

        if (raw_editor.is_open()) {
            ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
            bool p_open = true;
            raw_editor.render(&p_open);
            if (!p_open) {
                raw_editor.set_open(false);
            }
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
