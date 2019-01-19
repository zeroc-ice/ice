//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
