#!/usr/bin/env bash
set -e

BUILD_TYPE="${BUILD_TYPE:-Release}"

print_usage() {
    echo "Usage: ./build.sh [native|proton|test|clean]"
    echo ""
    echo "  native  : Build Linux native shared libraries (.so)."
    echo "  proton  : Cross-compile Windows PE libraries (.dll) for Proton/Wine using MinGW."
    echo "  test    : Run Vulkan hardware integration tests."
    echo "  clean   : Clean build directories."
    echo ""
}

case "$1" in
    native|"")
        echo "==> Building Linux Native (.so) [Build Type: ${BUILD_TYPE}]..."
        cmake -B build-native -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
        cmake --build build-native -j"$(nproc)"
        echo ""
        echo "==> Native build completed successfully!"
        echo "    Outputs: build-native/src/libdlssg_vulkan_*.so"
        ;;
    proton)
        if ! command -v x86_64-w64-mingw32-g++ &>/dev/null; then
            echo "ERROR: MinGW compiler (x86_64-w64-mingw32-g++) not found."
            echo "To install on Arch Linux / CachyOS:"
            echo "  sudo pacman -S mingw-w64-gcc"
            exit 1
        fi
        echo "==> Preparing Vulkan headers for MinGW cross-compilation..."
        mkdir -p .deps/include
        ln -sfn /usr/include/vulkan .deps/include/vulkan
        if [ -d /usr/include/vk_video ]; then
            ln -sfn /usr/include/vk_video .deps/include/vk_video
        fi

        VULKAN_LIB_ARG=""
        if [ -f "/usr/lib/wine/x86_64-windows/libvulkan-1.a" ]; then
            VULKAN_LIB_ARG="-DVulkan_LIBRARY=/usr/lib/wine/x86_64-windows/libvulkan-1.a"
        fi

        echo "==> Cross-compiling for Proton / Windows (.dll) via MinGW..."
        cmake -B build-proton \
            -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake \
            -DVulkan_INCLUDE_DIR="$(pwd)/.deps/include" \
            ${VULKAN_LIB_ARG} \
            -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            -DDLSSG_BUILD_TESTS=OFF
        cmake --build build-proton -j"$(nproc)"
        echo ""
        echo "==> Proton build completed successfully!"
        echo "    Outputs: build-proton/src/dlssg_vulkan_*.dll"
        ;;
    test)
        echo "==> Running integration tests on active Vulkan GPU..."
        if [ ! -d "build-native" ]; then
            cmake -B build-native -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
        fi
        cmake --build build-native --target test_vulkan_route -j"$(nproc)"
        ./build-native/test_vulkan_route
        ;;
    clean)
        echo "==> Cleaning build directories..."
        rm -rf build build-native build-proton .deps
        echo "Cleaned."
        ;;
    *)
        print_usage
        exit 1
        ;;
esac
