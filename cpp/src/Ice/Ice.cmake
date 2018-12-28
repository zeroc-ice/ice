add_definitions(-DICE_BUILDING_SRC -DICE_API_EXPORTS)

file(GLOB SLICE2CPP_LIST ${SLICE_GENERATED_DIR}/Ice/*.cpp)
#TODO: get_target_property(SLICE2CPP_LIST Ice_Slice2Cpp ICE2XXX_SOURCES)
aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${SLICE2CPP_LIST})

if(UNIX)
    list(REMOVE_ITEM SRC_LIST ./DLLMain.cpp)
endif()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${SLICE_GENERATED_DIR}
)

iceInstallHeaders(${CMAKE_CURRENT_SOURCE_DIR}/../../include/${PROJECT_NAME})
iceInstallHeaders(${SLICE_GENERATED_DIR}/${PROJECT_NAME})
