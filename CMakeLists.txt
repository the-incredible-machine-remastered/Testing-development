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
# Catálogo del menú lateral (auto desde TIM_MENU_SPAWN en src/objetos/)
# =============================================================
set(CATALOGO_MENU_GEN "${CMAKE_CURRENT_SOURCE_DIR}/src/objetos/catalogo_menu.gen.h")
find_package(Python3 COMPONENTS Interpreter QUIET)
file(GLOB OBJETO_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/objetos/*.h")
# El .gen.h es OUTPUT del comando; no puede ser DEPEND de sí mismo
list(FILTER OBJETO_HEADERS EXCLUDE REGEX ".*catalogo_menu\\.gen\\.h$")

if(Python3_FOUND)
    add_custom_command(
        OUTPUT "${CATALOGO_MENU_GEN}"
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/generar_catalogo_menu.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/generar_catalogo_menu.py"
            ${OBJETO_HEADERS}
        COMMENT "Generando catalogo_menu.gen.h"
    )
    add_custom_target(catalogo_menu_gen DEPENDS "${CATALOGO_MENU_GEN}")
endif()

# =============================================================
# Ejecutable principal
# =============================================================
add_executable(${PROJECT_NAME} main.cpp)

if(Python3_FOUND)
    add_dependencies(${PROJECT_NAME} catalogo_menu_gen)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Incluir src/ para que los #include "core/vector2d.h" funcionen
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Copiar Assets y fuentes junto al ejecutable (Visual Studio / out/build)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts"
    COMMENT "Copiando Assets y fonts al directorio del ejecutable"
)
