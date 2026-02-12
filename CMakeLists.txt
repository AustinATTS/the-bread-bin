cmake_minimum_required(VERSION 3.16)
project(BreadBin VERSION 0.1 LANGUAGES CXX)

# Set C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Include directories.
include_directories(${PROJECT_SOURCE_DIR}/include)

# Core library sources (non-GUI).
set(CORE_SOURCES
    src/Loaf.cc
    src/LoafItem.cc
    src/LoafEditor.cc
    src/TextEditor.cc
    src/ThemeEditor.cc
    src/AppDiscovery.cc
)

# GUI sources.
set(GUI_SOURCES
    src/gui/MainWindow.cc
    src/gui/HomeWidget.cc
    src/gui/LoafEditorWidget.cc
    src/gui/TextEditorWidget.cc
    src/gui/ThemeEditorWidget.cc
    src/gui/LoafRuntimeWidget.cc
    src/gui/AppBrowserWidget.cc
    src/gui/LoafBrowserWidget.cc
    src/gui/ThemeBrowserWidget.cc
    src/main_gui.cc
)

# GUI headers (needed for MOC).
set(GUI_HEADERS
    include/gui/MainWindow.h
    include/gui/HomeWidget.h
    include/gui/LoafEditorWidget.h
    include/gui/TextEditorWidget.h
    include/gui/ThemeEditorWidget.h
    include/gui/LoafRuntimeWidget.h
    include/gui/AppBrowserWidget.h
    include/gui/LoafBrowserWidget.h
    include/gui/ThemeBrowserWidget.h
)

# Create executable.
add_executable(breadbin ${CORE_SOURCES} ${GUI_SOURCES} ${GUI_HEADERS})

# Link Qt6.
target_link_libraries(breadbin Qt6::Core Qt6::Widgets Qt6::Gui)

# Platform specific settings.
if(WIN32)
    # Windows-specific settings.
    target_compile_definitions(breadbin PRIVATE _WIN32)
    set_target_properties(breadbin PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings.
    target_compile_definitions(breadbin PRIVATE __linux__)
elseif(APPLE)
    # macOS-specific settings.
    target_compile_definitions(breadbin PRIVATE __APPLE__)
    set_target_properties(breadbin PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# Installation.
install(TARGETS breadbin DESTINATION bin)

# Enable testing.
enable_testing()

# Add tests subdirectory if it exists and contains CMakeLists.txt
if(EXISTS ${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()