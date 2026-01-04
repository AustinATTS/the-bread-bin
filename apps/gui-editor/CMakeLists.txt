add_executable(breadbin-gui
        main_gui.cpp
        src/ThemeLoader.cpp
        src/LoafBrowser.cpp
        src/TextEditor.cpp
)

target_include_directories(breadbin-gui PRIVATE ${DBUS_INCLUDE_DIRS})
target_include_directories(breadbin-gui PRIVATE include)

target_link_libraries(breadbin-gui PRIVATE ${DBUS_LIBRARIES})
target_link_libraries(breadbin-gui PRIVATE
        breadbin_core
        imgui::imgui
        glfw
        glad::glad
        OpenGL::GL
        nfd::nfd
)