#include <breadbin/LoafFile.hpp>
#include <breadbin/ReloadManager.hpp>
#include <ThemeLoader.hpp>
#include <LoafEditor.hpp>
#include <LoafBrowser.hpp>
#include <TextEditor.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
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
static breadbin::core::LoafFile current_loaf;
static breadbin::gui::TextEditor raw_editor;

static std::map<std::string, std::string> installed_apps;
static std::vector<std::string> installed_app_names;

static std::filesystem::path loafs_dir() {
    return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin" / "loafs";
}

static std::map<std::string, std::string> get_installed_apps() {
    std::map<std::string, std::string> apps;
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
                apps[name] = exec;
            }
        }
    }
    return apps;
}

static void refresh_installed_apps() {
    installed_apps = get_installed_apps();
    installed_app_names.clear();
    for (auto const& [name, _] : installed_apps) {
        installed_app_names.push_back(name);
    }
}

static bool save_loaf_to_path(breadbin::core::LoafFile& loaf, const std::filesystem::path& path, breadbin::core::ReloadManager& rm) {
    if (loaf.save_to_file(path)) {
        loaf_dirty = false;
        rm.notify_internal_change(path);
        if (raw_editor.is_open() && raw_editor.get_current_path() == path) {
            raw_editor.open_file(path);
        }
        return true;
    }
    return false;
}

static void save_loaf_as(breadbin::core::LoafFile& loaf, std::optional<std::filesystem::path>& path, breadbin::core::ReloadManager& rm) {
    std::filesystem::create_directories(loafs_dir());
    NFD::UniquePath outPath;
    nfdfilteritem_t filter[] = {{"Loaf files", "loaf"}};

    if (NFD::SaveDialog(outPath, filter, 1, loafs_dir().string().c_str()) == NFD_OKAY) {
        std::filesystem::path save_path = outPath.get();
        if (save_path.extension() != ".loaf") {
            save_path += ".loaf";
        }
        path = save_path;
        save_loaf_to_path(loaf, save_path, rm);
    }
}

static void glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    breadbin::theme::LoadPersistedTheme();
    if (NFD::Init() != NFD_OKAY) {
        return 1;
    }

    refresh_installed_apps();
    current_loaf.name = "Untitled Loaf";

    breadbin::gui::LoafBrowser loaf_browser(loaf_dirty, current_loaf, raw_editor, current_loaf_path);
    breadbin::gui::LoafEditor loaf_editor(loaf_dirty, current_loaf);
    breadbin::core::ReloadManager reload_mgr;

    loaf_editor.update_apps(installed_apps, installed_app_names);

    reload_mgr.watch_directory(loafs_dir(), [&]() {
        loaf_browser.reload();
    });

    auto on_file_changed = [&](const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            if (current_loaf_path && *current_loaf_path == path) {
                current_loaf = breadbin::core::LoafFile();
                current_loaf_path = std::nullopt;
                loaf_dirty = false;
            }
            if (raw_editor.is_open() && raw_editor.get_current_path() == path) {
                raw_editor.set_open(false);
            }
        } else {
            if (current_loaf_path && *current_loaf_path == path && !loaf_dirty) {
                current_loaf.load_from_file(path);
            }
            if (raw_editor.is_open() && raw_editor.get_current_path() == path && !raw_editor.is_dirty()) {
                raw_editor.open_file(path);
            }
            if (path.filename() == "theme.toml") {
                breadbin::theme::LoadPersistedTheme();
            }
        }
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        reload_mgr.update();

        static std::optional<std::filesystem::path> last_watched_path;
        if (current_loaf_path != last_watched_path) {
            if (last_watched_path) {
                reload_mgr.unwatch_file(*last_watched_path);
            }
            if (current_loaf_path) {
                reload_mgr.watch_file(*current_loaf_path, [&]() {
                    on_file_changed(*current_loaf_path);
                });
            }
            last_watched_path = current_loaf_path;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool last_raw_dirty = false;
        if (last_raw_dirty && !raw_editor.is_dirty()) {
             reload_mgr.notify_internal_change(raw_editor.get_current_path());
             if (current_loaf_path && *current_loaf_path == raw_editor.get_current_path()) {
                 current_loaf.load_from_file(*current_loaf_path);
             }
        }
        last_raw_dirty = raw_editor.is_dirty();

        bool reload_requested = ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_R);

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::Begin("##BreadBinRoot", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

        ImGuiID dockspace_id = ImGui::GetID("BreadBinDockSpace");
        ImGui::DockSpace(dockspace_id);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load Loaf")) {
                    NFD::UniquePath inPath;
                    nfdfilteritem_t filter[] = {{"Loaf files", "loaf"}};
                    if (NFD::OpenDialog(inPath, filter, 1, loafs_dir().string().c_str()) == NFD_OKAY) {
                        if (current_loaf.load_from_file(inPath.get())) {
                            current_loaf_path = inPath.get();
                            loaf_dirty = false;
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reload", "Ctrl+R")) {
                    reload_requested = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Save", "Ctrl+S", false, loaf_dirty || current_loaf_path)) {
                    if (current_loaf_path) {
                        save_loaf_to_path(current_loaf, *current_loaf_path, reload_mgr);
                    }
                    else {
                        save_loaf_as(current_loaf, current_loaf_path, reload_mgr);
                    }
                }
                if (ImGui::MenuItem("Save As")) {
                    save_loaf_as(current_loaf, current_loaf_path, reload_mgr);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Visual Editor", nullptr, &show_editor);
                ImGui::MenuItem("Loaf Browser", nullptr, &show_browser);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();

        if (reload_requested) {
            loaf_browser.reload();
            refresh_installed_apps();
            loaf_editor.update_apps(installed_apps, installed_app_names);
            if (current_loaf_path && !loaf_dirty) {
                current_loaf.load_from_file(*current_loaf_path);
            }
        }

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