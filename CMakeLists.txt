cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("GEOAProject")

# --- Sources ---
add_executable(GEOAProject
        FlyFish.cpp
        Game.cpp
        structs.cpp
        utils.cpp
        main.cpp
)

# Automatically include all .cpp in Gameplay/
file(GLOB GAMEPLAY_SRC CONFIGURE_DEPENDS "Gameplay/*.cpp")
target_sources(GEOAProject PRIVATE ${GAMEPLAY_SRC})

# (optional) upgrade to C++20 if CMake >= 3.12
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET GEOAProject PROPERTY CXX_STANDARD 20)
endif()

# Make sure the compiler can find "Gameplay/*.h" in includes
target_include_directories(GEOAProject PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# --- SDL2 ---
set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/SDL2-2.30.9")
add_library(SDL STATIC IMPORTED)
set_target_properties(SDL PROPERTIES
        IMPORTED_LOCATION "${SDL_DIR}/lib/SDL2.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL_DIR}/include"
)

# --- SDL2_ttf ---
set(SDL_TTF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/SDL2_ttf-2.22.0")
add_library(SDL_TTF STATIC IMPORTED)
set_target_properties(SDL_TTF PROPERTIES
        IMPORTED_LOCATION "${SDL_TTF_DIR}/lib/SDL2_ttf.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL_TTF_DIR}/include"
)

# Link SDL2main.lib explicitly if available
if(EXISTS "${SDL_DIR}/lib/SDL2main.lib")
    set_target_properties(SDL PROPERTIES
            IMPORTED_LINK_INTERFACE_LIBRARIES "${SDL_DIR}/lib/SDL2main.lib")
else()
    message(FATAL_ERROR "SDL2main.lib not found in ${SDL_DIR}/lib.")
endif()

target_link_libraries(GEOAProject PRIVATE SDL SDL_TTF opengl32)

# Copy runtime DLLs next to the exe
file(GLOB_RECURSE DLL_FILES
        "${SDL_DIR}/lib/*.dll"
        "${SDL_DIR}/lib/*.manifest"
        "${SDL_TTF_DIR}/lib/*.dll"
        "${SDL_TTF_DIR}/lib/*.manifest"
)
foreach(DLL ${DLL_FILES})
    add_custom_command(TARGET GEOAProject POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${DLL} $<TARGET_FILE_DIR:GEOAProject>)
endforeach()
