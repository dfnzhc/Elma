find_path(EMBREE_INCLUDE_PATH embree4/rtcore.h
        ${CMAKE_SOURCE_DIR}/ThirdParty/embree/include)

find_library(EMBREE_LIBRARY NAMES embree4 PATHS
        ${CMAKE_SOURCE_DIR}/ThirdParty/embree/lib)

if (EMBREE_INCLUDE_PATH AND EMBREE_LIBRARY)
    set(EMBREE_FOUND TRUE)
endif ()