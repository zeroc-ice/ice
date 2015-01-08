// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <binding/TestI.h>

@implementation RemoteCommunicatorI
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    nextPort_ = 10001;
    return self;
}


-(id<TestBindingRemoteObjectAdapterPrx>) createObjectAdapter:(NSMutableString*)name endpoints:(NSMutableString*)endpts
                                              current:(ICECurrent*)current
{
    id<ICECommunicator> com = [current.adapter getCommunicator];
    [[com getProperties] setProperty:[name stringByAppendingString:@".ThreadPool.Size"] value:@"1"];
    id<ICEObjectAdapter> adapter = [com createObjectAdapterWithEndpoints:name endpoints:endpts];
#if defined(__clang__) && !__has_feature(objc_arc)
    RemoteObjectAdapterI* remote = [[[RemoteObjectAdapterI alloc] initWithAdapter:adapter] autorelease];
#else
    RemoteObjectAdapterI* remote = [[RemoteObjectAdapterI alloc] initWithAdapter:adapter];
#endif
    return [TestBindingRemoteObjectAdapterPrx uncheckedCast:[current.adapter addWithUUID:remote]];
}

-(void) deactivateObjectAdapter:(id<TestBindingRemoteObjectAdapterPrx>)adapter current:(ICECurrent*)current
{
    [adapter deactivate]; // Collocated call
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end

@implementation RemoteObjectAdapterI
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
#if defined(__clang__) && !__has_feature(objc_arc)
    adapter_ = [adapter retain];
    testIntf_ = [TestBindingTestIntfPrx uncheckedCast:[adapter_ add:[[[TestBindingI alloc] init] autorelease]
                                                    identity:[[adapter_ getCommunicator] stringToIdentity:@"test"]]];
    [testIntf_ retain];
#else
    adapter_ = adapter;
    testIntf_ = [TestBindingTestIntfPrx uncheckedCast:[
                adapter_ add:[[TestBindingI alloc] init]
                    identity:[[adapter_ getCommunicator] stringToIdentity:@"test"]]];
#endif
    [adapter_ activate];
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [testIntf_ release];
    [adapter_ release];
    [super dealloc];
}
#endif

-(id<TestBindingTestIntfPrx>) getTestIntf:(ICECurrent*)current
{
    return testIntf_;
}

-(void) deactivate:(ICECurrent*)current
{
    @try
    {
        [adapter_ destroy];
    }
    @catch(ICEObjectAdapterDeactivatedException*)
    {
    }
}
@end

@implementation TestBindingI
-(NSString*) getAdapterName:(ICECurrent*)current
{
    return [current.adapter getName];
}
@end
