project(IceGridLib)
include(IceGridLib.cmake)

add_library(${PROJECT_NAME} ${SRC_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME IceGrid
    POSTFIX "${ICE_LIB_POSTFIX}"
    DEBUG_POSTFIX "${ICE_LIB_POSTFIX}"
)

target_link_libraries(${PROJECT_NAME}
    Ice
    Glacier2Lib
)

add_dependencies(${PROJECT_NAME}
    Ice
    Glacier2Lib
)

iceInstallLibrary(${PROJECT_NAME})
