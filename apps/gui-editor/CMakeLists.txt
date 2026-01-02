add_executable(breadbin-gui
        main_gui.cpp
)

target_link_libraries(breadbin-gui PRIVATE
        breadbin_core
        glfw
        OpenGL::GL)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(breadbing-gui
        main_gui.cpp
)