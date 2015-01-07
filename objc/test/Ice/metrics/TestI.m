// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <metrics/TestI.h>

@implementation MetricsI

-(id) init
{
    self = [super init];
    return self;
}

-(void) op:(ICECurrent*)current
{
}

-(void) fail:(ICECurrent*)current
{
    [current.con close:YES];
}

-(void) opWithUserException:(ICECurrent*)current
{
    @throw [TestMetricsUserEx userEx];
}

-(void) opWithRequestFailedException:(ICECurrent*)current
{
    @throw [ICEObjectNotExistException objectNotExistException:__FILE__ line:__LINE__];
}

-(void) opWithLocalException:(ICECurrent*)current
{
    @throw [ICESyscallException syscallException:__FILE__ line:__LINE__];
}

-(void) opWithUnknownException:(ICECurrent*)current
{
    @throw @"TEST";
}

-(void) opByteS:(ICEByteSeq*)bs current:(ICECurrent*)current
{
}

-(ICEObjectPrx*) getAdmin:(ICECurrent*)current
{
    return [[current.adapter getCommunicator] getAdmin];
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end

@implementation ControllerI
-(id) init:(id<ICEObjectAdapter>)adapter_p
{
    self = [super init];
    if(self)
    {
#if defined(__clang__) && !__has_feature(objc_arc)
        self->adapter = [adapter_p retain];
#else
        self->adapter = adapter_p;
#endif
    }
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->adapter release];
    [super dealloc];
}
#endif

-(void) hold:(ICECurrent*)current
{
    [adapter hold];
    [adapter waitForHold];
}

-(void) resume:(ICECurrent*)current
{
    [adapter activate];
}
@end
