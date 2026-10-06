# Toolchain file for cross-compiling Windows (Proton/Wine) binaries on Linux using MinGW-w64

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Provide bundled MinGW Vulkan import library if not explicitly specified
if(NOT DEFINED Vulkan_LIBRARY AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/libvulkan-1.a")
    set(Vulkan_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/libvulkan-1.a" CACHE FILEPATH "Vulkan import library")
endif()
