// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>

#import <Foundation/Foundation.h>

@interface OnewayAMICallback : NSObject
{
    BOOL called;
    NSCondition* cond;
}
-(void) check;
-(void) called;
@end

@implementation OnewayAMICallback
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cond = [[NSCondition alloc] init];
    return self;
}

+(id) create
{
    return ICE_AUTORELEASE([[OnewayAMICallback alloc] init]);
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [cond release];
    [super dealloc];
}
#endif

-(void) check
{
    [cond lock];
    while(!called)
    {
        [cond wait];
    }
    called = NO;
    [cond unlock];
}
-(void) called
{
    [cond lock];
    called = YES;
    [cond signal];
    [cond unlock];
}
-(void) opVoidResponse
{
    [self called];
}
-(void) opVoidException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opVoidExResponse
{
    test(NO);
}
-(void) opVoidExException:(ICEException*)ex
{
    test([ex isKindOfClass:[ICENoEndpointException class]]);
    [self called];
}
-(void) opByteExResponse
{
    test(NO);
}
-(void) opByteEx:(ICEException*)__unused ex
{
    test(NO);
}
-(void) opByteExException:(ICEException*)ex
{
    test([ex isKindOfClass:[ICETwowayOnlyException class]]);
    [self called];
}
@end

void
onewaysAMI(id<ICECommunicator> __unused communicator, id<TestOperationsMyClassPrx> proxy)
{
    id<TestOperationsMyClassPrx> p = [TestOperationsMyClassPrx uncheckedCast:[proxy ice_oneway]];

    {
        OnewayAMICallback* cb = [OnewayAMICallback create];
        [p begin_opVoid:^() { [cb opVoidResponse]; } exception:^(ICEException* ex) { [cb opVoidException:ex]; }];
    }

    {
        OnewayAMICallback* cb = [OnewayAMICallback create];
        @try
        {
            [p begin_opByte:0 p2:0 response:^(ICEByte __unused r, ICEByte __unused p3) { [cb opByteExResponse]; } exception:^(ICEException* ex) { [cb opByteExException:ex]; }];
            [cb check];
        }
        @catch(NSException* ex)
        {
            test([ex name] == NSInvalidArgumentException);
        }
    }
}
