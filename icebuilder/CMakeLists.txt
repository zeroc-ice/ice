cmake_minimum_required(VERSION 3.7.2)

if(MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()
endif()

option(BUILD_ICEBUILDER_CXX OFF)

project(icebuilder)

find_package(Threads REQUIRED)

add_library(iceutil STATIC)

set_target_properties(iceutil 
	PROPERTIES 
		LANGUAGE CXX
)

target_sources(iceutil 
	PRIVATE
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/ConsoleUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/CtrlCHandler.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/FileUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/InputUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/MutexProtocol.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/Options.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/OutputUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/Random.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/RecMutex.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/Shared.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/StringConverter.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/StringUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/ThreadException.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/Time.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/UtilException.cpp
		${ICE_SOURCES_ROOT}/cpp/src/IceUtil/UUID.cpp
)

# handle win specific UTF limitations
if(CMAKE_VS_PLATFORM_TOOLSET_VERSION MATCHES "v100" OR UNIX)
	target_sources(iceutil
		PRIVATE
			${ICE_SOURCES_ROOT}/cpp/src/IceUtil/ConvertUTF.cpp
			${ICE_SOURCES_ROOT}/cpp/src/IceUtil/ConvertUTF.h
			${ICE_SOURCES_ROOT}/cpp/src/IceUtil/Unicode.cpp
			${ICE_SOURCES_ROOT}/cpp/src/IceUtil/Unicode.h
	)
endif()

target_include_directories(iceutil 
	PRIVATE
		${ICE_SOURCES_ROOT}/cpp/include
		${ICE_SOURCES_ROOT}/cpp/src
)

target_compile_definitions(iceutil
	PRIVATE
		"ICE_BUILDING_SRC"
		"ICE_BUILDING_SLICE_COMPILERS"
)

target_link_libraries(iceutil PRIVATE Threads::Threads)

if(NOT BUILD_SHARED_LIBS)
	target_compile_definitions(iceutil 
		PRIVATE
			"ICE_STATIC_LIBS"
	)
else()
	target_compile_definitions(iceutil 
		PRIVATE
			"ICE_API_EXPORTS"
	)
endif()

if(MSVC)
	target_compile_definitions(iceutil
		PRIVATE
			"_CONSOLE" 
			"WIN32_LEAN_AND_MEAN"
			"_SBCS"
			"_WIN32_WINNT=0x603"
	)

	target_compile_options(iceutil
		PRIVATE
			"/wd4121"
			"/wd4250"
			"/wd4251"
			"/wd4275"
			"/wd4324"
			"/wd4127"
			"/wd4505"
			"/wd4512"
			"/wd4834"
			"/MP"
			"/bigobj"
	)
endif()

install(TARGETS iceutil DESTINATION lib)

add_library(slice STATIC)

set_target_properties(slice 
	PROPERTIES 
		LANGUAGE CXX
)

target_sources(slice
	PRIVATE
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Checksum.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Checksum.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/CPlusPlusUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/CPlusPlusUtil.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/FileTracker.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/FileTracker.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Grammar.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Grammar.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/GrammarUtil.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/JavaUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/JavaUtil.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/MD5.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/MD5.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/MD5I.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/MD5I.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Parser.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Parser.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/PHPUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/PHPUtil.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Preprocessor.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Preprocessor.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Python.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/PythonUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/PythonUtil.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Ruby.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/RubyUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/RubyUtil.h
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Scanner.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/SliceUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/StringLiteralUtil.cpp
		${ICE_SOURCES_ROOT}/cpp/src/Slice/Util.h
)

target_include_directories(slice 
	PRIVATE 
		${ICE_SOURCES_ROOT}/cpp/include
		${ICE_SOURCES_ROOT}/cpp/src
)

target_link_libraries(slice
	PRIVATE
		iceutil
		${MCPP_LIB}
)

target_compile_definitions(slice 
	PRIVATE  
		"ICE_BUILDING_SRC"
		"ICE_STATIC_LIBS"
		"ICE_BUILDING_SLICE_COMPILERS"
)

if(MSVC)
	target_compile_definitions(slice 
		PRIVATE  
			"_CONSOLE" 
			"WIN32_LEAN_AND_MEAN"
			"_SBCS"
			"_WIN32_WINNT=0x603"
	)

	target_compile_options(slice 
		PRIVATE 
			"/wd4121"
			"/wd4250"
			"/wd4251"
			"/wd4275"
			"/wd4324"
			"/wd4127"
			"/wd4505"
			"/wd4512"
			"/wd4834"
			"/wd4098"
			"/MP"
			"/bigobj"
			"/wd4250"
	)
endif()

install(TARGETS slice DESTINATION lib)

if(BUILD_ICEBUILDER_CXX)
	set(SLICE2X_TARGET slice2cpp)

	set(SLICE2X_SOURCES
		${ICE_SOURCES_ROOT}/cpp/src/slice2cpp/Gen.cpp
		${ICE_SOURCES_ROOT}/cpp/src/slice2cpp/Gen.h
		${ICE_SOURCES_ROOT}/cpp/src/slice2cpp/Main.cpp
	)

	if(MSVC)
		source_group("Resource Files"
			FILES ${ICE_SOURCES_ROOT}/cpp/src/slice2cpp/Slice2Cpp.rc
		)

		set(SLICE2X_SOURCES
			${SLICE2X_SOURCES}
			${ICE_SOURCES_ROOT}/cpp/src/slice2cpp/Slice2Cpp.rc
		)
	endif()
endif()

add_executable(${SLICE2X_TARGET})

set_target_properties(${SLICE2X_TARGET}
	PROPERTIES
		LANGUAGE CXX
)

target_sources(${SLICE2X_TARGET}
	PRIVATE
		${SLICE2X_SOURCES}
)

target_include_directories(${SLICE2X_TARGET}
	PRIVATE
		${ICE_SOURCES_ROOT}/cpp/include
		${ICE_SOURCES_ROOT}/cpp/src
)

target_compile_definitions(${SLICE2X_TARGET} 
	PRIVATE 
		"ICE_BUILDING_SRC"
		"ICE_BUILDING_SLICE_COMPILERS"
		"ICE_STATIC_LIBS"
)

target_link_libraries(${SLICE2X_TARGET} 
	PRIVATE
		${MCPP_LIB}
		Threads::Threads
		slice 
)

if(MSVC)
	target_compile_definitions(${SLICE2X_TARGET} 
		PRIVATE 
			"_CONSOLE" 
			"WIN32_LEAN_AND_MEAN"
			"_SBCS"
			"_WIN32_WINNT=0x603"
	)

	target_compile_options(${SLICE2X_TARGET} 
		PRIVATE 
			"/wd4121"
			"/wd4250"
			"/wd4251"
			"/wd4275"
			"/wd4324"
			"/wd4127"
			"/wd4505"
			"/wd4512"
			"/wd4834"
			"/MP"
			"/bigobj"
			"/wd4250"
	)

	set_property(TARGET ${SLICE2X_TARGET} PROPERTY LINK_FLAGS "wsetargv.obj")

	target_link_libraries(${SLICE2X_TARGET} PRIVATE
		rpcrt4.lib
		advapi32.lib
		DbgHelp.lib
		Shlwapi.lib
	)
endif()

install(TARGETS ${SLICE2X_TARGET} DESTINATION bin)
