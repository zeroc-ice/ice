include(slice.cmake)

list(APPEND SLICE_DIRS_LIST
    Ice
    Glacier2
    IceGrid
    IceSSL
)

foreach(DIR_NAME ${SLICE_DIRS_LIST})
    set(INCLUDE_SOURCE_SLICE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
    createSlice2XXXTarget(${DIR_NAME}_Slice2Cpp ${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${DIR_NAME} ${Ice_SLICE2CPP_EXECUTABLE} "${INCLUDE_SOURCE_SLICE_DIRS}" h cpp ${DIR_NAME})

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/slice/
            FILES_MATCHING
                PATTERN "*.ice"
    )

    add_dependencies(${DIR_NAME}_Slice2Cpp slice2cpp)
endforeach()
