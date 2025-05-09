cmake_minimum_required(VERSION 3.10)
project(native_exif_windows)

set(CMAKE_CXX_STANDARD 17)

find_package(Flutter REQUIRED)
find_package(WIC REQUIRED)

add_library(native_exif_windows SHARED
    "native_exif_windows.cpp"
    "native_exif_plugin.cpp"
)

target_link_libraries(native_exif_windows PRIVATE
    flutter
    WindowsCodecs.lib
)

target_include_directories(native_exif_windows PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(native_exif_windows PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)