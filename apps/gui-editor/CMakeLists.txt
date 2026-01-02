add_executable(breadbin-gui
        main_gui.cpp
)

target_link_libraries(breadbin-gui PRIVATE
        breadbin_core
        imgui::imgui
        glfw
        glad::glad
        OpenGL::GL
)