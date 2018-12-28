project(Ice)
include(Ice.cmake)

add_library(${PROJECT_NAME} ${SRC_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSTFIX "${ICE_LIB_POSTFIX}"
    DEBUG_POSTFIX "${ICE_LIB_POSTFIX}"
)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    IceUtil
)

if(NOT UNIX)
    target_link_libraries(${PROJECT_NAME}
        ws2_32
        wsock32
        iphlpapi
    )
else()
    target_link_libraries(${PROJECT_NAME}
        bz2
        dl
        crypto
    )
endif()

add_dependencies(${PROJECT_NAME}
    Ice_Slice2Cpp
)

iceInstallLibrary(${PROJECT_NAME})
