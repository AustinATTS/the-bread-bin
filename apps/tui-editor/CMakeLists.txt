add_executable(breadbin-tui
        main_tui.cpp
)

target_link_libraries(breadbin-tui PRIVATE
        breadbin_core
)