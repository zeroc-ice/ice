set(icessl_config_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceSSL
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
	${OPENSSL_INCLUDE_DIR}
)

if(MSVC)
	set(icessl_client_deps
		${ICESSL_TEST_DEPS}
		advapi32.lib
		crypt32.lib
	)
else()
	set(icessl_client_deps ${ICESSL_TEST_DEPS})
endif()

set(icessl_configclnt_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceSSL
	"${icessl_config_includes}"
	${ICESSL_TEST_OUTPUT_DIR}/configuration
	"${icessl_configclnt_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icesslconfigclient
	client
	"${icessl_client_deps}"
)

set(icessl_configsrv_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(IceSSL
	"${icessl_config_includes}"
	${ICESSL_TEST_OUTPUT_DIR}/configuration
	"${icessl_configsrv_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icesslconfigserver
	server
	"${ICESSL_TEST_DEPS}"
)

CreateOpenSSLExecutableIceTest(IceSSL
	"${icessl_config_includes}"
	${ICESSL_TEST_OUTPUT_DIR}/configuration
	"${icessl_configclnt_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icesslopensslconfigclnt
	clientopenssl
	"${OPENSSL_TEST_DEPS}"
)

CreateOpenSSLExecutableIceTest(IceSSL
	"${icessl_config_includes}"
	${ICESSL_TEST_OUTPUT_DIR}/configuration
	"${icessl_configsrv_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icesslopensslconfigsrv
	serveropenssl
	"${OPENSSL_TEST_DEPS}"
)
