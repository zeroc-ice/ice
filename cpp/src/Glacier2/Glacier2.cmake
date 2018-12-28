add_definitions(-DICE_BUILDING_SRC)

createSlice2XXXTarget(Glacier2_Slice2Cpp_Internal ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Ice_SLICE2CPP_EXECUTABLE} "${SLICE_SOURCE_DIR}" h cpp Glacier2)
add_dependencies(Glacier2_Slice2Cpp_Internal slice2cpp)

file(GLOB SLICE2CPP_LIST ${CMAKE_CURRENT_BINARY_DIR}/*.cpp)
#TODO: get_target_property(SLICE2CPP_LIST Glacier2_Slice2Cpp_Internal ICE2XXX_SOURCES)

aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${SLICE2CPP_LIST})

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${SLICE_GENERATED_DIR}
)
