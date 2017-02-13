// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <BindingTest.h>

#import <Foundation/Foundation.h>

@interface GetAdapterNameCB : NSObject
{
    NSCondition* cond_;
    NSString* name_;
}
-(id) init;
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc;
#endif
-(void) response:(NSString*)name;
-(void) exception:(ICEException*)ex;
-(NSString*) getResult;
@end

@implementation GetAdapterNameCB
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cond_ = [[NSCondition alloc] init];
    name_ = nil;
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [cond_ release];
    [name_ release];
    [super dealloc];
}
#endif

-(void) response:(NSString*)name
{
    [cond_ lock];
    name_ = ICE_RETAIN(name);
    [cond_ signal];
    [cond_ unlock];
}
-(void) exception:(ICEException*)ex
{
    tprintf("unexpected exception: %@", ex);
    test(NO);
}
-(NSString*) getResult
{
    [cond_ lock];
    @try
    {
        while(name_ == nil)
        {
            [cond_ wait];
        }
        return ICE_AUTORELEASE(ICE_RETAIN(name_));
    }
    @finally
    {
        [cond_ unlock];
    }
    return nil;
}
@end

NSString*
getAdapterNameWithAMI(id<TestBindingTestIntfPrx> test)
{
    GetAdapterNameCB* cb = ICE_AUTORELEASE([[GetAdapterNameCB alloc] init]);
    [test begin_getAdapterName:^(NSMutableString* name) { [cb response:name]; }
                     exception:^(ICEException* ex) { [cb exception:ex]; }];
    return [cb getResult];
}

NSArray*
getEndpoints(id<TestBindingTestIntfPrx> proxy)
{
    NSMutableArray* edpts = [NSMutableArray array];
    bool escape = NO;
    int beg = 0;
    int length = 0;
    NSString* s = [proxy ice_toString];
    int index;
    for(index = 0; index < [s length]; ++index)
    {
        unichar c = [s characterAtIndex:index];
        if(c == '"')
        {
            escape = !escape;
        }

        if(!escape && c == ':')
        {
            NSRange range = { beg, length };
            [edpts addObject:[s substringWithRange:range]];
            beg = beg + length + 1;
            length = 0;
        }
        else
        {
            ++length;
        }
    }
    if(length > 0)
    {
        NSRange range = { beg, length };
        [edpts addObject:[s substringWithRange:range]];
    }
    [edpts removeObjectAtIndex:0];
    return edpts;
}

id<TestBindingTestIntfPrx>
createTestIntfPrx(NSArray* adapters)
{
    NSMutableArray* endpoints = [NSMutableArray arrayWithCapacity:[adapters count]];
    id<TestBindingTestIntfPrx> test = nil;
    for(id<TestBindingRemoteObjectAdapterPrx> a in adapters)
    {
        test = [a getTestIntf];
        [endpoints addObjectsFromArray:getEndpoints(test)];
    }
    NSString* proxy = [[test ice_getCommunicator] identityToString:[test ice_getIdentity]];
    for(NSString* e in endpoints)
    {
        proxy = [proxy stringByAppendingString:@":"];
        proxy = [proxy stringByAppendingString:e];
    }
    return [TestBindingTestIntfPrx uncheckedCast:[[test ice_getCommunicator] stringToProxy:proxy]];
}

void
deactivate(id<TestBindingRemoteCommunicatorPrx> com, NSArray* adapters)
{
    for(id<TestBindingRemoteObjectAdapterPrx> a in adapters)
    {
        [com deactivateObjectAdapter:a];
    }
}

void
random_shuffle(NSMutableArray* array)
{
    NSUInteger count = [array count];
    while(count--)
    {
        [array exchangeObjectAtIndex:count withObjectAtIndex:(random() % (count + 1))];
    }
}

void
bindingAllTests(id<ICECommunicator> communicator)
{
    NSString* ref = @"communicator:default -p 12010";
    id<TestBindingRemoteCommunicatorPrx> com = [TestBindingRemoteCommunicatorPrx uncheckedCast:[
            communicator stringToProxy:ref]];

    tprintf("testing binding with single endpoint... ");
    {
        id<TestBindingRemoteObjectAdapterPrx> adapter = [com createObjectAdapter:@"Adapter" endpoints:@"default"];

        id<TestBindingTestIntfPrx> test1 = [adapter getTestIntf];
        id<TestBindingTestIntfPrx> test2 = [adapter getTestIntf];
        test([[test1 ice_getConnection] isEqual:[test2 ice_getConnection]]);

        [test1 ice_ping];
        [test2 ice_ping];

        [com deactivateObjectAdapter:adapter];

        id<TestBindingTestIntfPrx> test3 = [TestBindingTestIntfPrx uncheckedCast:test1];
        test([[test3 ice_getConnection] isEqual:[test1 ice_getConnection]]);
        test([[test3 ice_getConnection] isEqual:[test2 ice_getConnection]]);

        @try
        {
            [test3 ice_ping];
            test(NO);
        }
        @catch(ICEConnectionRefusedException*)
        {
        }
    }
    tprintf("ok\n");

    tprintf("testing binding with multiple endpoints... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"Adapter11" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter12" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter13" endpoints:@"default"]];

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        NSMutableSet* names = [NSMutableSet setWithCapacity:3];
        [names addObject:@"Adapter11"];
        [names addObject:@"Adapter12"];
        [names addObject:@"Adapter13"];
        while([names count] != 0)
        {
            NSMutableArray* adpts = ICE_AUTORELEASE([adapters mutableCopy]);

            id<TestBindingTestIntfPrx> test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test3 = createTestIntfPrx(adpts);

            test([[test1 ice_getConnection] isEqual:[test2 ice_getConnection]]);
            test([[test2 ice_getConnection] isEqual:[test3 ice_getConnection]]);

            [names removeObject:[test1 getAdapterName]];
            [[test1 ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for(id<TestBindingRemoteObjectAdapterPrx> a in adapters)
            {
                [[a getTestIntf] ice_ping];
            }

            id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
            NSString* name = [test getAdapterName];
            const int nRetry = 10;
            int i;
            for(i = 0; i < nRetry &&  [[test getAdapterName] isEqualToString:name]; i++);
            test(i == nRetry);

            for(id<TestBindingRemoteObjectAdapterPrx> a in adapters)
            {
                [[[a getTestIntf] ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        [com deactivateObjectAdapter:[adapters objectAtIndex:0]];
        [names addObject:@"Adapter12"];
        [names addObject:@"Adapter13"];
        while([names count] != 0)
        {
            NSMutableArray* adpts = ICE_AUTORELEASE([adapters mutableCopy]);

            id<TestBindingTestIntfPrx> test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test3 = createTestIntfPrx(adpts);

            test([[test1 ice_getConnection] isEqual:[test2 ice_getConnection]]);
            test([[test2 ice_getConnection] isEqual:[test3 ice_getConnection]]);

            [names removeObject:[test1 getAdapterName]];
            [[test1 ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        [com deactivateObjectAdapter:[adapters objectAtIndex:2]];
        id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
        test([[test getAdapterName] isEqualToString:@"Adapter12"]);

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing binding with multiple endpoints and AMI... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI11" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI12" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI13" endpoints:@"default"]];

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        NSMutableSet* names = [NSMutableSet setWithCapacity:3];
        [names addObject:@"AdapterAMI11"];
        [names addObject:@"AdapterAMI12"];
        [names addObject:@"AdapterAMI13"];
        while([names count] != 0)
        {
            NSMutableArray* adpts = ICE_AUTORELEASE([adapters mutableCopy]);

            id<TestBindingTestIntfPrx> test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test3 = createTestIntfPrx(adpts);

            test([[test1 ice_getConnection] isEqual:[test2 ice_getConnection]]);
            test([[test2 ice_getConnection] isEqual:[test3 ice_getConnection]]);

            [names removeObject:getAdapterNameWithAMI(test1)];
            [[test1 ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for(id<TestBindingRemoteObjectAdapterPrx> a in adapters)
            {
                [[a getTestIntf] ice_ping];
            }

            id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
            NSString* name = getAdapterNameWithAMI(test);
            const int nRetry = 10;
            int i;
            for(i = 0; i < nRetry && [getAdapterNameWithAMI(test) isEqualToString:name]; i++);
            test(i == nRetry);

            for(id<TestBindingRemoteObjectAdapterPrx> a in adapters)
            {
                [[[a getTestIntf] ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        [com deactivateObjectAdapter:[adapters objectAtIndex:0]];
        [names addObject:@"AdapterAMI12"];
        [names addObject:@"AdapterAMI13"];
        while([names count] != 0)
        {
            NSMutableArray* adpts = ICE_AUTORELEASE([adapters mutableCopy]);

            id<TestBindingTestIntfPrx> test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts);
            id<TestBindingTestIntfPrx> test3 = createTestIntfPrx(adpts);

            test([[test1 ice_getConnection] isEqual:[test2 ice_getConnection]]);
            test([[test2 ice_getConnection] isEqual:[test3 ice_getConnection]]);

            [names removeObject:[test1 getAdapterName]];
            [[test1 ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        [com deactivateObjectAdapter:[adapters objectAtIndex:2]];
        id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
        test([[test getAdapterName] isEqualToString:@"AdapterAMI12"]);

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing random endpoint selection... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"Adapter21" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter22" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter23" endpoints:@"default"]];

        id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
        test([test ice_getEndpointSelection] == ICERandom);

        NSMutableSet* names = [NSMutableSet setWithCapacity:3];
        [names addObject:@"Adapter21"];
        [names addObject:@"Adapter22"];
        [names addObject:@"Adapter23"];
        while([names count] != 0)
        {
            [names removeObject:[test getAdapterName]];
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        }

        test = [TestBindingTestIntfPrx uncheckedCast:[test ice_endpointSelection:ICERandom]];
        test([test ice_getEndpointSelection] == ICERandom);

        [names addObject:@"Adapter21"];
        [names addObject:@"Adapter22"];
        [names addObject:@"Adapter23"];
        while([names count] != 0)
        {
            [names removeObject:[test getAdapterName]];
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        }

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing ordered endpoint selection... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"Adapter31" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter32" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter33" endpoints:@"default"]];

        id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
        test = [TestBindingTestIntfPrx uncheckedCast:[test ice_endpointSelection:ICEOrdered]];
        test([test ice_getEndpointSelection] == ICEOrdered);
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter31"]; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter31"]; i++);
        }
#endif
        test(i == nRetry);
        [com deactivateObjectAdapter:[adapters objectAtIndex:0]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter32"]; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter32"]; i++);
        }
#endif
        test(i == nRetry);
        [com deactivateObjectAdapter:[adapters objectAtIndex:1]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter33"]; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter33"]; i++);
        }
#endif
        test(i == nRetry);
        [com deactivateObjectAdapter:[adapters objectAtIndex:2]];

        @try
        {
            [test getAdapterName];
        }
        @catch(ICEConnectionRefusedException*)
        {
        }

        NSArray* endpoints = getEndpoints(test);

        adapters = [NSMutableArray arrayWithCapacity:3];

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        [adapters addObject:[com createObjectAdapter:@"Adapter36" endpoints:[endpoints objectAtIndex:2]]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter36"]; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter36"]; i++);
        }
#endif
        test(i == nRetry);
        [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        [adapters addObject:[com createObjectAdapter:@"Adapter35" endpoints:[endpoints objectAtIndex:1]]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter35"]; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter35"]; i++);
        }
#endif
        test(i == nRetry);
        [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
        [adapters addObject:[com createObjectAdapter:@"Adapter34" endpoints:[endpoints objectAtIndex:0]]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter34"]; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter34"]; i++);
        }
#endif
        test(i == nRetry);
        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing per request binding with single endpoint... ");
    {
        id<TestBindingRemoteObjectAdapterPrx> adapter = [com createObjectAdapter:@"Adapter41" endpoints:@"default"];

        id<TestBindingTestIntfPrx> test1 = [TestBindingTestIntfPrx uncheckedCast:[[adapter getTestIntf] ice_connectionCached:NO]];
        id<TestBindingTestIntfPrx> test2 = [TestBindingTestIntfPrx uncheckedCast:[[adapter getTestIntf] ice_connectionCached:NO]];
        test(![test1 ice_isConnectionCached]);
        test(![test2 ice_isConnectionCached]);
        test([test1 ice_getConnection] && [test2 ice_getConnection]);
        test([[test1 ice_getConnection] isEqual:[test2 ice_getConnection]]);

        [test1 ice_ping];

        [com deactivateObjectAdapter:adapter];

        id<TestBindingTestIntfPrx> test3 = [TestBindingTestIntfPrx uncheckedCast:test1];
        @try
        {
            test([[test3 ice_getConnection] isEqual:[test1 ice_getConnection]]);
            test(NO);
        }
        @catch(ICEConnectionRefusedException*)
        {
        }
    }
    tprintf("ok\n");

    tprintf("testing per request binding with multiple endpoints... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"Adapter51" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter52" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter53" endpoints:@"default"]];

        id<TestBindingTestIntfPrx> test = [createTestIntfPrx(adapters) ice_connectionCached:NO];
        test(![test ice_isConnectionCached]);

        NSMutableSet* names = [NSMutableSet setWithCapacity:3];
        [names addObject:@"Adapter51"];
        [names addObject:@"Adapter52"];
        [names addObject:@"Adapter53"];
        while([names count] != 0)
        {
            [names removeObject:[test getAdapterName]];
        }

        [com deactivateObjectAdapter:[adapters objectAtIndex:0]];

        [names addObject:@"Adapter52"];
        [names addObject:@"Adapter53"];
        while([names count] != 0)
        {
            [names removeObject:[test getAdapterName]];
        }

        [com deactivateObjectAdapter:[adapters objectAtIndex:2]];


        test([[test getAdapterName] isEqualToString:@"Adapter52"]);

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing per request binding with multiple endpoints and AMI... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI51" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI52" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI53" endpoints:@"default"]];

        id<TestBindingTestIntfPrx> test = [createTestIntfPrx(adapters) ice_connectionCached:NO];
        test(![test ice_isConnectionCached]);

        NSMutableSet* names = [NSMutableSet setWithCapacity:3];
        [names addObject:@"AdapterAMI51"];
        [names addObject:@"AdapterAMI52"];
        [names addObject:@"AdapterAMI53"];
        while([names count] != 0)
        {
            [names removeObject:getAdapterNameWithAMI(test)];
        }

        [com deactivateObjectAdapter:[adapters objectAtIndex:0]];

        [names addObject:@"AdapterAMI52"];
        [names addObject:@"AdapterAMI53"];
        while([names count] != 0)
        {
            [names removeObject:getAdapterNameWithAMI(test)];
        }

        [com deactivateObjectAdapter:[adapters objectAtIndex:2]];

        test([[test getAdapterName] isEqualToString:@"AdapterAMI52"]);

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing per request binding and ordered endpoint selection... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"Adapter61" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter62" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter63" endpoints:@"default"]];

        id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
        test = [TestBindingTestIntfPrx uncheckedCast:[test ice_endpointSelection:ICEOrdered]];
        test([test ice_getEndpointSelection] == ICEOrdered);
        test = [TestBindingTestIntfPrx uncheckedCast:[test ice_connectionCached:NO]];
        test(![test ice_isConnectionCached]);
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter61"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [com deactivateObjectAdapter:[adapters objectAtIndex:0]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter62"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [com deactivateObjectAdapter:[adapters objectAtIndex:1]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter63"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [com deactivateObjectAdapter:[adapters objectAtIndex:2]];

        @try
        {
            [test getAdapterName];
        }
        @catch(ICEConnectionRefusedException*)
        {
        }

        NSArray* endpoints = getEndpoints(test);

        adapters = [NSMutableArray arrayWithCapacity:3];

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        [adapters addObject:[com createObjectAdapter:@"Adapter66" endpoints:[endpoints objectAtIndex:2]]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter66"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [adapters addObject:[com createObjectAdapter:@"Adapter65" endpoints:[endpoints objectAtIndex:1]]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter65"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [adapters addObject:[com createObjectAdapter:@"Adapter64" endpoints:[endpoints objectAtIndex:0]]];
        for(i = 0; i < nRetry && [[test getAdapterName] isEqualToString:@"Adapter64"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing per request binding and ordered endpoint selection and AMI... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI61" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI62" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI63" endpoints:@"default"]];

        id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
        test = [TestBindingTestIntfPrx uncheckedCast:[test ice_endpointSelection:ICEOrdered]];
        test([test ice_getEndpointSelection] == ICEOrdered);
        test = [TestBindingTestIntfPrx uncheckedCast:[test ice_connectionCached:NO]];
        test(![test ice_isConnectionCached]);
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        for(i = 0; i < nRetry && [getAdapterNameWithAMI(test) isEqualToString:@"AdapterAMI61"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [com deactivateObjectAdapter:[adapters objectAtIndex:0]];
        for(i = 0; i < nRetry && [getAdapterNameWithAMI(test) isEqualToString:@"AdapterAMI62"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [com deactivateObjectAdapter:[adapters objectAtIndex:1]];
        for(i = 0; i < nRetry && [getAdapterNameWithAMI(test) isEqualToString:@"AdapterAMI63"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [com deactivateObjectAdapter:[adapters objectAtIndex:2]];

        @try
        {
            [test getAdapterName];
        }
        @catch(ICEConnectionRefusedException*)
        {
        }

        NSArray* endpoints = getEndpoints(test);

        adapters = [NSMutableArray arrayWithCapacity:3];

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI66" endpoints:[endpoints objectAtIndex:2]]];
        for(i = 0; i < nRetry && [getAdapterNameWithAMI(test) isEqualToString:@"AdapterAMI66"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI65" endpoints:[endpoints objectAtIndex:1]]];
        for(i = 0; i < nRetry && [getAdapterNameWithAMI(test) isEqualToString:@"AdapterAMI65"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        [adapters addObject:[com createObjectAdapter:@"AdapterAMI64" endpoints:[endpoints objectAtIndex:0]]];
        for(i = 0; i < nRetry && [getAdapterNameWithAMI(test) isEqualToString:@"AdapterAMI64"]; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    tprintf("testing endpoint mode filtering... ");
    {
        NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
        [adapters addObject:[com createObjectAdapter:@"Adapter71" endpoints:@"default"]];
        [adapters addObject:[com createObjectAdapter:@"Adapter72" endpoints:@"udp"]];

        id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
        test([[test getAdapterName] isEqualToString:@"Adapter71"]);

        id<TestBindingTestIntfPrx> testUDP = [TestBindingTestIntfPrx uncheckedCast:[test ice_datagram]];
        test([test ice_getConnection] != [testUDP ice_getConnection]);
        @try
        {
            [testUDP getAdapterName];
        }
        @catch(ICETwowayOnlyException*)
        {
        }
    }
    tprintf("ok\n");

    if([[[communicator getProperties] getProperty:@"Ice.Default.Protocol"] isEqual:@"ssl"])
    {
        tprintf("testing unsecure vs. secure endpoints... ");
        {
            NSMutableArray* adapters = [NSMutableArray arrayWithCapacity:3];
            [adapters addObject:[com createObjectAdapter:@"Adapter81" endpoints:@"ssl"]];
            [adapters addObject:[com createObjectAdapter:@"Adapter82" endpoints:@"tcp"]];

            id<TestBindingTestIntfPrx> test = createTestIntfPrx(adapters);
            int i;
            for(i = 0; i < 5; i++)
            {
                test([[test getAdapterName] isEqualToString:@"Adapter82"]);
                [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            }

            id<TestBindingTestIntfPrx> testSecure = [TestBindingTestIntfPrx uncheckedCast:[test ice_secure:YES]];
            test([testSecure ice_isSecure]);
            testSecure = [TestBindingTestIntfPrx uncheckedCast:[test ice_secure:NO]];
            test(![testSecure ice_isSecure]);
            testSecure = [TestBindingTestIntfPrx uncheckedCast:[test ice_secure:YES]];
            test([testSecure ice_isSecure]);
            test([test ice_getConnection] != [testSecure ice_getConnection]);

            [com deactivateObjectAdapter:[adapters objectAtIndex:1]];

            for(i = 0; i < 5; i++)
            {
                test([[test getAdapterName] isEqualToString:@"Adapter81"]);
                [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            }

            [com createObjectAdapter:@"Adapter83" endpoints:[getEndpoints(test) objectAtIndex:1]]; // Reactive tcp OA.

            for(i = 0; i < 5; i++)
            {
                test([[test getAdapterName] isEqualToString:@"Adapter83"]);
                [[test ice_getConnection] close:ICEConnectionCloseGracefullyWithWait];
            }

            [com deactivateObjectAdapter:[adapters objectAtIndex:0]];
            @try
            {
                [testSecure ice_ping];
                test(NO);
            }
            @catch(ICEConnectionRefusedException*)
            {
            }

            deactivate(com, adapters);
        }
        tprintf("ok\n");
    }

    [com shutdown];
}

