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
#include <iostream>

static bool show_loaf_editor  = true;
static bool show_browser = true;
static bool show_text_editor = false;
static bool show_theme_editor = false;

static bool loaf_dirty   = false;
static bool file_dirty = false;
static bool theme_dirty = false;

static std::optional<std::filesystem::path> current_loaf_path;
static std::optional<std::filesystem::path> current_file_path;

static breadbin::core::LoafFile current_loaf;
static breadbin::core::TextFile current_file;


static std::filesystem::path loafs_dir() {
    return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin" / "loafs";
}

static std::filesystem::path theme_dir() {
    return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin" / "themes";
}

static std::filesystem::path main_dir() {
    return std::filesystem::path(std::getenv("HOME")) / ".config" / "the-bread-bin";
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    breadbin::theme::LoadPersistedTheme();

    if (NFD::Init() != NFD_OKAY) {
        return 1;
    }

    current_loaf.name = "Untitled Loaf";

    breadbin::core::ReloadManager reload_mgr;

    breadbin::gui::TextEditor text_editor(file_dirty, current_file, current_file_path, reload_mgr);
    breadbin::gui::LoafEditor loaf_editor(loaf_dirty, current_loaf, current_loaf_path, reload_mgr);
    breadbin::gui::LoafBrowser loaf_browser(loaf_dirty, current_loaf, text_editor, current_loaf_path);

    loaf_editor.refresh_installed_apps();

    reload_mgr.watch_directory(loafs_dir(), [&]() {
        loaf_browser.reload();
    });

    reload_mgr.watch_directory(theme_dir(), [&]() {
        loaf_browser.reload();
    });

    static std::optional<std::filesystem::path> last_loaf_watch;
    static std::optional<std::filesystem::path> last_file_watch;


    auto on_file_changed = [&](const std::filesystem::path& path) {

        if (current_loaf_path && *current_loaf_path == path && !loaf_dirty) {
            current_loaf.load_from_file(path);
        }

        if (current_file_path && *current_file_path == path && !file_dirty) {
            text_editor.reload_from_disk();
        }
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        reload_mgr.update();

        if (current_loaf_path != last_loaf_watch) {
            if (last_loaf_watch) reload_mgr.unwatch_file(*last_loaf_watch);
            if (current_loaf_path)
                reload_mgr.watch_file(*current_loaf_path, [&]() {
                    on_file_changed(*current_loaf_path);
                });
            last_loaf_watch = current_loaf_path;
        }

        // Watch text file
        if (current_file_path != last_file_watch) {
            if (last_file_watch) reload_mgr.unwatch_file(*last_file_watch);
            if (current_file_path)
                reload_mgr.watch_file(*current_file_path, [&]() {
                    on_file_changed(*current_file_path);
                });
            last_file_watch = current_file_path;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool reload_requested = ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_R);

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGui::Begin("##BreadBinRoot", nullptr, ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_MenuBar
        );

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

                if (ImGui::MenuItem("Load Text File")) {
                    NFD::UniquePath inPath;
                    nfdfilteritem_t filter[] = {
                        {"Loaf files", "loaf"},
                        {"Toml files", "toml"}
                    };
                    if (NFD::OpenDialog(inPath, filter, 2, loafs_dir().string().c_str()) == NFD_OKAY) {
                        current_file_path = inPath.get();
                        text_editor.save();
                        show_text_editor = true;
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Reload", "Ctrl+R")) {
                    reload_requested = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    loaf_editor.save();
                    text_editor.save();
                }

                if (ImGui::MenuItem("Save As")) {
                    loaf_editor.save_as();
                    text_editor.save_as();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Loaf Editor", "Ctrl+L", &show_loaf_editor);
                ImGui::MenuItem("Text Editor", "Ctrl+O", &show_text_editor);
                ImGui::MenuItem("Theme Editor", "Ctrl+A", &show_theme_editor);
                ImGui::MenuItem("Loaf Browser", "Ctrl+F", &show_browser);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (reload_requested) {
            loaf_browser.reload();
            loaf_editor.refresh_installed_apps();

            if (current_loaf_path && !loaf_dirty)
                current_loaf.load_from_file(*current_loaf_path);

            if (current_file_path && !file_dirty)
                current_file.load_from_file(*current_file_path);
        }

        if (show_loaf_editor) {
            loaf_editor.render(&show_loaf_editor, dockspace_id);
        }

        if (show_browser) {
            ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
            loaf_browser.render(&show_browser);
        }

        if (show_text_editor) {
            ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
            text_editor.render(&show_text_editor, dockspace_id);
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
