// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
