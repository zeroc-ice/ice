// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <IceUtil/Config.h>

#ifdef ICE_TEST_COMMON_API_EXPORTS
#   define ICE_TEST_COMMON_API ICE_DECLSPEC_EXPORT
#else
#   define ICE_TEST_COMMON_API ICE_DECLSPEC_IMPORT
#endif

ICE_TEST_COMMON_API void tprintf(const char* fmt, ...);

ICE_TEST_COMMON_API void testFailed(const char*, const char*, unsigned int);

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

class ICE_TEST_COMMON_API TestApplication
{
public:

#ifdef _WIN32_WCE
    int main(HINSTANCE);
#else
    int main(int, char*[]);
#endif

    virtual int run(int, char*[]) = 0;
};

#endif
