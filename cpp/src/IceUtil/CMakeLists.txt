project(IceUtil)

aux_source_directory(. SRC_LIST)
#add_definitions(-DICE_LIBBACKTRACE)
add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
set_target_properties(${PROJECT_NAME} PROPERTIES
    POSTFIX "${ICE_LIB_POSTFIX}"
    DEBUG_POSTFIX "${ICE_LIB_POSTFIX}"
)

if(NOT UNIX)
    target_link_libraries(${PROJECT_NAME}
        shlwapi
        rpcrt4
    )
endif()

iceInstallLibrary(${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING
            PATTERN "*.h"
)
