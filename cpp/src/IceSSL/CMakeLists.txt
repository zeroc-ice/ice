if(NOT ${GENERATED_SLICES_DIR}/IceSSL)
	file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IceSSL)
endif()

set(GENERATED_ICESSL_HEADERS
	${GENERATED_SLICES_DIR}/IceSSL/ConnectionInfo.h
	${GENERATED_SLICES_DIR}/IceSSL/ConnectionInfoF.h
	${GENERATED_SLICES_DIR}/IceSSL/EndpointInfo.h
)

set(GENERATED_ICESSL_SOURCES
	${GENERATED_SLICES_DIR}/IceSSL/ConnectionInfo.cpp
	${GENERATED_SLICES_DIR}/IceSSL/ConnectionInfoF.cpp
	${GENERATED_SLICES_DIR}/IceSSL/EndpointInfo.cpp
)

set(ICESSL_SOURCES
	AcceptorI.cpp
	CertificateI.cpp
	ConnectorI.cpp
	EndpointI.cpp
	Instance.cpp
	PluginI.cpp
	RFC2253.cpp
	SSLEngine.cpp
	TrustManager.cpp
	Util.cpp
	AcceptorI.h
	CertificateI.h
	ConnectorI.h
	EndpointI.h
	Instance.h
	InstanceF.h
	PluginI.h
	RFC2253.h
	SSLEngine.h
	SSLEngineF.h
	TrustManager.h
	TrustManagerF.h
	Util.h
)

list(APPEND ICESSL_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICESSL_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceSSL")

foreach(slice IN LISTS ICESSL_SLICES)
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
	CompileSlice("${ICESSL_SLICE_PARAMS}"
		"${ICESSL_SLICE_DIRS}"
		${slice}
		${GENERATED_SLICES_DIR}/IceSSL
	)
endforeach()

install(DIRECTORY ${GENERATED_SLICES_DIR}/IceSSL 
	DESTINATION ${GENERATED_H_INSTALL_DIR}
	FILES_MATCHING PATTERN "*.h"
)

if(BUILD_ICE_CPP11)
	set(ICESSL_TARGET icessl++11)

	add_library(${ICESSL_TARGET})

	set_target_properties(${ICESSL_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceSSL${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
			CXX_STANDARD 11
	)

	target_link_libraries(${ICESSL_TARGET} PRIVATE ice++11)
else()
	set(ICESSL_TARGET icessl)

	add_library(${ICESSL_TARGET})

	set_target_properties(${ICESSL_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceSSL${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	target_link_libraries(${ICESSL_TARGET} PRIVATE ice)
endif()

if(APPLE)
	find_library(LIB_COREFOUNDATION "CoreFoundation")
	target_link_libraries(${ICESSL_TARGET} PRIVATE ${LIB_COREFOUNDATION})
endif()

target_sources(${ICESSL_TARGET}
	PRIVATE
		${ICESSL_SOURCES}
		${GENERATED_ICESSL_HEADERS}
		${GENERATED_ICESSL_SOURCES}
)

target_include_directories(${ICESSL_TARGET}
	PRIVATE 
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src 
)

target_compile_definitions(${ICESSL_TARGET}
	PRIVATE
		${ICE_COMPILE_DEFS}
		"ICESSL_API_EXPORTS"
)

if(APPLE)
	target_sources(${ICESSL_TARGET}
		PRIVATE
			SecureTransportCertificateI.cpp
			SecureTransportEngine.cpp
			SecureTransportEngine.h
			SecureTransportEngineF.h
			SecureTransportTransceiverI.cpp
			SecureTransportTransceiverI.h
			SecureTransportUtil.cpp
			SecureTransportUtil.h
	)
endif()

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceSSL.rc
	)

	target_sources(${ICESSL_TARGET}
		PRIVATE
			SChannelCertificateI.cpp
			SChannelEngine.cpp
			SChannelPluginI.cpp
			SChannelTransceiverI.cpp
			SChannelEngine.h
			SChannelEngineF.h
			SChannelTransceiverI.h
			IceSSL.rc
	)

	target_link_libraries(${ICESSL_TARGET}
		PRIVATE
			advapi32.lib
			secur32.lib
			crypt32.lib
			ws2_32.lib
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceSSL
		PREFIX "Header Files/Generated"
		FILES
			${GENERATED_ICESSL_HEADERS}
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceSSL
		PREFIX "Source Files/Generated"
		FILES
			${GENERATED_ICESSL_SOURCES}
	)

	source_group(
		TREE ${SLICE_DIR}/IceSSL
		PREFIX "Slice Files"
		FILES
			${ICESSL_SLICES}
	)

	set_property(TARGET ${ICESSL_TARGET} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

	target_compile_options(${ICESSL_TARGET} 
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS ${ICESSL_TARGET} DESTINATION bin)
else()
	install(TARGETS ${ICESSL_TARGET} DESTINATION lib)
endif()

if(BUILD_ICE_CPP11)
	set(ICESSL_OPENSSL_TARGET icesslopenssl++11)

	add_library(${ICESSL_OPENSSL_TARGET})

	set_target_properties(${ICESSL_OPENSSL_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceSSLOpenSSL${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
			CXX_STANDARD 11
	)
else()
	set(ICESSL_OPENSSL_TARGET icesslopenssl)

	add_library(${ICESSL_OPENSSL_TARGET})

	set_target_properties(${ICESSL_OPENSSL_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceSSLOpenSSL${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)
endif()

target_sources(${ICESSL_OPENSSL_TARGET}
	PRIVATE
		OpenSSLCertificateI.cpp
		OpenSSLEngine.cpp
		OpenSSLEngine.h
		OpenSSLEngineF.h
		OpenSSLPluginI.cpp
		OpenSSLTransceiverI.cpp
		OpenSSLTransceiverI.h
		OpenSSLUtil.cpp
		OpenSSLUtil.h
)

target_include_directories(${ICESSL_OPENSSL_TARGET}
	PRIVATE
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src 
		${OPENSSL_INCLUDE_DIR}
)

target_compile_definitions(${ICESSL_OPENSSL_TARGET}
	PRIVATE
		${ICE_COMPILE_DEFS}
		"ICESSL_OPENSSL_API_EXPORTS"
)


if(BUILD_ICE_CPP11)
	target_link_libraries(${ICESSL_OPENSSL_TARGET}
		PRIVATE
			icessl++11
			ice++11
			OpenSSL::SSL
			OpenSSL::Crypto
	)
else()
	target_link_libraries(${ICESSL_OPENSSL_TARGET}
		PRIVATE 
			icessl
			ice
			OpenSSL::SSL 
			OpenSSL::Crypto
	)
endif()

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceSSLOpenSSL.rc
	)

	target_sources(${ICESSL_OPENSSL_TARGET}
		PRIVATE
			IceSSLOpenSSL.rc
	)

	set_property(TARGET ${ICESSL_OPENSSL_TARGET} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

	target_compile_options(${ICESSL_OPENSSL_TARGET} 
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)

	target_compile_definitions(${ICESSL_OPENSSL_TARGET}
		PRIVATE
			${ICE_WIN32_COMPILE_DEFS}
	)
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS ${ICESSL_OPENSSL_TARGET} DESTINATION bin)
else()
	install(TARGETS ${ICESSL_OPENSSL_TARGET} DESTINATION lib)
endif()
