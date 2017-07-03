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
#import <admin/TestI.h>

#import <Foundation/Foundation.h>

static void
testFacets(id<ICECommunicator> com, BOOL builtInFacets)
{
    if(builtInFacets)
    {
        test([com findAdminFacet:@"Properties"]);
        test([com findAdminFacet:@"Process"]);
        test([com findAdminFacet:@"Logger"]);
        test([com findAdminFacet:@"Metrics"]);
    }

    TestAdminTestFacet* f1 = [TestAdminTestFacetI testFacet];
    TestAdminTestFacet* f2 = [TestAdminTestFacetI testFacet];
    TestAdminTestFacet* f3 = [TestAdminTestFacetI testFacet];

    [com addAdminFacet:f1 facet:@"Facet1"];
    [com addAdminFacet:f2 facet:@"Facet2"];
    [com addAdminFacet:f3 facet:@"Facet3"];

    test([com findAdminFacet:@"Facet1"] == f1);
    test([com findAdminFacet:@"Facet2"] == f2);
    test([com findAdminFacet:@"Facet3"] == f3);
    test(![com findAdminFacet:@"Bogus"]);

    ICEFacetMap* facetMap = [com findAllAdminFacets];

    if(builtInFacets)
    {
        test([facetMap count] == 7);
        test([facetMap objectForKey:@"Properties"] != nil);
        test([facetMap objectForKey:@"Process"] != nil);
        test([facetMap objectForKey:@"Logger"] != nil);
        test([facetMap objectForKey:@"Metrics"] != nil);
    }
    else
    {
        test([facetMap count] >= 3);
    }
    test([facetMap objectForKey:@"Facet1"] != nil);
    test([facetMap objectForKey:@"Facet2"] != nil);
    test([facetMap objectForKey:@"Facet3"] != nil);

    @try
    {
        [com addAdminFacet:f1 facet:@"Facet1"];
        test(NO);
    }
    @catch(ICEAlreadyRegisteredException*)
    {
        // Expected
    }

    @try
    {
        [com removeAdminFacet:@"Bogus"];
        test(NO);
    }
    @catch(ICENotRegisteredException*)
    {
        // Expected
    }

    [com removeAdminFacet:@"Facet1"];
    [com removeAdminFacet:@"Facet2"];
    [com removeAdminFacet:@"Facet3"];

    @try
    {
        [com removeAdminFacet:@"Facet1"];
        test(NO);
    }
    @catch(ICENotRegisteredException*)
    {
        // Expected
    }
}

@interface RemoteLoggerI : ICERemoteLogger<ICERemoteLogger>
{
    NSCondition* _cond;
    int _receivedCalls;
    NSString* _expectedPrefix;
    ICEMutableLogMessageSeq* _expectedInitMessages;
    ICEMutableLogMessageSeq* _expectedLogMessages;
}
-(void) checkNextInit:(NSString*)prefix logMessages:(ICELogMessageSeq*)logMessages;
-(void) checkNextLog:(ICELogMessageType)messageType message:(NSString*)message category:(NSString*)category;
-(void) wait:(int)duration;
@end

@implementation RemoteLoggerI

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->_receivedCalls = 0;
    self->_cond = [[NSCondition alloc] init];
    self->_expectedLogMessages = [[ICEMutableLogMessageSeq alloc] init];
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_expectedPrefix release];
    [_expectedInitMessages release];
    [_expectedLogMessages release];
    [_cond release];
    [super dealloc];
}
#endif
-(void) init:(NSString*)prefix logMessages:(ICEMutableLogMessageSeq*)logMessages current:(ICECurrent*)current
{
    [_cond lock];
    @try
    {
        test([prefix isEqual:_expectedPrefix]);
        test([logMessages isEqual:_expectedInitMessages]);
        _receivedCalls++;
        [_cond signal];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) log:(ICELogMessage*)logMessage current:(ICECurrent*)current
{
    [_cond lock];
    @try
    {
        ICELogMessage* front = [_expectedLogMessages objectAtIndex:0];

        test(front.type == logMessage.type &&
             [front.message isEqual:logMessage.message] &&
             [front.traceCategory isEqual:logMessage.traceCategory]);

        [_expectedLogMessages removeObjectAtIndex:0];
        _receivedCalls++;
        [_cond signal];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) checkNextInit:(NSString*)prefix logMessages:(ICEMutableLogMessageSeq*)logMessages
{
    [_cond lock];
    @try
    {
        ICE_RELEASE(_expectedPrefix);
        _expectedPrefix = ICE_RETAIN(prefix);
        ICE_RELEASE(_expectedInitMessages);
        _expectedInitMessages = ICE_RETAIN(logMessages);
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) checkNextLog:(ICELogMessageType)type message:(NSString*)message category:(NSString*)category
{
    [_cond lock];
    @try
    {
        ICELogMessage* msg = [ICELogMessage logMessage:type timestamp:0 traceCategory:category message:message];
        [_expectedLogMessages addObject:msg];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) wait:(int)calls
{
    [_cond lock];
    @try
    {
        _receivedCalls -= calls;
        while(_receivedCalls < 0)
        {
            [_cond wait];
        }
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

void
adminAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing communicator operations... ");
    {
        //
        // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
        //
        ICEInitializationData* init = [ICEInitializationData initializationData];
        init.properties = [ICEUtil createProperties];
        [init.properties setProperty:@"Ice.Admin.Endpoints" value:@"tcp -h 127.0.0.1"];
        [init.properties setProperty:@"Ice.Admin.InstanceName" value:@"Test"];
        id<ICECommunicator> com = [ICEUtil createCommunicator:init];
        testFacets(com, YES);
        [com destroy];
    }
    {
        //
        // Test: Verify that the operations work correctly in the presence of facet filters.
        //
        ICEInitializationData* init = [ICEInitializationData initializationData];
        init.properties = [ICEUtil createProperties];
        [init.properties setProperty:@"Ice.Admin.Endpoints" value:@"tcp -h 127.0.0.1"];
        [init.properties setProperty:@"Ice.Admin.InstanceName" value:@"Test"];
        [init.properties setProperty:@"Ice.Admin.Facets" value:@"Properties"];
        id<ICECommunicator> com = [ICEUtil createCommunicator:init];
        testFacets(com, NO);
        [com destroy];
    }
    {
        //
        // Test: Verify that the operations work correctly with the Admin object disabled.
        //
        id<ICECommunicator> com = [ICEUtil createCommunicator];
        testFacets(com, NO);
        [com destroy];
    }
    {
        //
        // Test: Verify that the operations work correctly when Ice.Admin.Enabled is set
        //
        ICEInitializationData* init = [ICEInitializationData initializationData];
        init.properties = [ICEUtil createProperties];
        [init.properties setProperty:@"Ice.Admin.Enabled" value:@"1"];
        id<ICECommunicator> com = [ICEUtil createCommunicator:init];
        test([com getAdmin] == nil);

        ICEIdentity* ident = [ICEUtil stringToIdentity:@"test-admin"];
        @try
        {
            [com createAdmin:nil adminId:ident];
            test(NO);
        }
        @catch(ICEInitializationException*)
        {
        }

        id<ICEObjectAdapter> adapter = [com createObjectAdapter:@""];
        test([com createAdmin:adapter adminId:ident] != nil);
        test([com getAdmin] != nil);

        testFacets(com, YES);
        [com destroy];
    }
    {
        //
        // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
        //
        ICEInitializationData* init = [ICEInitializationData initializationData];
        init.properties = [ICEUtil createProperties];
        [init.properties setProperty:@"Ice.Admin.Endpoints" value:@"tcp -h 127.0.0.1"];
        [init.properties setProperty:@"Ice.Admin.InstanceName" value:@"Test"];
        [init.properties setProperty:@"Ice.Admin.DelayCreation" value:@"1"];
        id<ICECommunicator> com = [ICEUtil createCommunicator:init];
        testFacets(com, YES);
        [com getAdmin];
        testFacets(com, YES);
        [com destroy];
    }
    tprintf("ok\n");

    NSString* ref = @"factory:default -p 12010 -t 10000";
    id<TestAdminRemoteCommunicatorFactoryPrx> factory =
        [TestAdminRemoteCommunicatorFactoryPrx uncheckedCast:[communicator stringToProxy:ref]];

    tprintf("testing process facet... ");
    {
        //
        // Test: Verify that Process::shutdown() operation shuts down the communicator.
        //
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICEProcessPrx> proc = [ICEProcessPrx checkedCast:obj facet:@"Process"];
        [proc shutdown];
        [com waitForShutdown];
        [com destroy];
    }
    tprintf("ok\n");

    tprintf("testing properties facet... ");
    {
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        [props setObject:@"1" forKey:@"Prop1"];
        [props setObject:@"2" forKey:@"Prop2"];
        [props setObject:@"3" forKey:@"Prop3"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICEPropertiesAdminPrx> pa = [ICEPropertiesAdminPrx checkedCast:obj facet:@"Properties"];

        //
        // Test: PropertiesAdmin::getProperty()
        //
        test([[pa getProperty:@"Prop2"] isEqual:@"2"]);
        test([[pa getProperty:@"Bogus"] isEqual:@""]);

        //
        // Test: PropertiesAdmin::getProperties()
        //
        ICEPropertyDict* pd = [pa getPropertiesForPrefix:@""];
        test([pd count] == 5);
        test([[pd objectForKey:@"Ice.Admin.Endpoints"] isEqual:@"tcp -h 127.0.0.1"]);
        test([[pd objectForKey:@"Ice.Admin.InstanceName"] isEqual:@"Test"]);
        test([[pd objectForKey:@"Prop1"] isEqual:@"1"]);
        test([[pd objectForKey:@"Prop2"] isEqual:@"2"]);
        test([[pd objectForKey:@"Prop3"] isEqual:@"3"]);

        ICEPropertyDict* changes;

        //
        // Test: PropertiesAdmin::setProperties()
        //
        ICEMutablePropertyDict* setProps = [ICEMutablePropertyDict dictionary];
        [setProps setObject:@"10" forKey:@"Prop1"]; // Changed
        [setProps setObject:@"20" forKey:@"Prop2"]; // Changed
        [setProps setObject:@"" forKey:@"Prop3"]; // Removed
        [setProps setObject:@"4" forKey:@"Prop4"]; // Added
        [setProps setObject:@"5" forKey:@"Prop5"]; // Added
        [pa setProperties:setProps];
        test([[pa getProperty:@"Prop1"] isEqual:@"10"]);
        test([[pa getProperty:@"Prop2"] isEqual:@"20"]);
        test([[pa getProperty:@"Prop3"] isEqual:@""]);
        test([[pa getProperty:@"Prop4"] isEqual:@"4"]);
        test([[pa getProperty:@"Prop5"] isEqual:@"5"]);
        changes = [com getChanges];
        test([changes count] == 5);
        test([[changes objectForKey:@"Prop1"] isEqual:@"10"]);
        test([[changes objectForKey:@"Prop2"] isEqual:@"20"]);
        test([[changes objectForKey:@"Prop3"] isEqual:@""]);
        test([[changes objectForKey:@"Prop4"] isEqual:@"4"]);
        test([[changes objectForKey:@"Prop5"] isEqual:@"5"]);
        [pa setProperties:setProps];
        changes = [com getChanges];
        test([changes count] == 0);

        [com destroy];
    }
    tprintf("ok\n");

    tprintf("testing logger facet... ");
    {
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        [props setObject:@"1" forKey:@"NullLogger"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];

        [com trace:@"testCat" message:@"trace"];
        [com warning:@"warning"];
        [com error:@"error"];
        [com print:@"print"];

        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICELoggerAdminPrx> logger = [ICELoggerAdminPrx checkedCast:obj facet:@"Logger"];
        test(logger);

        NSMutableString* prefix = nil;

        //
        // Get all
        //
        ICELogMessageSeq* logMessages = [logger getLog:nil traceCategories:nil messageMax:-1 prefix:&prefix];

        test([logMessages count] == 4);
        test([prefix isEqual:@"NullLogger"]);
        int i = 0;
        test([((ICELogMessage*)[logMessages objectAtIndex:i]).traceCategory isEqual:@"testCat"] &&
             [((ICELogMessage*)[logMessages objectAtIndex:i++]).message isEqual:@"trace"]);
        test([((ICELogMessage*)[logMessages objectAtIndex:i++]).message isEqual:@"warning"]);
        test([((ICELogMessage*)[logMessages objectAtIndex:i++]).message isEqual:@"error"]);
        test([((ICELogMessage*)[logMessages objectAtIndex:i++]).message isEqual:@"print"]);

        //
        // Get only errors and warnings
        //
        [com error:@"error2"];
        [com print:@"print2"];
        [com trace:@"testCat" message:@"trace2"];
        [com warning:@"warning2"];

        ICELogMessageTypeSeq* messageTypes;

        ICELogMessageType messageTypesArray1[] = { ICEErrorMessage, ICEWarningMessage };
        messageTypes = [ICEMutableLogMessageTypeSeq dataWithBytes:messageTypesArray1 length:sizeof(messageTypesArray1)];

        logMessages = [logger getLog:messageTypes traceCategories:nil messageMax:-1 prefix:&prefix];
        test([logMessages count] == 4);
        test([prefix isEqual:@"NullLogger"]);

        for(ICELogMessage* message in logMessages)
        {
            test(message.type == ICEErrorMessage || message.type == ICEWarningMessage);
        }

        //
        // Get only errors and traces with Cat = "testCat"
        //
        [com trace:@"testCat2" message:@"A"];
        [com trace:@"testCat" message:@"trace3"];
        [com trace:@"testCat2" message:@"B"];

        ICELogMessageType messageTypesArray2[] = { ICEErrorMessage, ICETraceMessage };
        messageTypes = [ICEMutableLogMessageTypeSeq dataWithBytes:messageTypesArray2 length:sizeof(messageTypesArray2)];

        ICEMutableStringSeq* categories = [ICEMutableStringSeq array];
        [categories addObject:@"testCat"];

        logMessages = [logger getLog:messageTypes traceCategories:categories messageMax:-1 prefix:&prefix];
        test([logMessages count] == 5);
        test([prefix isEqual:@"NullLogger"]);

        for(ICELogMessage* message in logMessages)
        {
            test(message.type == ICEErrorMessage ||
                 (message.type == ICETraceMessage && [message.traceCategory isEqual:@"testCat"]));
        }

        //
        // Same, but limited to last 2 messages (trace3 + error3)
        //
        [com error:@"error3"];

        logMessages = [logger getLog:messageTypes traceCategories:categories messageMax:2 prefix:&prefix];
        test([logMessages count] == 2);
        test([prefix isEqual:@"NullLogger"]);

        i = 0;
        test([((ICELogMessage*)[logMessages objectAtIndex:i++]).message isEqual:@"trace3"]);
        test([((ICELogMessage*)[logMessages objectAtIndex:i++]).message isEqual:@"error3"]);

        //
        // Now, test RemoteLogger
        //

        id<ICEObjectAdapter> adapter = [communicator createObjectAdapterWithEndpoints:@"RemoteLoggerAdapter"
                                                                            endpoints:@"tcp -h localhost"];

        RemoteLoggerI* remoteLogger = [RemoteLoggerI remoteLogger];
        id<ICERemoteLoggerPrx> myProxy = [ICERemoteLoggerPrx uncheckedCast:[adapter addWithUUID:remoteLogger]];

        [adapter activate];

        //
        // No filtering
        //
        logMessages = [logger getLog:nil traceCategories:nil messageMax:-1 prefix:&prefix];
        [remoteLogger checkNextInit:prefix logMessages:logMessages];

        [logger attachRemoteLogger:myProxy messageTypes:nil traceCategories:nil messageMax:-1];
        [remoteLogger wait:1];

        [remoteLogger checkNextLog:ICETraceMessage message:@"rtrace" category:@"testCat"];
        [remoteLogger checkNextLog:ICEWarningMessage message:@"rwarning" category:@""];
        [remoteLogger checkNextLog:ICEErrorMessage message:@"rerror" category:@""];
        [remoteLogger checkNextLog:ICEPrintMessage message:@"rprint" category:@""];

        [com trace:@"testCat" message:@"rtrace"];
        [com warning:@"rwarning"];
        [com error:@"rerror"];
        [com print:@"rprint"];
        [remoteLogger wait:4];

        test([logger detachRemoteLogger:myProxy]);
        test(![logger detachRemoteLogger:myProxy]);

        //
        // Use Error + Trace with "traceCat" filter with 4 limit
        //
        logMessages = [logger getLog:messageTypes traceCategories:categories messageMax:4 prefix:&prefix];
        test([logMessages count] == 4);
        [remoteLogger checkNextInit:prefix logMessages:logMessages];
        [logger attachRemoteLogger:myProxy messageTypes:messageTypes traceCategories:categories messageMax:4];
        [remoteLogger wait:1];

        [remoteLogger checkNextLog:ICETraceMessage message:@"rtrace2" category:@"testCat"];
        [remoteLogger checkNextLog:ICEErrorMessage message:@"rerror2" category:@""];

        [com warning:@"rwarning2"];
        [com trace:@"testCat" message:@"rtrace2"];
        [com warning:@"rwarning3"];
        [com error:@"rerror2"];
        [com print:@"rprint2"];
        [remoteLogger wait:2];

        //
        // Attempt reconnection with slightly different proxy
        //
        @try
        {
            [logger attachRemoteLogger:[myProxy ice_oneway]
                          messageTypes:messageTypes
                       traceCategories:categories
                            messageMax:4];
            test(NO);
        }
        @catch(ICERemoteLoggerAlreadyAttachedException*)
        {
            // expected
        }

        [com destroy];
    }
    tprintf("ok\n");

    tprintf("testing custom facet... ");
    {
        //
        // Test: Verify that the custom facet is present.
        //
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<TestAdminTestFacetPrx> tf = [TestAdminTestFacetPrx checkedCast:obj facet:@"TestFacet"];
        [tf op];
        [com destroy];
    }
    tprintf("ok\n");

    tprintf("testing facet filtering... ");
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Properties facet,
        // meaning no other facet is available.
        //
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        [props setObject:@"Properties" forKey:@"Ice.Admin.Facets"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICEProcessPrx> proc = [ICEProcessPrx checkedCast:obj facet:@"Process"];
        test(proc == nil);
        id<TestAdminTestFacetPrx> tf = [TestAdminTestFacetPrx checkedCast:obj facet:@"TestFacet"];
        test(tf == nil);
        [com destroy];
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Process facet,
        // meaning no other facet is available.
        //
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        [props setObject:@"Process" forKey:@"Ice.Admin.Facets"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICEPropertiesAdminPrx> pa = [ICEPropertiesAdminPrx checkedCast:obj facet:@"Properties"];
        test(pa == nil);
        id<TestAdminTestFacetPrx> tf = [TestAdminTestFacetPrx checkedCast:obj facet:@"TestFacet"];
        test(tf == nil);
        [com destroy];
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
        // meaning no other facet is available.
        //
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        [props setObject:@"TestFacet" forKey:@"Ice.Admin.Facets"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICEPropertiesAdminPrx> pa = [ICEPropertiesAdminPrx checkedCast:obj facet:@"Properties"];
        test(pa == nil);
        id<ICEProcessPrx> proc = [ICEProcessPrx checkedCast:obj facet:@"Process"];
        test(proc == nil);
        [com destroy];
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
        // facet names.
        //
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        [props setObject:@"Properties TestFacet" forKey:@"Ice.Admin.Facets"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICEPropertiesAdminPrx> pa = [ICEPropertiesAdminPrx checkedCast:obj facet:@"Properties"];
        test([[pa getProperty:@"Ice.Admin.InstanceName"] isEqual:@"Test"]);
        id<TestAdminTestFacetPrx> tf = [TestAdminTestFacetPrx checkedCast:obj facet:@"TestFacet"];
        [tf op];
        id<ICEProcessPrx> proc = [ICEProcessPrx checkedCast:obj facet:@"Process"];
        test(proc == nil);
        [com destroy];
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
        // facet names.
        //
        ICEMutablePropertyDict* props = [ICEMutablePropertyDict dictionary];
        [props setObject:@"tcp -h 127.0.0.1" forKey:@"Ice.Admin.Endpoints"];
        [props setObject:@"Test" forKey:@"Ice.Admin.InstanceName"];
        [props setObject:@"TestFacet, Process" forKey:@"Ice.Admin.Facets"];
        id<TestAdminRemoteCommunicatorPrx> com = [factory createCommunicator:props];
        id<ICEObjectPrx> obj = [com getAdmin];
        id<ICEPropertiesAdminPrx> pa = [ICEPropertiesAdminPrx checkedCast:obj facet:@"Properties"];
        test(pa == nil);
        id<TestAdminTestFacetPrx> tf = [TestAdminTestFacetPrx checkedCast:obj facet:@"TestFacet"];
        [tf op];
        id<ICEProcessPrx> proc = [ICEProcessPrx checkedCast:obj facet:@"Process"];
        [proc shutdown];
        [com waitForShutdown];
        [com destroy];
    }
    tprintf("ok\n");

    [factory shutdown];
}
