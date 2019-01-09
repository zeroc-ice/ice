// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <proxy/TestI.h>
#import <TestCommon.h>

@implementation TestProxyMyDerivedClassI
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_ctx release];
    [super dealloc];
}
#endif

-(id<ICEObjectPrx>) echo:(id<ICEObjectPrx>)obj current:(ICECurrent*)__unused current
{
    return obj;
}

-(void) shutdown:(ICECurrent*)current
{
    [[[current adapter] getCommunicator] shutdown];
}

-(ICEContext*) getContext:(ICECurrent*)__unused current
{
    return ICE_AUTORELEASE(ICE_RETAIN(_ctx));
}

-(BOOL) ice_isA:(NSString*)s current:(ICECurrent*)current
{
    ICE_RELEASE(_ctx);
    _ctx = ICE_RETAIN([current ctx]);
    return [super ice_isA:s current:current];
}

@end
