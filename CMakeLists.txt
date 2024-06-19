cmake_minimum_required(VERSION 3.27)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

project(Elma)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CompilerFlags)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source)

set(ELMA_RUNTIME_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(ELMA_LIBRARY_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_subdirectory(ThirdParty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glad/include)
include_directories(${EMBREE_INCLUDE_PATH})

link_libraries(${EMBREE_LIBRARY} glfw glm::glm)

include_directories(${CMAKE_SOURCE_DIR}/Sources/Elma)
file(GLOB_RECURSE SOURCE_FILES Sources/Elma/**.hpp Sources/Elma/**.inl Sources/Elma/**.cpp Sources/Elma/**.h Elma/**.c)

add_library(ElmaLib STATIC
        ${SOURCE_FILES}
        ${EXTERNAL_FILES}
)

target_compile_definitions(ElmaLib
        PUBLIC
        GLM_ENABLE_EXPERIMENTAL
)

add_executable(Elma Sources/main.cpp)
target_link_libraries(Elma ElmaLib) #  Threads::Threads

set_target_properties(ElmaLib PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ELMA_RUNTIME_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${ELMA_LIBRARY_OUTPUT_DIR}
)
set_target_properties(Elma PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ELMA_RUNTIME_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${ELMA_LIBRARY_OUTPUT_DIR}
)

if (MSVC)
    add_custom_command(TARGET Elma POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/embree/bin"
            ${ELMA_RUNTIME_OUTPUT_DIR})

    add_custom_command(TARGET Elma POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Data"
            ${ELMA_RUNTIME_OUTPUT_DIR}/Data)
endif ()

enable_testing()
add_subdirectory(Tests)