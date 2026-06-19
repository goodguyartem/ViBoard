cmake_minimum_required(VERSION 4.0)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed.\n"
        "Please run: cmake -B build"
    )
endif()

project(ViBoard LANGUAGES CXX C VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Project build types." FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(GNUInstallDirs)

set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

set(MINIAUDIO_NO_LIBVORBIS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
)
FetchContent_Declare(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG v1.17.0
)
FetchContent_Declare(
	nativefiledialog-extended
	GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
	GIT_TAG v1.3.0
)
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.9.0
)
 
FetchContent_MakeAvailable(
    glfw
    spdlog
    nativefiledialog-extended
    yaml-cpp
)

find_package(OpenGL REQUIRED)
include(cmake/imgui.cmake)

add_executable(ViBoardBin)

set_target_properties(ViBoardBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_NAME viboard
)

set(PROJECT_RESOURCE_DIR
    "${CMAKE_INSTALL_DATADIR}/viboard")

configure_file(
    src/Project.hpp.in
    ${CMAKE_BINARY_DIR}/Project.hpp
    @ONLY
)
target_include_directories(ViBoardBin PUBLIC 
    "${CMAKE_BINARY_DIR}" 
)

if (MSVC)
	target_compile_options(ViBoardBin PRIVATE /W4 /wd4458)
else()
	target_compile_options(ViBoardBin PRIVATE 
        -Wall 
        -Wextra
        -Wno-missing-field-initializers 
        $<$<CONFIG:Debug>:-fsanitize=address>
    )

    target_link_options(ViBoardBin PRIVATE 
        $<$<CONFIG:Debug>:-fsanitize=address>
    )

    find_package(X11)
    
    if (X11_FOUND)
        target_compile_definitions(ViBoardBin PRIVATE VI_HAS_X11)
        target_link_libraries(ViBoardBin PRIVATE X11 Xtst)
    endif()
endif()

target_sources(ViBoardBin PRIVATE 
    src/Main.cpp
    src/App.cpp
    src/App.hpp
    src/Soundboard.cpp
    src/Soundboard.hpp
    src/Window.cpp
    src/Window.hpp
    src/CommandLine.cpp
    src/CommandLine.hpp
    src/Audio.cpp
    src/Audio.hpp
    src/Util.cpp
    src/Util.hpp
    src/YamlTypes.cpp
    src/YamlTypes.hpp
    src/Paths.cpp
    src/Paths.hpp
    src/IO.cpp
    src/IO.hpp
    src/platforms/Platform.cpp
    src/platforms/Platform.hpp
    src/hotkeys/HotkeyRegistry.cpp
    src/hotkeys/HotkeyRegistry.hpp
)

if (WIN32)
    target_sources(ViBoardBin PRIVATE 
        src/hotkeys/WindowsHotkeyRegistry.cpp
        src/hotkeys/WindowsHotkeyRegistry.hpp
        src/platforms/Windows.cpp
        src/platforms/Windows.hpp
    )
    target_compile_definitions(ViBoardBin PRIVATE NOMINMAX)

elseif(LINUX)
    target_sources(ViBoardBin PRIVATE src/platforms/Linux.cpp)
    if (X11_FOUND)
        target_sources(ViBoardBin PRIVATE 
            src/hotkeys/X11HotkeyRegistry.cpp
            src/hotkeys/X11HotkeyRegistry.hpp
            src/platforms/X11.cpp
            src/platforms/X11.hpp
        )

        target_compile_definitions(ViBoardBin PRIVATE
            GLFW_EXPOSE_NATIVE_X11
        )
    endif()
else()
    target_sources(ViBoardBin PRIVATE 
        src/hotkeys/DummyHotkeyRegistry.cpp
        src/hotkeys/DummyHotkeyRegistry.hpp
        src/platforms/Dummy.cpp
    )
endif()

get_target_property(src ViBoardBin SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${src})

# Local deps
add_subdirectory(deps/stb)
add_subdirectory(deps/miniaudio)

target_link_libraries(ViBoardBin PRIVATE
    OpenGL::GL
	glfw
    spdlog
    imgui
    stb
    miniaudio
    nfd
    yaml-cpp::yaml-cpp
)

target_compile_definitions(ViBoardBin PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
)

install(TARGETS ViBoardBin 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY res/ DESTINATION ${CMAKE_INSTALL_DATADIR}/viboard)