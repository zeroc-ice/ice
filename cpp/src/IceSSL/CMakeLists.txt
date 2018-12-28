project(IceSSL)
include(IceSSL.cmake)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSTFIX "${ICE_LIB_POSTFIX}"
    DEBUG_POSTFIX "${ICE_LIB_POSTFIX}"
)

target_link_libraries(${PROJECT_NAME}
    Ice
    ${OPENSSL_LIBRARIES}
)

add_dependencies(${PROJECT_NAME}
    IceSSL_Slice2Cpp
)

iceInstallLibrary(${PROJECT_NAME})
