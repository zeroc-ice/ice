// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <objects/TestI.h>

#if defined(__clang__) && __has_feature(objc_arc)
static ICEValueFactory factory = ^ICEObject* (NSString* type) NS_RETURNS_RETAINED
#else
static ICEValueFactory factory = ^ICEObject* (NSString* type)
#endif
{
    if([type isEqualToString:@"::Test::I"])
    {
        return [[TestObjectsI alloc] init];
    }
    else if([type isEqualToString:@"::Test::J"])
    {
        return [[TestObjectsJI alloc] init];
    }
    else if([type isEqualToString:@"::Test::H"])
    {
        return [[TestObjectsHI alloc] init];
    }
    else
    {
        test(NO); // Should never be reached
    }
    return nil;
};

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getValueFactoryManager] add:factory sliceId:@"::Test::I"];
    [[communicator getValueFactoryManager] add:factory sliceId:@"::Test::J"];
    [[communicator getValueFactoryManager] add:factory sliceId:@"::Test::H"];

    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    ICEObject* initial = [TestObjectsInitialI initial];
    [adapter add:initial identity:[ICEUtil stringToIdentity:@"initial"]];

    ICEObject* testObj = ICE_AUTORELEASE([[TestObjectsTestIntfI alloc] init]);
    [adapter add:testObj identity:[ICEUtil stringToIdentity:@"test"]];

    ICEObject* uoet = ICE_AUTORELEASE([[UnexpectedObjectExceptionTestI alloc] init]);
    [adapter add:uoet identity:[ICEUtil stringToIdentity:@"uoet"]];
    [adapter activate];

    serverReady(communicator);

    [communicator waitForShutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main objectsServer
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
            [initData.properties setProperty:@"Ice.Warn.Dispatch" value:@"0"];
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestObjects", @"::Test",
                                      nil];
#endif
            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator);
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
