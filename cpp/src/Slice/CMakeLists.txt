add_library(slice STATIC)

target_sources(slice
	PRIVATE
		Checksum.cpp
		Checksum.h
		CPlusPlusUtil.cpp
		CPlusPlusUtil.h
		FileTracker.cpp
		FileTracker.h
		Grammar.cpp
		Grammar.h
		GrammarUtil.h
		JavaUtil.cpp
		JavaUtil.h
		MD5.cpp
		MD5.h
		MD5I.cpp
		MD5I.h
		Parser.cpp
		Parser.h
		PHPUtil.cpp
		PHPUtil.h
		Preprocessor.cpp
		Preprocessor.h
		Python.cpp
		PythonUtil.cpp
		PythonUtil.h
		Ruby.cpp
		RubyUtil.cpp
		RubyUtil.h
		Scanner.cpp
		SliceUtil.cpp
		StringLiteralUtil.cpp
		Util.h
)

target_include_directories(slice 
	PRIVATE 
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
)

target_link_libraries(slice
	PRIVATE
		iceutil
		${MCPP_LIB}
)

target_compile_definitions(slice 
	PRIVATE  
		${ICE_COMPILE_DEFS}
)

if(MSVC)
	target_compile_definitions(slice 
		PRIVATE  
			${ICE_MSVC_COMPILE_DEFS}
	)

	target_compile_options(slice 
		PRIVATE 
			${ICE_MSVC_COMPILE_OPTIONS}
	)
endif()

install(TARGETS slice DESTINATION lib)