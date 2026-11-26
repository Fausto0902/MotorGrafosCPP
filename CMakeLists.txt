cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# ==============================================================================
# PROYECTO: GraphEngine - Motor Modular de Análisis de Redes y Grafos
# AUTOR: AGENTE PROGRAMADOR SENIOR
# VERSIÓN: 1.0.0
# ESTÁNDAR: C++17
# ==============================================================================

project(GraphEngine
    VERSION 1.0.0
    DESCRIPTION "Motor de grafos de alto rendimiento con algoritmos de flujo, caminos mínimos y serialización JSON"
    LANGUAGES CXX
)

# Configuración del estándar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configuración robusta de flags de compilación para MSVC vs GCC/MinGW
if(MSVC)
    # Flags estrictos para Microsoft Visual C++
    add_compile_options(
        /W4             # Nivel de advertencia 4
        /WX             # Tratar advertencias como errores
        /permissive-    # Conformidad estricta con el estándar
        /Zc:__cplusplus # Reportar correctamente el estándar C++ soportado
        /EHsc           # Modelo de excepciones estándar
    )
else()
    # Flags estrictos para GCC y Clang (compatible con MinGW en Windows)
    add_compile_options(
        -Wall           # Advertencias base
        -Wextra         # Advertencias adicionales
        -Wpedantic      # Conformidad estricta con ISO C++
        -Werror         # Tratar advertencias como errores
        -Wshadow        # Advertir si una variable local oculta otra
        -Wnon-virtual-dtor # Advertir si una clase con funciones virtuales no tiene destructor virtual
        -Wold-style-cast # Advertir sobre casts estilo C
        -Wcast-align    # Advertir sobre alineación de punteros en casts
        -Wunused        # Advertir sobre variables/funciones no usadas
        -Woverloaded-virtual # Advertir si se oculta una función virtual heredada
        -Wconversion    # Advertir sobre conversiones que pueden perder datos
        -Wsign-conversion # Advertir sobre conversiones de signo
        -Wnull-dereference # Advertir sobre posibles desreferencias nulas
        -Wdouble-promotion # Advertir si un float se promueve a double implícitamente
        -Wformat=2      # Verificación estricta de formato
    )
endif()

# Definición de archivos de cabecera
set(HEADER_FILES
    Exceptions.hpp
    Graph.hpp
    Algorithms.hpp
    Serializer.hpp
)

# Definición de archivos de implementación
set(SOURCE_FILES
    Serializer.cpp
    main.cpp
)

# Creación del ejecutable principal
add_executable(GraphEngine ${SOURCE_FILES} ${HEADER_FILES})

# Directorios de inclusión
target_include_directories(GraphEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Configuración de optimización por defecto a Release si no se especifica
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# Mensajes informativos durante la generación de CMake
message(STATUS "-----------------------------------------------------------")
message(STATUS "Configurando GraphEngine v${PROJECT_VERSION}")
message(STATUS "Compilador: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Tipo de Build: ${CMAKE_BUILD_TYPE}")
message(STATUS "-----------------------------------------------------------")

# Propiedades del ejecutable
set_target_properties(GraphEngine PROPERTIES
    OUTPUT_NAME "MotorGrafos"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Regla de instalación
install(TARGETS GraphEngine DESTINATION bin)
