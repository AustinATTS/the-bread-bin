add_executable(breadbin-gui
        main_gui.cpp
)

target_link_libraries(breadbin-gui PRIVATE ${DBUS_LIBRARIES})
target_include_directories(breadbin-gui PRIVATE ${DBUS_INCLUDE_DIRS})


target_link_libraries(breadbin-gui PRIVATE
        breadbin_core
        imgui::imgui
        glfw
        glad::glad
        OpenGL::GL
        nfd::nfd
)