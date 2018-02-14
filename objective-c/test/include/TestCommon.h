// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Foundation/NSObject.h>
#import <Foundation/NSException.h>
#import <objc/Ice/Config.h>

#ifndef TEST_API
#   if defined(ICE_STATIC_LIBS)
#       define TEST_API /**/
#   elif defined(TEST_API_EXPORTS)
#       define TEST_API ICE_DECLSPEC_EXPORT
#   else
#       define TEST_API ICE_DECLSPEC_IMPORT
#   endif
#endif

TEST_API @interface TestFailedException : NSException
@end

#if TARGET_OS_IPHONE
TEST_API void TestCommonInit(id, SEL);
TEST_API void TestCommonTestInit(id, SEL, NSString*, BOOL, BOOL);
#endif

@protocol ICECommunicator;
@protocol ICEProperties;

TEST_API id<ICEProperties> defaultServerProperties();
TEST_API id<ICEProperties> defaultClientProperties();

TEST_API void serverReady(id<ICECommunicator>);

TEST_API void serverStop();

TEST_API void tprintf(const char* fmt, ...);

TEST_API void testFailed(const char*, const char*, unsigned int);

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))
