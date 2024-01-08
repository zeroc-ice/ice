//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `InstrumentationF.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/InstrumentationF.h>
#import <objc/runtime.h>

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(CA2E53775_3308_4E67_9231_F4672DA18BB1)
-(void)addPrefixes_CA2E53775_3308_4E67_9231_F4672DA18BB1:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICE" forKey:@"::Ice"];
    [prefixTable setObject:@"ICEINSTRUMENTATION" forKey:@"::Ice::Instrumentation"];
}
@end
