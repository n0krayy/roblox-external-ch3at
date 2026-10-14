cmake_minimum_required(VERSION 3.20)
project(rbx_external CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui must be cloned into thirdparty/imgui (see README build steps).
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/thirdparty/imgui)
if(NOT EXISTS ${IMGUI_DIR}/imgui.cpp)
    message(FATAL_ERROR "imgui not found at ${IMGUI_DIR} -- see README build steps")
endif()

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

add_executable(rbx_external WIN32
    src/main.cpp
    src/Memory.cpp
    src/Overlay.cpp
    src/Menu.cpp
    src/ESP.cpp
    src/Aimbot.cpp
    src/Movement.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(rbx_external PRIVATE
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_compile_definitions(rbx_external PRIVATE UNICODE _UNICODE)
