find_package(OpenSSL REQUIRED)
add_definitions(-DICESSL_API_EXPORTS -DICESSL_OPENSSL_API_EXPORTS)# -DICE_IGNORE_VERSION)-DICE_BUILDING_SRC

aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST
    "./SecureTransportCertificateI.cpp"
    "./SecureTransportEngine.cpp"
    "./SecureTransportPluginI.cpp"
    "./SecureTransportTransceiverI.cpp"
    "./SecureTransportUtil.cpp"
    "./SChannelEngine.cpp"
    "./SChannelCertificateI.cpp"
    "./SChannelPluginI.cpp"
    "./SChannelTransceiverI.cpp"
    "./UWPEngine.cpp"
    "./UWPCertificateI.cpp"
    "./UWPPluginI.cpp"
    "./UWPTransceiverI.cpp"
)

file(GLOB SLICE2CPP_LIST ${SLICE_GENERATED_DIR}/IceSSL/*.cpp)
list(APPEND SRC_LIST ${SLICE2CPP_LIST})

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${SLICE_GENERATED_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING
            PATTERN "*.h"
)
