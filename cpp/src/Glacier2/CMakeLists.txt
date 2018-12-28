project(Glacier2)
include(Glacier2.cmake)

add_executable(${PROJECT_NAME} ${SRC_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME glacier2router
    POSTFIX "${ICE_LIB_POSTFIX}"
    DEBUG_POSTFIX "${ICE_LIB_POSTFIX}"
)

target_link_libraries(${PROJECT_NAME}
    Ice
    Glacier2Lib
    IceSSL
)

add_dependencies(${PROJECT_NAME}
    slice2cpp
    Ice
    Glacier2Lib
    Glacier2_Slice2Cpp_Internal
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
