#!/bin/bash
set -e

echo "=== Building WASM with Docker ==="

sudo docker run --rm \
    -v $(pwd):/build \
    -w /build \
    emscripten/emsdk:4.0.14-updated \
    bash -c "
        rm -rf webassembly/build && \
        emcmake cmake -GNinja -S. -Bwebassembly/build \
            -DCMAKE_BUILD_TYPE=Release \
            -DVTK_DIR=/build/webassembly/dependencies/VTK/Release/lib/cmake/vtk && \
        cd webassembly/build && ninja
    "

echo "=== Copying WASM files to public/wasm ==="
mkdir -p public/wasm
cp webassembly/build/VTK-Workbench.wasm public/wasm/
cp webassembly/build/VTK-Workbench.js public/wasm/
cp webassembly/build/VTK-Workbench.data public/wasm/

echo "=== WASM build complete ==="
ls -la public/wasm/
