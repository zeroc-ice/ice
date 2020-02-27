if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceSSL)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceSSL)
endif()

set(GENERATED_SSL_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceSSL/Test.h)
set(GENERATED_SSL_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceSSL/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceSSL")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/IceSSL/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/IceSSL/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/IceSSL
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(SSL_DEPS 
		ice
		icessl
		testcommon
	)

	set(OPENSSL_DEPS 
		ice
		icessl
		icesslopenssl
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SSL_DEPS 
		ice++11
		icessl++11
		testcommon++11
	)

	set(OPENSSL_DEPS 
		ice++11
		icessl++11
		icesslopenssl++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(SSL_DEPS 
		iceuwp++11
		icessluwp++11
		testcommonuwp++11
	)
endif()

add_executable(icesslconfigurationclient)

target_sources(icesslconfigurationclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_SSL_HEADERS}
		${GENERATED_SSL_SOURCES}
)

target_include_directories(icesslconfigurationclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceSSL
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icesslconfigurationclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icesslconfigurationclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icesslconfigurationclient ${SSL_DEPS})
	set_property(TARGET icesslconfigurationclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icesslconfigurationclient 
		PRIVATE 
		${SSL_DEPS}
		advapi32.lib
		crypt32.lib
	)
endif()

target_compile_definitions(icesslconfigurationclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SSL_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SSL_HEADERS}
	)

	target_compile_options(icesslconfigurationclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icesslconfigurationclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icesslconfigurationclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icesslconfigurationclient DESTINATION tests/bin)

if(NOT BUILD_ICE_UWP)
	add_executable(icesslconfigurationclientopenssl)

	target_sources(icesslconfigurationclientopenssl
		PRIVATE
			AllTests.cpp
			Client.cpp
			${GENERATED_SSL_HEADERS}
			${GENERATED_SSL_SOURCES}
	)

	target_include_directories(icesslconfigurationclientopenssl
		PRIVATE
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/test/include
			${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_CURRENT_BINARY_DIR}/IceSSL
			${CMAKE_CURRENT_BINARY_DIR}
			${GENERATED_SLICES_DIR}
			${OPENSSL_INCLUDE_DIR}
	)

	if(BUILD_ICE_CPP98)
		set_property(TARGET icesslconfigurationclientopenssl PROPERTY CXX_STANDARD 98)
	elseif(BUILD_ICE_CPP11)
		set_property(TARGET icesslconfigurationclientopenssl PROPERTY CXX_STANDARD 11)
	endif()

	if(BUILD_ICE_UWP)
		add_dependencies(icesslconfigurationclientopenssl ${SSL_DEPS})
		set_property(TARGET icesslconfigurationclientopenssl PROPERTY CXX_STANDARD 11)
	else()
		target_link_libraries(icesslconfigurationclientopenssl 
			PRIVATE 
				${OPENSSL_DEPS}
				OpenSSL::SSL
				OpenSSL::Crypto
		)
	endif()

	target_compile_definitions(icesslconfigurationclientopenssl 
		PRIVATE 
			${ICE_COMPILE_DEFS}
			"ICE_USE_OPENSSL"
	)

	if(MSVC)
		source_group("Slice Files"
			FILES
				Test.ice
		)

		source_group("Source Files/Generated"
			FILES
				${GENERATED_SSL_SOURCES}
		)

		source_group("Header Files/Generated"
			FILES
				${GENERATED_SSL_HEADERS}
		)

		target_compile_options(icesslconfigurationclientopenssl
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}
		)

		if(NOT BUILD_ICE_UWP)
			set_property(TARGET icesslconfigurationclientopenssl PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
		else()
			set_property(TARGET icesslconfigurationclientopenssl PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
		endif()
	endif()

	install(TARGETS icesslconfigurationclientopenssl DESTINATION tests/bin)
endif()

add_executable(icesslconfigurationserver)

target_sources(icesslconfigurationserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_SSL_HEADERS}
		${GENERATED_SSL_SOURCES}
)

target_include_directories(icesslconfigurationserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceSSL
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icesslconfigurationserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icesslconfigurationserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icesslconfigurationserver ${SSL_DEPS})
	set_property(TARGET icesslconfigurationserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icesslconfigurationserver PRIVATE ${SSL_DEPS})
endif()

target_compile_definitions(icesslconfigurationserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SSL_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SSL_HEADERS}
	)

	target_compile_options(icesslconfigurationserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icesslconfigurationserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icesslconfigurationserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icesslconfigurationserver DESTINATION tests/bin)

if(NOT BUILD_ICE_UWP)
	add_executable(icesslconfigurationserveropenssl)

	target_sources(icesslconfigurationserveropenssl
		PRIVATE
			Server.cpp
			TestI.cpp
			${GENERATED_SSL_HEADERS}
			${GENERATED_SSL_SOURCES}
	)

	target_include_directories(icesslconfigurationserveropenssl
		PRIVATE
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/test/include
			${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_CURRENT_BINARY_DIR}/IceSSL
			${CMAKE_CURRENT_BINARY_DIR}
			${GENERATED_SLICES_DIR}
			${OPENSSL_INCLUDE_DIR}
	)

	if(BUILD_ICE_CPP98)
		set_property(TARGET icesslconfigurationserveropenssl PROPERTY CXX_STANDARD 98)
	elseif(BUILD_ICE_CPP11)
		set_property(TARGET icesslconfigurationserveropenssl PROPERTY CXX_STANDARD 11)
	endif()

	if(BUILD_ICE_UWP)
		add_dependencies(icesslconfigurationserveropenssl ${SSL_DEPS})
		set_property(TARGET icesslconfigurationserveropenssl PROPERTY CXX_STANDARD 11)
	else()
		target_link_libraries(icesslconfigurationserveropenssl 
			PRIVATE 
				${OPENSSL_DEPS}
				OpenSSL::SSL
				OpenSSL::Crypto
		)
	endif()

	target_compile_definitions(icesslconfigurationserveropenssl 
		PRIVATE 
			${ICE_COMPILE_DEFS}
			"ICE_USE_OPENSSL"
	)

	if(MSVC)
		source_group("Slice Files"
			FILES
				Test.ice
		)

		source_group("Source Files/Generated"
			FILES
				${GENERATED_SSL_SOURCES}
		)

		source_group("Header Files/Generated"
			FILES
				${GENERATED_SSL_HEADERS}
		)

		target_compile_options(icesslconfigurationserveropenssl
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}
		)

		if(NOT BUILD_ICE_UWP)
			set_property(TARGET icesslconfigurationserveropenssl PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
		else()
			set_property(TARGET icesslconfigurationserveropenssl PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
		endif()
	endif()

	install(TARGETS icesslconfigurationserveropenssl DESTINATION tests/bin)
endif()
