project(Slice)

aux_source_directory(. SRC_LIST)
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_library(${PROJECT_NAME} ${SRC_LIST})
set_target_properties(${PROJECT_NAME} PROPERTIES
    POSTFIX "${ICE_LIB_POSTFIX}"
    DEBUG_POSTFIX "${ICE_LIB_POSTFIX}"
)

target_link_libraries(${PROJECT_NAME}
    IceUtil
    mcpp
)

iceInstallLibrary(${PROJECT_NAME})
iceInstallHeaders(${CMAKE_CURRENT_SOURCE_DIR})
