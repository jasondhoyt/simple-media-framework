cmake_minimum_required(VERSION 3.5)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(simple-media-framework)

add_subdirectory(src)
