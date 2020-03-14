if(BUILD_ICE_CPP11)
	set(ICESSL_TEST_DEPS
		ice++11
		icessl++11
		testcommon++11
	)

	set(OPENSSL_TEST_DEPS
		ice++11
		icessl++11
		icesslopenssl++11
		testcommon++11
		OpenSSL::SSL
		OpenSSL::Crypto
	)
else()
	set(ICESSL_TEST_DEPS
		ice
		icessl
		testcommon
	)

	set(OPENSSL_TEST_DEPS
		ice
		icessl
		icesslopenssl
		testcommon
		OpenSSL::SSL
		OpenSSL::Crypto
	)
endif()

set(ICESSL_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/IceSSL)

add_subdirectory(configuration)
