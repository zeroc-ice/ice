// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Logger.h>

#import <objc/Ice/LocalObject.h>

#include <Ice/Logger.h>

@interface ICELogger : ICELocalObject<ICELogger>
{
    NSString* prefix_;
    NSString* formattedPrefix_;
}
+(Ice::Logger*)loggerWithLogger:(id<ICELogger>)arg;
@end

@interface ICELoggerWrapper : ICELocalObject<ICELogger>
{
    Ice::Logger* logger_;
}
+(id)loggerWithLogger:(Ice::Logger*)arg;
@end
