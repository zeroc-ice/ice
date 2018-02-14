// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <MetricsTest.h>

#import <Foundation/Foundation.h>

@interface UpdateCallbackI : NSObject<ICEPropertiesAdminUpdateCallback>
{
@private
BOOL updated;
id<ICEPropertiesAdminPrx> serverProps;
NSCondition* cond;
}
-(id) initWithServerProps:(id<ICEPropertiesAdminPrx>)serverProps;
-(void) waitForUpdate;
-(void) updated:(ICEMutablePropertyDict*)properties;
@end

@implementation UpdateCallbackI
-(id) initWithServerProps:(id<ICEPropertiesAdminPrx>)serverProps_
{
    self = [super init];
    if(self)
    {
        self->serverProps = ICE_RETAIN(serverProps_);
        self->cond = [[NSCondition alloc] init];
    }
    return self;
}

-(void) waitForUpdate
{
    [cond lock];
    while(!updated)
    {
        [cond wait];
    }
    // Ensure that the previous updates were committed, the setProperties call returns before 
    // notifying the callbacks so to ensure all the update callbacks have be notified we call
    // a second time, this will block until all the notifications from the first update have
    // completed.
    [serverProps setProperties:[ICEPropertyDict dictionary]];
    updated = NO;
    [cond unlock];
}

-(void) updated:(ICEMutablePropertyDict*)properties
{
    [cond lock];
    updated = YES;
    [cond signal];
    [cond unlock];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [serverProps release];
    [cond release];
    [super dealloc];
}
#endif
@end

@interface Callback : NSObject
{
BOOL wait;
NSCondition* cond;
}

-(id) init;
+(id) callback;

-(void) response;
-(void) exception:(ICEException*)exception;
-(void) waitForResponse;
@end

@implementation Callback
-(id) init
{
    self = [super init];
    if(self)
    {
        wait = true;
        cond = [[NSCondition alloc] init];
    }
    return self;
}

+(id) callback
{
    return ICE_AUTORELEASE([[Callback alloc] init]);
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [cond release];
    [super dealloc];
}
#endif

-(void) response
{
    [cond lock];
    wait = false;
    [cond signal];
    [cond unlock];
}

-(void) exception:(ICEException*)exception
{
    [self response];
}

-(void) waitForResponse
{
    [cond lock];
    while(wait)
    {
        [cond wait];
    }
    wait = true;
    [cond unlock];
}
@end

@interface Operation : NSObject
{
@protected
    ICEObjectPrx* proxy;
}
-(id) init:(ICEObjectPrx*)proxy_;
@end

@implementation Operation
-(id) init:(ICEObjectPrx*)proxy_
{
    self = [super init];
    if(self)
    {
        self->proxy = ICE_RETAIN(proxy_);
    }
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->proxy release];
    [super dealloc];
}
#endif
@end

@interface Connect : Operation
-(id) init:(ICEObjectPrx*)proxy_;
+(id) connect:(ICEObjectPrx*)proxy_;
-(void) run;
@end

@implementation Connect
-(id) init:(ICEObjectPrx*)proxy_
{
    self = [super init:proxy_];
    return self;
}

+(id) connect:(ICEObjectPrx*)proxy_
{
    return ICE_AUTORELEASE([[Connect alloc] init:proxy_]);
}

-(void) run
{
    if([proxy ice_getCachedConnection])
    {
        [[proxy ice_getCachedConnection] close:NO];
    }
    @try
    {
        [proxy ice_ping];
    }
    @catch(ICELocalException*)
    {
    }
    if([proxy ice_getCachedConnection])
    {
        [[proxy ice_getCachedConnection] close:NO];
    }
}
@end


@interface InvokeOp : Operation
-(id) init:(ICEObjectPrx*)proxy_;
+(id) invokeOp:(ICEObjectPrx*)proxy_;
-(void) run;
@end

@implementation InvokeOp
-(id) init:(ICEObjectPrx*)proxy_
{
    self = [super init:proxy_];
    return self;
}

+(id) invokeOp:(ICEObjectPrx*)proxy_
{
    return ICE_AUTORELEASE([[InvokeOp alloc] init:proxy_]);
}

-(void) run
{
    ICEMutableContext* ctx = [ICEMutableContext dictionary];
    [ctx setObject:@"test" forKey:@"entry1"];
    [ctx setObject:@"" forKey:@"entry2"];
    TestMetricsMetricsPrx* metrics = [TestMetricsMetricsPrx uncheckedCast:proxy];
    [metrics op:ctx];
}
@end

void
waitForCurrent(ICEMXMetricsAdminPrx* metrics, NSString* viewName, NSString* map, int value)
{
    while(true)
    {
        ICELong timestamp;
        ICEMXMetricsView* view = [metrics getMetricsView:viewName timestamp:&timestamp];
        test([view objectForKey:map] != nil);

        bool ok = true;
        ICEMXMetricsMap* mmap = [view objectForKey:map];
        for(ICEMXMetrics* m in mmap)
        {
            if(m.current != value)
            {
                ok = false;
                break;
            }
        }

        if(ok)
        {
            break;
        }
        [NSThread sleepForTimeInterval:50 / 1000.0];
    }
}

ICEMutablePropertyDict*
getClientProps(id<ICEPropertiesAdminPrx> p, ICEMutablePropertyDict* orig, NSString* m)
{
    ICEMutablePropertyDict* props = [p getPropertiesForPrefix:@"IceMX.Metrics"];
    ICEPropertyDict* cprops = ICE_AUTORELEASE([props copy]);
    [cprops enumerateKeysAndObjectsUsingBlock: ^(id key, id obj, BOOL *stop)
        {
            [props setObject:@"" forKey:key];
        }];

    [orig enumerateKeysAndObjectsUsingBlock: ^(id key, id obj, BOOL *stop)
        {
            [props setObject:obj forKey:key];
        }];

    NSString* map = @"";
    if(![m isEqualToString:@""])
    {
        map = [map stringByAppendingFormat:@"Map.%@.", m];
    }
    [props setObject:@"Ice\\.Admin"
              forKey:[NSString stringWithFormat:@"IceMX.Metrics.View.%@Reject.parent", map]];

    [props setObject:@"12010"
              forKey:[NSString stringWithFormat:@"IceMX.Metrics.View.%@Accept.endpointPort", map] ];

    [props setObject:@".*/admin|controller"
              forKey:[NSString stringWithFormat:@"IceMX.Metrics.View.%@Reject.identity", map]];
    return props;
}

ICEMutablePropertyDict*
getServerProps(id<ICEPropertiesAdminPrx> p, ICEMutablePropertyDict* orig, NSString* m)
{
    ICEMutablePropertyDict* props = [p getPropertiesForPrefix:@"IceMX.Metrics"];
    ICEPropertyDict* sprops = ICE_AUTORELEASE([props copy]);
    [sprops enumerateKeysAndObjectsUsingBlock: ^(id key, id obj, BOOL *stop)
        {
            [props setObject:@"" forKey:key];
        }];

    [orig enumerateKeysAndObjectsUsingBlock: ^(id key, id obj, BOOL *stop)
        {
            [props setObject:obj forKey:key];
        }];

    NSString* map = @"";
    if(![m isEqualToString:@""])
    {
        map = [map stringByAppendingFormat:@"Map.%@.", m];
    }
    [props setObject:@"Ice\\.Admin|Controller" 
              forKey:[NSString stringWithFormat:@"IceMX.Metrics.View.%@Reject.parent", map]];

    [props setObject:@"12010" 
              forKey:[NSString stringWithFormat:@"IceMX.Metrics.View.%@Accept.endpointPort", map]];

    [props setObject:@".*/admin|controller" 
              forKey:[NSString stringWithFormat:@"IceMX.Metrics.View.%@Reject.identity", map]];
    return props;
}

ICEMXConnectionMetrics*
getServerConnectionMetrics(ICEMXMetricsAdminPrx* metrics, ICELong expected)
{
    ICEMXConnectionMetrics* s;
    int nRetry = 30;
    ICELong timestamp;
    s = (ICEMXConnectionMetrics*)
                [[[metrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];
    while(s.sentBytes != expected && nRetry-- > 0)
    {
        // On some platforms, it's necessary to wait a little before obtaining the server metrics
        // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
        // to the operation is sent and getMetricsView can be dispatched before the metric is really
        // updated.
        [NSThread sleepForTimeInterval:100 / 1000.0];
        s = (ICEMXConnectionMetrics*)
                [[[metrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];
    }
    return s;
}

void
updateProps(id<ICEPropertiesAdminPrx> cprops, 
            id<ICEPropertiesAdminPrx> sprops, 
            UpdateCallbackI* callback, 
            ICEMutablePropertyDict* props,
            NSString* map)
{
    [cprops setProperties:getClientProps(cprops, props, map)];
    [sprops setProperties:getServerProps(sprops, props, map)];
    [callback waitForUpdate];
}

void
testAttribute(ICEMXMetricsAdminPrx* metrics, 
              ICEPropertiesAdminPrx* props, 
              UpdateCallbackI* update,
              NSString* map, 
              NSString* attr,
              NSString* value,
              id func)
{
    ICEMutablePropertyDict* dict = [ICEMutablePropertyDict dictionary];
    [dict setObject:attr forKey:[NSString stringWithFormat:@"IceMX.Metrics.View.Map.%@.GroupBy", map]];
    if([[props ice_getIdentity].category isEqualToString:@"client"])
    {
        [props setProperties:getClientProps(props, dict, map)];
        [update waitForUpdate];
    }
    else
    {
        [props setProperties:getServerProps(props, dict, map)];
        [props setProperties:[ICEPropertyDict dictionary]];
    }
    [func run];
    ICELong timestamp;
    ICEMXMetricsView* view = [metrics getMetricsView:@"View" timestamp:&timestamp];
    if([view objectForKey:map] == nil || [[view objectForKey:map] count] == 0)
    {
        if(![value isEqualToString:@""])
        {
            test(NO);
        }
    }
    else if([[view objectForKey:map] count] != 1 ||
            ![((ICEMXMetrics*)[[view objectForKey:map] objectAtIndex:0]).id_ isEqualToString:value])
    {
        test(NO);
    }

    [dict removeAllObjects];
    if([[props ice_getIdentity].category isEqualToString:@"client"])
    {
        [props setProperties:getClientProps(props, dict, map)];
        [update waitForUpdate];
    }
    else
    {
        [props setProperties:getServerProps(props, dict, map)];
        [props setProperties:[ICEPropertyDict dictionary]];
    }
}

void
clearView(ICEPropertiesAdminPrx* cprops, ICEPropertiesAdminPrx* sprops, UpdateCallbackI* callback)
{
    ICEMutablePropertyDict* dict = [cprops getPropertiesForPrefix:@"IceMX.Metrics"];
    [dict setObject:@"1" forKey:@"IceMX.Metrics.View.Disabled"];
    [cprops setProperties:dict];

    dict = [sprops getPropertiesForPrefix:@"IceMX.Metrics"];
    [dict setObject:@"1" forKey:@"IceMX.Metrics.View.Disabled"];
    [sprops setProperties:dict];

    [callback waitForUpdate];

    dict = [cprops getPropertiesForPrefix:@"IceMX.Metrics"];
    [dict setObject:@"" forKey:@"IceMX.Metrics.View.Disabled"];
    [cprops setProperties:dict];

    dict = [sprops getPropertiesForPrefix:@"IceMX.Metrics"];
    [dict setObject:@"" forKey:@"IceMX.Metrics.View.Disabled"];
    [sprops setProperties:dict];

    [callback waitForUpdate];
}

void
checkFailure(ICEMXMetricsAdminPrx* m, NSString* map, NSString* id_, NSString* failure, int count)
{
    ICEMXMetricsFailures* f = [m getMetricsFailures:@"View" map:map id:id_];
    if([f.failures objectForKey:failure] == nil)
    {
        NSLog(@"couldn't find failure `%@' for `%@'", failure, id_);
        test(NO);
    }
    if(count > 0 && [[f.failures objectForKey:failure] intValue] != count)
    {
        NSLog(@"count for failure `%@' of `%@' is different from expected: ", failure, id_);
        NSLog(@"%i != %@", count, [f.failures objectForKey:failure]);
        test(NO);
    }
}

NSMutableDictionary*
toMap(ICEMXMetricsMap* mmap)
{
    NSMutableDictionary* m = [NSMutableDictionary dictionary];
    for(ICEMXMetrics* metrics in mmap)
    {
        [m setObject:metrics forKey:metrics.id_];
    }
    return m;
}

id<TestMetricsMetricsPrx>
metricsAllTests(id<ICECommunicator> communicator)
{
    id<TestMetricsMetricsPrx> metrics = [TestMetricsMetricsPrx checkedCast:
                                                            [communicator stringToProxy:@"metrics:default -p 12010"]];

    tprintf("testing metrics admin facet checkedCast... ");
    id<ICEObjectPrx> admin = [communicator getAdmin];
    id<ICEPropertiesAdminPrx> clientProps = [ICEPropertiesAdminPrx checkedCast:admin facet:@"Properties"];

    id<ICEMXMetricsAdminPrx> clientMetrics = [ICEMXMetricsAdminPrx checkedCast:admin facet:@"Metrics"];
    test(clientProps && clientMetrics);

    admin = [metrics getAdmin];
    id<ICEPropertiesAdminPrx> serverProps = [ICEPropertiesAdminPrx checkedCast:admin facet:@"Properties"];
    id<ICEMXMetricsAdminPrx> serverMetrics = [ICEMXMetricsAdminPrx checkedCast:admin facet:@"Metrics"];
    test(serverProps && serverMetrics);

    UpdateCallbackI* update = ICE_AUTORELEASE([[UpdateCallbackI alloc] initWithServerProps:serverProps]);
    id<ICENativePropertiesAdmin> nativePropertiesAdmin = 
                                            (id<ICENativePropertiesAdmin>)[communicator findAdminFacet:@"Properties"];
    [nativePropertiesAdmin addUpdateCallback:update];
    tprintf("ok\n");

    ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
    tprintf("testing group by none...");

    [props setObject:@"none" forKey:@"IceMX.Metrics.View.GroupBy"];

    updateProps(clientProps, serverProps, update, props, @"");

    int threadCount = 4;

    ICELong timestamp = 0;
    ICEMXMetricsView* view = [clientMetrics getMetricsView:@"View" timestamp:&timestamp];

    test([[view objectForKey:@"Connection"] count] == 1 &&
         [[[view objectForKey:@"Connection"] objectAtIndex:0] current] == 1 &&
         [[[view objectForKey:@"Connection"] objectAtIndex:0] total] == 1);

    test([[view objectForKey:@"Thread"] count] == 1);
    test([[[view objectForKey:@"Thread"] objectAtIndex:0] current] == threadCount);
    test([[[view objectForKey:@"Thread"] objectAtIndex:0] total] == threadCount);

    tprintf("ok\n");

    tprintf("testing connection metrics... ");

    [props setObject:@"none" forKey:@"IceMX.Metrics.View.Map.Connection.GroupBy"];
    updateProps(clientProps, serverProps, update, props, @"Connection");

    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] count] == 1);
    test([[[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] count] == 1);

    [metrics ice_ping];

    ICEMXConnectionMetrics *cm1, *sm1, *cm2, *sm2;
    cm1 = (ICEMXConnectionMetrics*)
        [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];

    sm1 = (ICEMXConnectionMetrics*)
        [[[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];

    sm1 = getServerConnectionMetrics(serverMetrics, 25);

    test(cm1.total == 1 && sm1.total == 1);

    [metrics ice_ping];

    cm2 = (ICEMXConnectionMetrics*)
            [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];
    sm2 = getServerConnectionMetrics(serverMetrics, 50);

    test(cm2.sentBytes - cm1.sentBytes == 45); // 45 for ice_ping request
    test(cm2.receivedBytes - cm1.receivedBytes == 25); // 25 bytes for ice_ping response
    test(sm2.receivedBytes - sm1.receivedBytes == 45);
    test(sm2.sentBytes - sm1.sentBytes == 25);

    cm1 = cm2;
    sm1 = sm2;

    TestMetricsMutableByteSeq* bs = [TestMetricsMutableByteSeq dataWithLength:0];
    [metrics opByteS:bs];

    cm2 = (ICEMXConnectionMetrics*)
            [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];

    sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + cm2.receivedBytes - cm1.receivedBytes);
    ICELong requestSz = cm2.sentBytes - cm1.sentBytes;
    ICELong replySz = cm2.receivedBytes - cm1.receivedBytes;

    cm1 = cm2;
    sm1 = sm2;

    [bs setLength:456];
    [metrics opByteS:bs];
    
    cm2 = (ICEMXConnectionMetrics*)
        [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];
    sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + replySz);

    // 4 is for the seq variable size
    test(cm2.sentBytes - cm1.sentBytes == requestSz + [bs length] + 4);
    test(cm2.receivedBytes - cm1.receivedBytes == replySz);
    test(sm2.receivedBytes - sm1.receivedBytes == requestSz + [bs length] + 4);
    test(sm2.sentBytes - sm1.sentBytes == replySz);

    cm1 = cm2;
    sm1 = sm2;

    [bs setLength:(1024 * 1024 * 10)]; // Try with large amount of data which should be sent in several chunks
    [metrics opByteS:bs];

    cm2 = (ICEMXConnectionMetrics*)
        [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];
    sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + replySz);

    // 4 is for the seq variable size
    test(cm2.sentBytes - cm1.sentBytes == requestSz + [bs length] + 4);
    test(cm2.receivedBytes - cm1.receivedBytes == replySz);
    test(sm2.receivedBytes - sm1.receivedBytes == requestSz + [bs length] + 4);
    test(sm2.sentBytes - sm1.sentBytes == replySz);

    [props setObject:@"state" forKey:@"IceMX.Metrics.View.Map.Connection.GroupBy"];
    updateProps(clientProps, serverProps, update, props, @"Connection");

    NSMutableDictionary* map = 
        toMap([[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"]);

    test([[map objectForKey:@"active"] current] == 1);

    TestMetricsControllerPrx* controller = 
        [TestMetricsControllerPrx checkedCast:[communicator stringToProxy:@"controller:default -p 12011"]];
    
    [controller hold];

    map = toMap([[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"]);
    test([[map objectForKey:@"active"] current] == 1);
    map = toMap([[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"]);
    test([[map objectForKey:@"holding"] current] == 1);

    [[metrics ice_getConnection] close:false];

    map = toMap([[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"]);
    test([[map objectForKey:@"closing"] current] == 1);
    map = toMap([[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"]);
    test([[map objectForKey:@"holding"] current] == 1);

    [controller resume];

    map = toMap([[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"]);
    test([[map objectForKey:@"holding"] current] == 0);

    [props setObject:@"none" forKey:@"IceMX.Metrics.View.Map.Connection.GroupBy"];
    updateProps(clientProps, serverProps, update, props, @"Connection");

    [[metrics ice_getConnection] close:false];

    [[metrics ice_timeout:500] ice_ping];
    [controller hold];
    @try
    {
        [[metrics ice_timeout:500] opByteS:[NSMutableData dataWithLength:10000000]];
        test(NO);
    }
    @catch(const ICETimeoutException*)
    {
    }
    [controller resume];

    cm1 = (ICEMXConnectionMetrics*)
            [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];
    while(true)
    {
        sm1 = (ICEMXConnectionMetrics*)
            [[[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Connection"] objectAtIndex:0];
        if(sm1.failures >= 2)
        {
            break;
        }
        [NSThread sleepForTimeInterval:10 / 1000.0];
    }

    test(cm1.failures == 2 && sm1.failures >= 1);

    checkFailure(clientMetrics, @"Connection", cm1.id_, @"Ice::TimeoutException", 1);
    checkFailure(clientMetrics, @"Connection", cm1.id_, @"Ice::ConnectTimeoutException", 1);
    checkFailure(serverMetrics, @"Connection", sm1.id_, @"Ice::ConnectionLostException", 0);

    ICEMXMetricsPrx* m = [[metrics ice_timeout:500] ice_connectionId:@"Con1"];
    [m ice_ping];

    testAttribute(clientMetrics, clientProps, update, @"Connection", @"parent", @"Communicator", nil);
    //testAttribute(clientMetrics, clientProps, update, "Connection", "id", "");
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpoint", @"tcp -h 127.0.0.1 -p 12010 -t 500", 
                  nil);

    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpointType", @"1", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpointIsDatagram", @"false", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpointIsSecure", @"false", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpointTimeout", @"500", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpointCompress", @"false", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpointHost", @"127.0.0.1", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"endpointPort", @"12010", nil);

    testAttribute(clientMetrics, clientProps, update, @"Connection", @"incoming", @"false", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"adapterName", @"", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"connectionId", @"Con1", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"localHost", @"127.0.0.1", nil);
    //testAttribute(clientMetrics, clientProps, update, "Connection", "localPort", "");
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"remoteHost", @"127.0.0.1", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"remotePort", @"12010", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"mcastHost", @"", nil);
    testAttribute(clientMetrics, clientProps, update, @"Connection", @"mcastPort", @"", nil);
    
    [[m ice_getConnection] close:false];

    waitForCurrent(clientMetrics, @"View", @"Connection", 0);
    waitForCurrent(serverMetrics, @"View", @"Connection", 0);

    tprintf("ok\n");

    tprintf("testing connection establishment metrics... ");

    [props setObject:@"id" forKey:@"IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"];
    updateProps(clientProps, serverProps, update, props, @"ConnectionEstablishment");
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] 
              objectForKey:@"ConnectionEstablishment"] count] == 0);

    [metrics ice_ping];
    
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp]
              objectForKey:@"ConnectionEstablishment"] count] == 1);
    ICEMXMetrics* m1 = [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] 
                                                            objectForKey:@"ConnectionEstablishment"] objectAtIndex:0];

    test(m1.current == 0 && m1.total == 1 && [m1.id_ isEqualToString:@"127.0.0.1:12010"]);

    [[metrics ice_getConnection] close:NO];
    [controller hold];
    @try
    {
        [[[communicator stringToProxy:@"test:tcp -p 12010 -h 127.0.0.1"] ice_timeout:10] ice_ping];
        test(NO);
    }
    @catch(ICEConnectTimeoutException*)
    {
    }
    @catch(const ICELocalException*)
    {
        test(NO);
    }
    [controller resume];
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp]
              objectForKey:@"ConnectionEstablishment"] count] == 1);
    m1 = [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] 
              objectForKey:@"ConnectionEstablishment"] objectAtIndex:0];
    test([m1.id_ isEqualToString:@"127.0.0.1:12010"] && m1.total == 3 && m1.failures == 2);

    checkFailure(clientMetrics, @"ConnectionEstablishment", m1.id_, @"Ice::ConnectTimeoutException", 2);

    Connect* c = [Connect connect:metrics];

    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"parent", @"Communicator", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"id", @"127.0.0.1:12010", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpoint",  
                  @"tcp -h 127.0.0.1 -p 12010 -t 60000", c);

    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpointType", @"1", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpointIsDatagram", @"false", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpointIsSecure", @"false", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpointTimeout", @"60000", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpointCompress", @"false", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpointHost", @"127.0.0.1", c);
    testAttribute(clientMetrics, clientProps, update, @"ConnectionEstablishment", @"endpointPort", @"12010", c);

    tprintf("ok\n");
    
    //
    // Ice doesn't do any endpoint lookup with WinRT, the WinRT
    // runtime takes care of if.
    //
    // In iOS we use CFStream transports that doesn't do any enpoint
    // lookup.
    //
#if !defined(ICE_OS_WINRT) && (!defined(__APPLE__) || (defined(__APPLE__) && !TARGET_OS_IPHONE))
    tprintf("testing endpoint lookup metrics... ");

    [props setObject:@"id" forKey:@"IceMX.Metrics.View.Map.EndpointLookup.GroupBy"];
    updateProps(clientProps, serverProps, update, props, @"EndpointLookup");
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"EndpointLookup"] count] == 0);

    ICEObjectPrx* prx = [communicator stringToProxy:@"metrics:default -p 12010 -h localhost -t infinite"];
    [prx ice_ping];
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"EndpointLookup"] count] == 1);
    m1 = [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"EndpointLookup"] objectAtIndex:0];

    test(m1.current <= 1 && m1.total == 1 && [m1.id_ isEqualToString:@"tcp -h localhost -p 12010 -t infinite"]);

    [[prx ice_getConnection] close:NO];

    BOOL dnsException = NO;
    @try
    {
        [[communicator stringToProxy:@"test:tcp -t 500 -p 12010 -h unknownfoo.zeroc.com"] ice_ping];
        test(NO);
    }
    @catch(const ICEDNSException*)
    {
        dnsException = YES;
    }
    @catch(ICELocalException*)
    {
        // Some DNS servers don't fail on unknown DNS names.
    }
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"EndpointLookup"] count] == 2);
    m1 = [[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"EndpointLookup"] objectAtIndex:1];
    test([m1.id_ isEqualToString:@"tcp -h unknownfoo.zeroc.com -p 12010 -t 500"] && m1.total == 2 && 
         (!dnsException || m1.failures == 2));
    if(dnsException)
    {
        checkFailure(clientMetrics, @"EndpointLookup", m1.id_, @"Ice::DNSException", 2);
    }

    c = [Connect connect:prx];
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"parent", @"Communicator", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"id", 
                  @"tcp -h localhost -p 12010 -t infinite", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpoint", 
                  @"tcp -h localhost -p 12010 -t infinite", c);

    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpointType", @"1", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpointIsDatagram", @"false", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpointIsSecure", @"false", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpointTimeout", @"-1", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpointCompress", @"false", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpointHost", @"localhost", c);
    testAttribute(clientMetrics, clientProps, update, @"EndpointLookup", @"endpointPort", @"12010", c);

    tprintf("ok\n");
#endif

    tprintf("testing dispatch metrics... ");

    [props setObject:@"operation" forKey:@"IceMX.Metrics.View.Map.Dispatch.GroupBy"];
    updateProps(clientProps, serverProps, update, props, @"Dispatch");
    test([[[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Dispatch"] count] == 0);

    [metrics op];
    @try
    {
        [metrics opWithUserException];
        test(NO);
    }
    @catch(TestMetricsUserEx*)
    {
    }
    @try
    {
        [metrics opWithRequestFailedException];
        test(NO);
    }
    @catch(ICERequestFailedException*)
    {
    }
    @try
    {
        [metrics opWithLocalException];
        test(NO);
    }
    @catch(ICELocalException*)
    {
    }
    @try
    {
        [metrics opWithUnknownException];
        test(NO);
    }
    @catch(ICEUnknownException*)
    {
    }
    @try
    {
        [metrics fail];
        test(NO);
    }
    @catch(ICEConnectionLostException*)
    {
    }

    map = toMap([[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Dispatch"]);
    test([map count] == 6);

    ICEMXDispatchMetrics* dm1 = (ICEMXDispatchMetrics*)[map objectForKey:@"op"];
    test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 0 && dm1.userException == 0);
    test(dm1.size == 21 && dm1.replySize == 7);

    dm1 = (ICEMXDispatchMetrics*)[map objectForKey:@"opWithUserException"];
    test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 0 && dm1.userException == 1);
    test(dm1.size == 38 && dm1.replySize == 23);

    dm1 = (ICEMXDispatchMetrics*)[map objectForKey:@"opWithLocalException"];
    test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
    checkFailure(serverMetrics, @"Dispatch", dm1.id_, @"Ice::SyscallException", 1);
    test(dm1.size == 39 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

    dm1 = (ICEMXDispatchMetrics*)[map objectForKey:@"opWithRequestFailedException"];
    test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
    checkFailure(serverMetrics, @"Dispatch", dm1.id_, @"Ice::ObjectNotExistException", 1);
    test(dm1.size == 47 && dm1.replySize == 40);

    dm1 = (ICEMXDispatchMetrics*)[map objectForKey:@"opWithUnknownException"];
    test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
    checkFailure(serverMetrics, @"Dispatch", dm1.id_, @"IceObjC::Exception", 1);
    test(dm1.size == 41 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

    InvokeOp* op = [InvokeOp invokeOp:metrics];
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"parent", @"TestAdapter", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"id", @"metrics [op]", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpoint",
                  @"tcp -h 127.0.0.1 -p 12010 -t 60000", op);
    //testAttribute(serverMetrics, serverProps, update, "Dispatch", "connection", "", op);

    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpointType", @"1", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpointIsDatagram", @"false", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpointIsSecure", @"false", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpointTimeout", @"60000", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpointCompress", @"false", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpointHost", @"127.0.0.1", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"endpointPort", @"12010", op);

    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"incoming", @"true", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"adapterName", @"TestAdapter", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"connectionId", @"", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"localHost", @"127.0.0.1", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"localPort", @"12010", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"remoteHost", @"127.0.0.1", op);
    //testAttribute(serverMetrics, serverProps, update, "Dispatch", "remotePort", "12010", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"mcastHost", @"", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"mcastPort", @"", op);

    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"operation", @"op", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"identity", @"metrics", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"facet", @"", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"mode", @"twoway", op);

    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"context.entry1", @"test", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"context.entry2", @"", op);
    testAttribute(serverMetrics, serverProps, update, @"Dispatch", @"context.entry3", @"", op);

    tprintf("ok\n");

    tprintf("testing invocation metrics... ");

    [props setObject:@"operation" forKey:@"IceMX.Metrics.View.Map.Invocation.GroupBy"];
    [props setObject:@"localPort" forKey:@"IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"];
    updateProps(clientProps, serverProps, update, props, @"Invocation");
    test([[[serverMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Invocation"] count] == 0);

    Callback* cb = [Callback callback];

    void(^responseCB)() = ^()
        {
            [cb response];
        };

    void(^exceptionCB)(ICEException*) = ^(ICEException* ex)
        {
            [cb exception:ex];
        };

    [metrics op];
    [metrics end_op:[metrics begin_op]];
    [metrics begin_op:responseCB exception:exceptionCB];
    [cb waitForResponse];

    // User exception
    @try
    {
        [metrics opWithUserException];
        test(NO);
    }
    @catch(TestMetricsUserEx*)
    {
    }
    @try
    {
        [metrics end_opWithUserException:[metrics begin_opWithUserException]];
        test(NO);
    }
    @catch(const TestMetricsUserEx*)
    {
    }
    [metrics begin_opWithUserException:responseCB exception:exceptionCB];
    [cb waitForResponse];

    // Request failed exception
    @try
    {
        [metrics opWithRequestFailedException];
        test(NO);
    }
    @catch(ICERequestFailedException*)
    {
    }
    @try
    {
        [metrics end_opWithRequestFailedException:[metrics begin_opWithRequestFailedException]];
        test(NO);
    }
    @catch(const ICERequestFailedException*)
    {
    }
    [metrics begin_opWithRequestFailedException:responseCB exception:exceptionCB];
    [cb waitForResponse];

    // Local exception
    @try
    {
        [metrics opWithLocalException];
        test(NO);
    }
    @catch(ICELocalException*)
    {
    }
    @try
    {
        [metrics end_opWithLocalException:[metrics begin_opWithLocalException]];
        test(NO);
    }
    @catch(ICELocalException*)
    {
    }
    [metrics begin_opWithLocalException:responseCB exception:exceptionCB];
    [cb waitForResponse];

    // Unknown exception
    @try
    {
        [metrics opWithUnknownException];
        test(NO);
    }
    @catch(ICEUnknownException*)
    {
    }
    @try
    {
        [metrics end_opWithUnknownException:[metrics begin_opWithUnknownException]];
        test(NO);
    }
    @catch(ICEUnknownException*)
    {
    }
    [metrics begin_opWithUnknownException:responseCB exception:exceptionCB];
    [cb waitForResponse];

    // Fail
    @try
    {
        [metrics fail];
        test(NO);
    }
    @catch(ICEConnectionLostException*)
    {
    }
    @try
    {
        [metrics end_fail:[metrics begin_fail]];
        test(NO);
    }
    @catch(ICEConnectionLostException*)
    {
    }
    [metrics begin_fail:responseCB exception:exceptionCB];
    [cb waitForResponse];

    map = toMap([[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Invocation"]);
    test([map count] == 6);

    ICEMXInvocationMetrics* im1;
    ICEMXRemoteMetrics* rim1;
    im1 = (ICEMXInvocationMetrics*)[map objectForKey:@"op"];
    test(im1.current <= 1 && im1.total == 3 && im1.failures == 0 && im1.retry == 0 && [im1.remotes count] == 1);
    rim1 = (ICEMXRemoteMetrics*)[im1.remotes objectAtIndex:0];
    test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
    test(rim1.size == 63 && rim1.replySize == 21);
    
    im1 = (ICEMXInvocationMetrics*)[map objectForKey:@"opWithUserException"];
    test(im1.current <= 1 && im1.total == 3 && im1.failures == 0 && im1.retry == 0 && [im1.remotes count] == 1);
    rim1 = (ICEMXRemoteMetrics*)[im1.remotes objectAtIndex:0];
    test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
    test(rim1.size == 114 && rim1.replySize == 69);
    test(im1.userException == 3);

    im1 = (ICEMXInvocationMetrics*)[map objectForKey:@"opWithLocalException"];
    test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0 && [im1.remotes count] == 1);
    rim1 = (ICEMXRemoteMetrics*)[im1.remotes objectAtIndex:0];
    test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
    test(rim1.size == 117 && rim1.replySize > 7);
    checkFailure(clientMetrics, @"Invocation", im1.id_, @"Ice::UnknownLocalException", 3);

    im1 = (ICEMXInvocationMetrics*)[map objectForKey:@"opWithRequestFailedException"];
    test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0 && [im1.remotes count] == 1);
    rim1 = (ICEMXRemoteMetrics*)[im1.remotes objectAtIndex:0];
    test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
    test(rim1.size == 141 && rim1.replySize == 120);
    checkFailure(clientMetrics, @"Invocation", im1.id_, @"Ice::ObjectNotExistException", 3);

    im1 = (ICEMXInvocationMetrics*)[map objectForKey:@"opWithUnknownException"];
    test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0 && [im1.remotes count] == 1);
    rim1 = (ICEMXRemoteMetrics*)[im1.remotes objectAtIndex:0];
    test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
    checkFailure(clientMetrics, @"Invocation", im1.id_, @"Ice::UnknownException", 3);

    im1 = (ICEMXInvocationMetrics*)[map objectForKey:@"fail"];
    test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 3 && [im1.remotes count] == 6);

    test(((ICEMXMetrics*)[im1.remotes objectAtIndex:0]).current == 0 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:0]).total == 1 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:0]).failures == 1);

    test(((ICEMXMetrics*)[im1.remotes objectAtIndex:1]).current == 0 &&
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:1]).total == 1 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:1]).failures == 1);

    test(((ICEMXMetrics*)[im1.remotes objectAtIndex:2]).current == 0 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:2]).total == 1 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:2]).failures == 1);

    test(((ICEMXMetrics*)[im1.remotes objectAtIndex:3]).current == 0 &&
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:3]).total == 1 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:3]).failures == 1);

    test(((ICEMXMetrics*)[im1.remotes objectAtIndex:4]).current == 0 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:4]).total == 1 &&
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:4]).failures == 1);

    test(((ICEMXMetrics*)[im1.remotes objectAtIndex:5]).current == 0 && 
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:5]).total == 1 &&
         ((ICEMXMetrics*)[im1.remotes objectAtIndex:5]).failures == 1);

    checkFailure(clientMetrics, @"Invocation", im1.id_, @"Ice::ConnectionLostException", 3);

    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"parent", @"Communicator", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"id", @"metrics -t -e 1.1 [op]", op);

    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"operation", @"op", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"identity", @"metrics", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"facet", @"", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"encoding", @"1.1", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"mode", @"twoway", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"proxy", 
                  @"metrics -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 60000", op);

    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"context.entry1", @"test", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"context.entry2", @"", op);
    testAttribute(clientMetrics, clientProps, update, @"Invocation", @"context.entry3", @"", op);

    tprintf("ok\n");

    tprintf("testing metrics view enable/disable...");


    [props setObject:@"none" forKey:@"IceMX.Metrics.View.GroupBy"];
    [props setObject:@"0" forKey:@"IceMX.Metrics.View.Disabled"];
    updateProps(clientProps, serverProps, update, props, @"Thread");
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Thread"] count] > 0);

    ICEMutableStringSeq* disabledViews;
    ICEMutableStringSeq* names = [clientMetrics getMetricsViewNames:&disabledViews];
    test([names count] == 1 && [disabledViews count] == 0);

    [props setObject:@"1" forKey:@"IceMX.Metrics.View.Disabled"];
    updateProps(clientProps, serverProps, update, props, @"Thread");
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Thread"] count] == 0);
    test([[clientMetrics getMetricsViewNames:&disabledViews] count] == 0 && [disabledViews count] == 1);

    [clientMetrics enableMetricsView:@"View"];
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Thread"] count] > 0);
    test([[clientMetrics getMetricsViewNames:&disabledViews] count] == 1 && [disabledViews count] == 0);

    [clientMetrics disableMetricsView:@"View"];
    test([[[clientMetrics getMetricsView:@"View" timestamp:&timestamp] objectForKey:@"Thread"] count] == 0);
    test([[clientMetrics getMetricsViewNames:&disabledViews] count] == 0 && [disabledViews count] == 1);

    @try
    {
        [clientMetrics enableMetricsView:@"UnknownView"];
    }
    @catch(ICEMXUnknownMetricsView*)
    {
    }

    tprintf("ok\n");

    return metrics;
}
