add_library(breadbin_core STATIC
        src/LoafFile.cpp
        src/LoafRunner.cpp
        src/ReloadManager.cpp
)

target_include_directories(breadbin_core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(breadbin_core PUBLIC
        yaml-cpp::yaml-cpp
)