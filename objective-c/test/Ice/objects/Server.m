// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <objects/TestI.h>

// Note that the factory must not autorelease the
// returned objects.
static ICEValueFactory factory = ^ICEObject* (NSString* type)
{
    if([type isEqualToString:@"::Test::I"])
    {
        return ICE_AUTORELEASE([[TestObjectsI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::J"])
    {
        return ICE_AUTORELEASE([[TestObjectsJI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::H"])
    {
        return ICE_AUTORELEASE([[TestObjectsHI alloc] init]);
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
#if TARGET_OS_IPHONE
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
#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
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
            @try
            {
                [communicator destroy];
            }
            @catch(ICEException* ex)
            {
                tprintf("%@\n", ex);
                status = EXIT_FAILURE;
            }
        }
    }
    return status;
}
