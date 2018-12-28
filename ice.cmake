option(CPP11 "c++11 support" ON)
option(CPP14 "c++14 support" ON)
option(CPP17 "c++17 support" ON)
option(BUILD_TESTS "c++17 support" OFF)
option(BUILD_SHARED_LIBS "Build shared libs" ON)

if(CPP11 OR CPP14 OR CPP17)
    option(ICE_CPP11 "c++11 support" ON)
    option(ICE_CPP11_MAPPING "c++11 mapping" ON)
else()
    option(ICE_CPP11 "c++11 support" OFF)
    option(ICE_CPP11_MAPPING "c++11 mapping" OFF)
endif()

if(ICE_CPP11)
    add_definitions(-DICE_CPP11)
    set(ICE_LIB_POSTFIX ++11)
endif()

if(ICE_CPP11_MAPPING)
    add_definitions(-DICE_CPP11_MAPPING)
endif()

if(CPP17)
    set(CMAKE_CXX_STANDARD 17)
elseif(CPP14)
    set(CMAKE_CXX_STANDARD 14)
elseif(CPP11)
    set(CMAKE_CXX_STANDARD 11)
endif()

if(NOT DEFINED MSVC)
    if(CPP17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    elseif(CPP14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    elseif(CPP11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

    # -Weverything
    set(WARNING_FLAGS "-Wall -Wextra -Wformat -Wformat-security -pedantic -Wpedantic -Wno-unused-parameter -Wstrict-aliasing")
    #clang
    set(WARNING_FLAGS "${WARNING_FLAGS} -Wno-vla -Wno-vla-extension")
    set(LIBC_FLAGS "-D_FORTIFY_SOURCE=2")
    set(HARDENING_TECH_FLAGS "-fPIC -pie -fstack-protector")
    # -fsanitize=address -fsanitize=thread

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fexceptions -Wno-deprecated-register")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${WARNING_FLAGS} -fstrict-aliasing -g3 -ggdb -Wno-vla -Wno-vla-extension")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror -pedantic-errors")
    set(INSTALL_LIBRARY_DLL LIBRARY)
else()
    set(INSTALL_LIBRARY_DLL RUNTIME)
endif()

find_package(Threads REQUIRED)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads -fno-keep-inline-dllexport -mwin32")#" -municode")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads -fno-keep-inline-dllexport -mwin32")#" -municode")
endif()

if(NOT DEFINED MSVC)
    list(APPEND CMAKE_THREAD_LIBS_INIT stdc++)

    if(NOT DEFINED MINGW)
        list(APPEND CMAKE_THREAD_LIBS_INIT rt)
    endif()
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*64.*")
    set(ARCH 64)
    set(LIB_ARCH "lib64")
else()
    set(ARCH 32)
    set(LIB_ARCH "lib")
endif()

if(WIN32)
    if(ARCH EQUAL 64)
        set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/ZeroC/Ice-${PROJECT_VERSION}")
    else()
        set(CMAKE_INSTALL_PREFIX "C:/Program Files/ZeroC/Ice-${PROJECT_VERSION}")
    endif()

    macro(iceInstallLibrary TARGET_NAME)
        install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
        install(TARGETS ${TARGET_NAME} ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    endmacro()
else()
    macro(iceInstallLibrary TARGET_NAME)
        install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIB_ARCH})
    endmacro()
endif()

macro(iceInstallHeaders SOURCE_HEADERS_DIRECTORY)
    install(DIRECTORY ${SOURCE_HEADERS_DIRECTORY}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include
            FILES_MATCHING
                PATTERN "*.h"
    )
endmacro()
