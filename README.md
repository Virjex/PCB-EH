# CAD GUI Application

## Overview
This project is a Computer-Aided Design (CAD) software application that utilizes OpenGL for rendering and ImGui for the graphical user interface. The application provides a user-friendly interface for creating and manipulating CAD drawings.

## Features
- Toolbar for quick access to drawing tools (e.g., line, rectangle).
- Viewport for displaying and interacting with CAD drawings.
- Rendering engine for drawing shapes and managing OpenGL settings.
- Document management for loading and saving CAD files.
- Logging utility for tracking application events and errors.

## Project Structure
```
cad-gui-app
├── src
│   ├── main.cpp               # Entry point of the application
│   ├── gui
│   │   ├── toolbar.cpp        # Implementation of the Toolbar class
│   │   ├── toolbar.h          # Declaration of the Toolbar class
│   │   ├── viewport.cpp       # Implementation of the Viewport class
│   │   └── viewport.h         # Declaration of the Viewport class
│   ├── rendering
│   │   ├── renderer.cpp       # Implementation of the Renderer class
│   │   └── renderer.h         # Declaration of the Renderer class
│   ├── core
│   │   ├── cad_document.cpp    # Implementation of the CADDocument class
│   │   └── cad_document.h      # Declaration of the CADDocument class
│   └── utils
│       └── logger.h           # Utility functions for logging
├── CMakeLists.txt             # CMake configuration file
└── README.md                   # Project documentation
```

## Setup Instructions
1. Clone the repository to your local machine.
2. Navigate to the project directory.
3. Create a build directory:
   ```
   mkdir build
   cd build
   ```
4. Run CMake to configure the project:
   ```
   cmake ..
   ```
5. Build the project:
   ```
   make
   ```
6. Run the application:
   ```
   ./cad-gui-app
   ```

## Usage Guidelines
- Use the toolbar to select drawing tools.
- Click and drag in the viewport to create shapes.
- Save your work frequently to avoid data loss.

## Contributing
Contributions are welcome! Please submit a pull request or open an issue for any enhancements or bug fixes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.