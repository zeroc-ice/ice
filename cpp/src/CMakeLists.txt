include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(IceUtil)
add_subdirectory(Slice)
add_subdirectory(slice2cpp)

add_subdirectory(../../slice slice2cpp_generated)

add_subdirectory(Ice)
add_subdirectory(Glacier2Lib)
add_subdirectory(IceSSL)
add_subdirectory(IceGridLib)
#add_subdirectory(IceGrid)

if(NOT ICE_CPP11_MAPPING)
    # TODO: It hasn't compiled yet
    add_subdirectory(Glacier2)
endif()
