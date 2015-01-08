// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Foundation/NSObject.h>
#import <Foundation/NSException.h>

@interface TestFailedException : NSException
@end

#if TARGET_OS_IPHONE
void TestCommonInit(id, SEL);
void TestCommonTestInit(id, SEL, BOOL, BOOL, BOOL);
#endif

@protocol ICECommunicator;
@protocol ICEProperties;

id<ICEProperties> defaultServerProperties();
id<ICEProperties> defaultClientProperties();

void serverReady(id<ICECommunicator>);

void serverStop();

void tprintf(const char* fmt, ...);

void testFailed(const char*, const char*, unsigned int);

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))
