include(cmake/CreateAMDExecutableIceTest.cmake)
include(cmake/CreateOpenSSLExecutableIceTest.cmake)
include(cmake/CreateExecutableIceTest.cmake)
include(cmake/CreateExecutableImplIceTest.cmake)
include(cmake/CreateExecutableInternalIceTest.cmake)
include(cmake/CreateExecutableTest.cmake)
include(cmake/CreateLibraryIceTest.cmake)
include(cmake/CreateLibraryInternalIceTest.cmake)
include(cmake/CreateLibraryTest.cmake)

add_subdirectory(Common)
add_subdirectory(Glacier2)
add_subdirectory(Ice)
add_subdirectory(IceBox)
add_subdirectory(IceDiscovery)
add_subdirectory(IceGrid)
add_subdirectory(IceSSL)

if(NOT BUILD_ICE_CPP11)
	add_subdirectory(IceUtil)
	add_subdirectory(Slice)
endif()

if(NOT (BUILD_ICE_CPP11 AND MSVC))
	# cpp-11 build errors, MSVC 2019
	add_subdirectory(IceBridge)
	add_subdirectory(IceStorm)
	# ------------------------------
endif()

# if(APPLE)
# 	add_subdirectory(ios)
# endif()
