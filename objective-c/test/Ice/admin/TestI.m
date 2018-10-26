// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <admin/TestI.h>

//
// A no-op Logger, used when testing the Logger Admin
//
@interface NullLogger : ICELocalObject<ICELogger>
@end

@implementation NullLogger
-(void) print:(NSString*)__unused message
{
}
-(void) trace:(NSString*)__unused category message:(NSString*)__unused message
{
}
-(void) warning:(NSString*)__unused message
{
}
-(void) error:(NSString*)__unused message
{
}
-(NSMutableString*) getPrefix
{
    return ICE_AUTORELEASE([@"NullLogger" mutableCopy]);
}
-(id<ICELogger>) cloneWithPrefix:(NSString*)__unused prefix
{
    return self;
}
@end

@implementation TestAdminRemoteCommunicatorI

+(id) remoteCommunicator:(id<ICECommunicator>)communicator
{
    TestAdminRemoteCommunicatorI* obj = [TestAdminRemoteCommunicatorI remoteCommunicator];
    obj->_communicator = ICE_RETAIN(communicator);
    obj->_called = NO;
    obj->_cond = [[NSCondition alloc] init];
    return obj;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_communicator release];
    [_cond release];
    [_changes release];
    [super dealloc];
}
#endif
-(id<ICEObjectPrx>) getAdmin:(ICECurrent*)__unused current
{
    return [_communicator getAdmin];
}

-(ICEPropertyDict*) getChanges:(ICECurrent*)__unused current
{
    [_cond lock];
    @try
    {
        //
        // The client calls PropertiesAdmin::setProperties() and then invokes
        // this operation. Since setProperties() is implemented using AMD, the
        // client might receive its reply and then call getChanges() before our
        // updated() method is called. We block here to ensure that updated()
        // gets called before we return the most recent set of changes.
        //
        while(!_called)
        {
            [_cond wait];
        }
        _called = NO;
        return _changes;
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) print:(NSString*)message current:(ICECurrent*)__unused current
{
    [[_communicator getLogger] print:message];
}
-(void) trace:(NSString*)category message:(NSString*)message current:(ICECurrent*)__unused current
{
    [[_communicator getLogger] trace:category message:message];
}
-(void) warning:(NSString*)message current:(ICECurrent*)__unused current
{
    [[_communicator getLogger] warning:message];
}
-(void) error:(NSString*)message current:(ICECurrent*)__unused current
{
    [[_communicator getLogger] error:message];
}
-(void) shutdown:(ICECurrent*)__unused current
{
    [_communicator shutdown];
}
-(void) waitForShutdown:(ICECurrent*)__unused current
{
    //
    // Note that we are executing in a thread of the *main* communicator,
    // not the one that is being shut down.
    //
    [_communicator waitForShutdown];
}
-(void) destroy:(ICECurrent*)__unused current
{
    [_communicator destroy];
}
-(void) updated:(ICEMutablePropertyDict*)changes
{
    [_cond lock];
    @try
    {
        ICE_RELEASE(_changes);
        _changes = ICE_RETAIN(changes);
        _called = YES;
        [_cond signal];
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@implementation TestAdminRemoteCommunicatorFactoryI
-(id<TestAdminRemoteCommunicatorPrx>) createCommunicator:(ICEMutablePropertyDict*)props current:(ICECurrent*)current
{
    //
    // Prepare the property set using the given properties.
    //
    ICEInitializationData* init = [ICEInitializationData initializationData];
    init.properties = [ICEUtil createProperties];
    for(NSString* key in props)
    {
        [init.properties setProperty:key value:[props objectForKey:key]];
    }

    if([init.properties getPropertyAsInt:@"NullLogger"] > 0)
    {
        init.logger = ICE_AUTORELEASE([NullLogger new]);
    }

    //
    // Initialize a new communicator.
    //
    id<ICECommunicator> communicator = [ICEUtil createCommunicator:init];

    //
    // Install a custom admin facet.
    //
    [communicator addAdminFacet:[TestAdminTestFacetI testFacet] facet:@"TestFacet"];

    //
    // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
    // Set the callback on the admin facet.
    //
    TestAdminRemoteCommunicatorI* servant = [TestAdminRemoteCommunicatorI remoteCommunicator:communicator];
    ICEObject* propFacet = [communicator findAdminFacet:@"Properties"];
    if(propFacet != nil)
    {
        //NSAssert([propFacet isKindOfClass:[ICENativePropertiesAdmin class]], @"");
        id<ICENativePropertiesAdmin> admin = (id<ICENativePropertiesAdmin>)propFacet;
        [admin addUpdateCallback:servant];
    }

    id<ICEObjectPrx> proxy = [current.adapter addWithUUID:servant];
    return [TestAdminRemoteCommunicatorPrx uncheckedCast:proxy];
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end

@implementation TestAdminTestFacetI
-(void) op:(ICECurrent*)__unused current
{
}
@end
