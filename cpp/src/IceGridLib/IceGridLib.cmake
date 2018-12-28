add_definitions(-DICE_BUILDING_SRC -DICEGRID_API_EXPORTS)

file(GLOB SLICE2CPP_LIST ${SLICE_GENERATED_DIR}/IceGrid/*.cpp)
aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${SLICE2CPP_LIST})

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${SLICE_GENERATED_DIR}
    ${SLICE_GENERATED_DIR}/Glacier2
)

iceInstallHeaders(${CMAKE_CURRENT_SOURCE_DIR}/../../include/IceGrid)
iceInstallHeaders(${SLICE_GENERATED_DIR}/IceGrid)
