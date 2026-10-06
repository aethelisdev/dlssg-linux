cmake_minimum_required(VERSION 3.20)
project(dlssg_vulkan_support LANGUAGES CXX)

add_library(renderer_backends STATIC
    src/renderer/renderer.cpp
    src/renderer/vulkan_renderer.cpp
    src/renderer/dx9_renderer.cpp)
target_include_directories(renderer_backends PUBLIC include)
target_compile_features(renderer_backends PUBLIC cxx_std_17)
find_package(Vulkan REQUIRED)
if(WIN32)
    target_link_libraries(renderer_backends PUBLIC Vulkan::Vulkan d3d9)
else()
    target_link_libraries(renderer_backends PUBLIC Vulkan::Vulkan)
endif()

option(DLSSG_BUILD_VULKAN_SUPPORT "Build the Vulkan bridge library" ON)
if(DLSSG_BUILD_VULKAN_SUPPORT)
    add_subdirectory(src)
endif()

option(DLSSG_BUILD_TESTS "Build integration tests" ON)
if(DLSSG_BUILD_TESTS AND DLSSG_BUILD_VULKAN_SUPPORT AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_vulkan_route.cpp")
    enable_testing()
    add_executable(test_vulkan_route tests/test_vulkan_route.cpp)
    target_include_directories(test_vulkan_route PRIVATE src)
    target_compile_features(test_vulkan_route PRIVATE cxx_std_17)
    target_link_libraries(test_vulkan_route PRIVATE dlssg_vulkan_route Vulkan::Vulkan)
    add_test(NAME test_vulkan_route COMMAND test_vulkan_route)
endif()

