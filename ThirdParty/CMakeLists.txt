find_package(Embree REQUIRED)

set(EXTERNAL_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/miniz.h
        ${CMAKE_CURRENT_SOURCE_DIR}/miniz.c
        ${CMAKE_CURRENT_SOURCE_DIR}/pugiconfig.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pugixml.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pugixml.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tinyexr.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tinyply.h
        PARENT_SCOPE
)

set(NANOGUI_BUILD_EXAMPLES OFF)
add_subdirectory(nanogui)

set_target_properties(nanogui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ELMA_RUNTIME_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${ELMA_LIBRARY_OUTPUT_DIR}
)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads CONFIG REQUIRED)
