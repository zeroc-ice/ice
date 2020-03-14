add_library(iceutil STATIC)

target_sources(iceutil 
	PRIVATE
		ConsoleUtil.cpp
		CtrlCHandler.cpp
		FileUtil.cpp
		InputUtil.cpp
		MutexProtocol.cpp
		Options.cpp
		OutputUtil.cpp
		Random.cpp
		RecMutex.cpp
		Shared.cpp
		StringConverter.cpp
		StringUtil.cpp
		ThreadException.cpp
		Time.cpp
		UtilException.cpp
		UUID.cpp
)

# handle win specific UTF limitations
if(CMAKE_VS_PLATFORM_TOOLSET_VERSION MATCHES "v100" OR UNIX)
	target_sources(iceutil
		PRIVATE
			ConvertUTF.cpp
			ConvertUTF.h
			Unicode.cpp	
			Unicode.h	
	)
endif()

target_include_directories(iceutil 
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
)

target_compile_definitions(iceutil 
	PRIVATE
		${ICE_COMPILE_DEFS}
)

if(BUILD_SHARED_LIBS)
	target_compile_definitions(iceutil 
		PRIVATE
			"ICE_API_EXPORTS"
	)
endif()

target_link_libraries(iceutil PRIVATE Threads::Threads)

if(MSVC)
	target_compile_options(iceutil 
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)
endif()

install(TARGETS iceutil DESTINATION lib)
