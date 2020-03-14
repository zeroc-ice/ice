set(ice_custom_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(custom_slices
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Wstring.ice
)

set(custom_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.h
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.h
)

CreateExecutableIceTest(Ice
	"${ice_custom_includes}"
	${ICE_TEST_OUTPUT_DIR}/custom 
	"${custom_client_sources}"
	"${custom_slices}"
	icecustomclient
	client 
	"${ICE_TEST_DEPS}"
)

set(custom_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.h
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
	${CMAKE_CURRENT_SOURCE_DIR}/WstringI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WstringI.h
)

CreateExecutableIceTest(Ice
	"${ice_custom_includes}"
	${ICE_TEST_OUTPUT_DIR}/custom 
	"${custom_collocated_sources}"
	"${custom_slices}"
	icecustomcollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(custom_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp	
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
	${CMAKE_CURRENT_SOURCE_DIR}/WstringI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WstringI.h
)

CreateExecutableIceTest(Ice
	"${ice_custom_includes}"
	${ICE_TEST_OUTPUT_DIR}/custom 
	"${custom_server_sources}"
	"${custom_slices}"
	icecustomserver
	server 
	"${ICE_TEST_DEPS}"
)

set(custom_slices_amd
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	${CMAKE_CURRENT_SOURCE_DIR}/WstringAMD.ice
)

set(custom_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MyByteSeq.h
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp	
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StringConverterI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
	${CMAKE_CURRENT_SOURCE_DIR}/WstringAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WstringAMDI.h
)

CreateExecutableIceTest(Ice
	"${ice_custom_includes}"
	${ICE_TEST_OUTPUT_DIR}/custom 
	"${custom_serveramd_sources}"
	"${custom_slices_amd}"
	icecustomserveramd
	serveramd 
	"${ICE_TEST_DEPS}"
)