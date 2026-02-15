# The Bread Bin - Cross Platform Loaf Management System

A cross platform Qt6 GUI application for organising, editing, and managine LOAFs (Libraries of Applications and Files). The Bread Bin provides a structured management of applications, files, configurations, and scripts with a modern, responsive interface designed for both desktop use and planned mobile control.

## Features

### Core Functionality

- Loaf Management: Create, edit, and organise structured libraries of items
- Multi Item Support: Manage applications, files, configurations, and scripts in one place
- Runtime Rules: Define behaviour and execution order for your loafs
- Custom Layouts: Organise your items with flexible layout options

### Built in Editors

- Loaf Editor: Create and modify loafs with an intuitive tabbed interface
- Text Editor: Multi tab text editor for files and scripts with find/replace
- Theme Editor: Visual theme customisation with colour pickers and font selection
- Runtime Manager: Control and monitor running loafs in real time
- App Browser: Automatically discover and browse installed applications
- Loaf Browser: View and manage all your loaf files in one place
- Theme Browser: Preview and quickly switch between themes

### Cross Platform Support

- Linux: Full native support
- Windows: Windows API integration
- macOS: Native macOS support (somewhat)
- Android: Planned for future releases

## Architecture

The Bread Bin is written in C++ using Qt6 with class based desgin and principles:

- Object oriented architecture with clear seperation of concerns
- Abstract base classes for extensibility
- Smart pointers for safe memory management
- Platform specific code abstractions

See [ARCHITECTURE.md](docs/ARCHITECTURE.md) for detailed design documentation.

## Building

### Prerequisites

- C++ compiler with C++26 support
- CMake 3.16 or higher
- Qt6 (Core, Widgets, Gui modules)

### Build Instructions

#### Linux/macOS

```bash
# Install Qt6 (Ubuntu/Debian)
sudo apt-get install qt6-base-dev qt6-tools-dev

# Build
mkdir build
cd build
cmake ..
make
```

#### Windows

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Installation

```bash
# After building
sudo make install # Linux/macOS
```

## Usage

### Graphial User Interface

```bash
# Run The Bread Bin GUI
./breadbin

# The application will open with a warm, toast themed interface featuring:
# - A loaf editor
# - A text editor
# - A theme editor
# - A runtime manager
# - An app browser
# - A loaf browser
# - A theme browser
```

## Contributing

Contributions are welcome! Please ensure:

- Code follows the existing class based design patterns
- Cross platform compatibility is maintained
- Documentation is updated for new features

## License

See [LICENSE](LICENSE) file for details.

## Contact

For questions, issues, or contributions, please use the GitHub repository.