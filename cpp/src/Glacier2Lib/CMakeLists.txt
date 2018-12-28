project(Glacier2Lib)
include(Glacier2Lib.cmake)

add_library(${PROJECT_NAME} ${SRC_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME Glacier2
    POSTFIX "${ICE_LIB_POSTFIX}"
    DEBUG_POSTFIX "${ICE_LIB_POSTFIX}"
)

target_link_libraries(${PROJECT_NAME}
    Ice
)

add_dependencies(${PROJECT_NAME}
    Ice
    Glacier2_Slice2Cpp
)

iceInstallLibrary(${PROJECT_NAME})
