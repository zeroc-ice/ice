//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <adapterDeactivation/TestI.h>

#import <Foundation/NSThread.h>

@implementation TestAdapterDeactivationI

-(void) transient:(ICECurrent*)current
{
    id<ICECommunicator> communicator = [current.adapter getCommunicator];
    id<ICEObjectAdapter> adapter =
        [communicator createObjectAdapterWithEndpoints:@"TransientTestAdapter" endpoints:@"default"];
    [adapter activate];
    [adapter destroy];
}

-(void) deactivate:(ICECurrent*)current
{
    [current.adapter deactivate];
    [NSThread sleepForTimeInterval:1];
}
@end
