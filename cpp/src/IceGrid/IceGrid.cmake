add_definitions(-DICE_BUILDING_SRC)
#-DICE_API_EXPORTS)# -DICE_IGNORE_VERSION)

createSlice2XXXTarget(IceGrid_Slice2Cpp_Internal ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Ice_SLICE2CPP_EXECUTABLE} "${SLICE_SOURCE_DIR}" h cpp IceGrid)
add_dependencies(IceGrid_Slice2Cpp_Internal slice2cpp)

#file(GLOB SLICE2CPP_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
#aux_source_directory(. SRC_LIST)
#list(APPEND SRC_LIST ${SLICE2CPP_LIST})

#if(UNIX)
#    list(REMOVE_ITEM SRC_LIST ./DLLMain.cpp)
#endif()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${SLICE_GENERATED_DIR}
)

list(APPEND LOCAL_NODE_SRCS Activator.cpp
                            NodeAdminRouter.cpp
                            NodeI.cpp
                            NodeSessionManager.cpp
                            ServerAdapterI.cpp
                            ServerI.cpp
)

list(APPEND LOCAL_REGISTRY_SRCS ${SLICE_GENERATED_DIR}/IceGrid/Internal.cpp
                                AdminRouter.cpp
                                DescriptorBuilder.cpp
                                DescriptorParser.cpp
                                FileCache.cpp
                                PlatformInfo.cpp
                                SessionManager.cpp
                                TraceLevels.cpp
                                AdminCallbackRouter.cpp
                                AdapterCache.cpp
                                AdminI.cpp
                                AdminSessionI.cpp
                                Allocatable.cpp
                                AllocatableObjectCache.cpp
                                Database.cpp
                                DescriptorHelper.cpp
                                FileUserAccountMapperI.cpp
                                InternalRegistryI.cpp
                                LocatorI.cpp
                                LocatorRegistryI.cpp
                                NodeCache.cpp
                                NodeSessionI.cpp
                                ObjectCache.cpp
                                PluginFacadeI.cpp
                                QueryI.cpp
                                ReapThread.cpp
                                RegistryAdminRouter.cpp
                                RegistryI.cpp
                                ReplicaCache.cpp
                                ReplicaSessionI.cpp
                                ReplicaSessionManager.cpp
                                ServerCache.cpp
                                SessionI.cpp
                                SessionServantManager.cpp
                                Topics.cpp
                                Util.cpp
                                WellKnownObjectsManager.cpp
)

list(APPEND LOCAL_ADMIN_SRCS ${SLICE_GENERATED_DIR}/IceGrid/Internal.cpp
                             Client.cpp
                             DescriptorBuilder.cpp
                             DescriptorHelper.cpp
                             DescriptorParser.cpp
                             FileParserI.cpp
                             Grammar.cpp
                             Parser.cpp
                             Scanner.cpp
                             Util.cpp
)

#[[
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING
            PATTERN "*.h"
)

install(DIRECTORY ${SLICE_GENERATED_DIR}/${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING
            PATTERN "*.h"
)]]
