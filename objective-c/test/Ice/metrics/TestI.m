//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <metrics/TestI.h>

@implementation MetricsI

-(id) init
{
    self = [super init];
    return self;
}

-(void) op:(ICECurrent*)__unused current
{
}

-(void) fail:(ICECurrent*)current
{
    [current.con close:ICEConnectionCloseForcefully];
}

-(void) opWithUserException:(ICECurrent*)__unused current
{
    @throw [TestMetricsUserEx userEx];
}

-(void) opWithRequestFailedException:(ICECurrent*)__unused current
{
    @throw [ICEObjectNotExistException objectNotExistException:__FILE__ line:__LINE__];
}

-(void) opWithLocalException:(ICECurrent*)__unused current
{
    @throw [ICESyscallException syscallException:__FILE__ line:__LINE__];
}

-(void) opWithUnknownException:(ICECurrent*)__unused current
{
    @throw @"TEST";
}

-(void) opByteS:(ICEByteSeq*)__unused bs current:(ICECurrent*)__unused current
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
        self->adapter = ICE_RETAIN(adapter_p);
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

-(void) hold:(ICECurrent*)__unused current
{
    [adapter hold];
    [adapter waitForHold];
}

-(void) resume:(ICECurrent*)__unused current
{
    [adapter activate];
}
@end
