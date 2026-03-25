# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

VTK Workbench is a Next.js application that integrates VTK (Visualization Toolkit) via WebAssembly for 3D visualization. The project combines a React frontend with a C++ VTK backend compiled to WebAssembly, enabling high-performance 3D rendering in the browser.

## Development Commands

### Next.js Development
- `npm run dev` - Start development server with Turbopack
- `npm run build` - Build Next.js application only
- `npm run start` - Start production server
- `npm run lint` - Run ESLint
- `npm run type-check` - Run TypeScript type checking

### WebAssembly Build Commands
- `npm run build-wasm:debug` - Build VTK WebAssembly module in debug mode
- `npm run build-wasm:release` - Build VTK WebAssembly module in release mode
- `npm run build:full` - Build WebAssembly + Next.js application
- `npm run build:standalone` - Build WebAssembly + Next.js + export for standalone deployment
- `npm run build:docker` - Build Docker image
- `npm run build:production` - Clean + build for production
- `npm run clean` - Clean all build artifacts
- `npm run rm-build` - Remove WebAssembly build directory
- `npm run rm-wasm` - Remove compiled WASM files from public/wasm
- `npm run health` - Simple health check

### Testing Commands
- `@playwright/test` is available as a dependency but no test scripts are configured
- No primary test framework is currently set up in package.json

## Architecture

### Frontend (Next.js)
- **App Router**: Uses Next.js 15 with app directory structure
- **Main Pages**:
  - `/` - Landing page (`app/page.tsx`)
  - `/workbench` - Main VTK workbench interface (`app/workbench/page.tsx`)
- **API Routes**: Health check endpoint at `/api/health`
- **UI Components**: Uses shadcn/ui with Radix UI primitives and Tailwind CSS

### WebAssembly Backend
- **Location**: `webassembly/src/` contains C++ source code
- **Build System**: CMake with Emscripten toolchain
- **Dependencies**:
  - VTK (kitware/vtk-wasm-sdk:wasm64-threads-v9.4.1)
  - OCCT 7.9.0 (OpenCASCADE)
  - ImGui 1.91.8 for UI
  - spdlog for logging
  - stb_image for image handling

### Key Architecture Components

#### VTK Integration
- WebAssembly module compiled from C++ VTK code
- Global `createViewer` function initializes VTK module
- Module instance stored in `window.VtkModule`
- Canvas-based rendering with WebGL2 support
- IDBFS integration for file system operations

#### WebAssembly Module Structure
- **Core**: `app.cpp`, `vtk_viewer.cpp` - Main application logic
- **UI**: `custom_ui.cpp`, `toolbar.cpp` - ImGui interface
- **Mesh Handling**: `mesh.cpp`, `mesh_manager.cpp` - 3D model management
- **Atoms System**: `webassembly/src/atoms/` - Molecular visualization
- **File I/O**: `file_loader.cpp`, `unv_reader.cpp` - File format support

#### Memory and Threading
- 64-bit WASM memory model (`-sMEMORY64=1`)
- Multi-threading with pthread support
- Initial memory: 256MB, maximum: 16GB
- Shared memory for multi-threading

## Build Dependencies

### System Requirements
- emsdk 4.0.3 (for WebAssembly compilation)
- CMake 3.13+ with Ninja build system
- Node.js 22+ (for Next.js)

### External Dependencies
- VTK WASM SDK (precompiled in `webassembly/dependencies/VTK/`)
- OCCT 7.9.0 (precompiled in `webassembly/dependencies/OCCT/`)
- ImGui 1.91.8 (source in `webassembly/dependencies/imgui-1.91.8-docking/`)

## File Structure

```
vtk-workbench/
├── app/                    # Next.js app directory
│   ├── page.tsx           # Landing page
│   ├── workbench/         # Main workbench application
│   └── api/health/        # Health check API
├── webassembly/           # C++ VTK source code
│   ├── src/              # Main C++ source files
│   ├── dependencies/     # VTK, OCCT, ImGui libraries
│   └── build/            # CMake build output
├── public/wasm/          # Compiled WebAssembly files
├── components/ui/        # React UI components (shadcn/ui)
├── types/vtk.d.ts       # TypeScript definitions for VTK module
└── CMakeLists.txt       # Main CMake configuration
```

## Configuration Files

- **TypeScript**: Path mapping `@/*` points to project root
- **Tailwind**: Configured with shadcn/ui design system
- **ESLint**: Uses Next.js recommended config with TypeScript support
- **Next.js**:
  - Standalone output for Docker deployment
  - WebAssembly-specific headers (COOP, COEP, CORP) for SharedArrayBuffer support
  - Optimized caching for WASM files (1 year cache for `/wasm/*` assets)
  - No-cache policy for main application to ensure fresh WebAssembly modules

## Development Workflow

1. **WebAssembly Development**:
   - Modify C++ code in `webassembly/src/`
   - Run `npm run build-wasm:debug` for faster iteration
   - Use `npm run build-wasm:release` for production builds

2. **Frontend Development**:
   - Modify React components in `app/` or `components/`
   - Run `npm run dev` for hot reload
   - WebAssembly files are served from `public/wasm/`

3. **Full Build**:
   - Use `npm run build:full` to compile both WebAssembly and Next.js
   - Required before production deployment

## Deployment

### Docker Deployment
The project includes Docker support with multi-stage builds:
- Stage 1: Emscripten environment for WebAssembly compilation
- Stage 2: Node.js environment for Next.js build
- Stage 3: Production runtime with Alpine Linux

Use `npm run build:docker` to build the Docker image locally.

### Kubernetes Deployment
Kubernetes deployment configurations are available in `k8s/` directory:
- **Target namespace**: `pt`
- **Registry**: `10.183.3.3:5000/vtk-workbench`
- **Ingress endpoint**: `https://crystal-test.edison.re.kr/`
- **Resource limits**: CPU 250m-500m, Memory 512Mi-1Gi
- **Host volume mount**: `/mnt/luck/service/crystal-editor/public` → `/app/public`
- **Health check**: Currently disabled for simplified deployment

Deployment commands:
```bash
# Build and push image
docker build -t vtk-workbench:latest .
docker tag vtk-workbench:latest 10.183.3.3:5000/vtk-workbench:latest
docker push 10.183.3.3:5000/vtk-workbench:latest

# Deploy to Kubernetes
kubectl apply -f k8s/vtk-workbench.yaml

# Monitor deployment
kubectl rollout status deployment/vtk-workbench -n pt
kubectl logs -f deployment/vtk-workbench -n pt
```

## WebAssembly Integration Notes

- WASM files must be built before running Next.js application
- Module initialization is asynchronous via `createViewer()` function
- Canvas resizing and WebGL context management handled in workbench page
- File system operations use Emscripten's IDBFS for browser storage