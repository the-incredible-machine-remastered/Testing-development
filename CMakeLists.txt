cmake_minimum_required(VERSION 3.14)
project(TIM_Grafica LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================
# Dependencia: Raylib (se descarga automáticamente con FetchContent)
# Usamos URL en vez de GIT_REPOSITORY para descarga más rápida.
# =============================================================
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)

# =============================================================
# Ejecutable principal
# =============================================================
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Incluir src/ para que los #include "core/vector2d.h" funcionen
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
