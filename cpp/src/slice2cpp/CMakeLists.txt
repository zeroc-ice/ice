project(slice2cpp)

aux_source_directory(. SRC_LIST)
include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    Slice
    IceUtil
)

set(Ice_SLICE2CPP_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/slice2cpp CACHE FILEPATH "path to slice2cpp executable")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
