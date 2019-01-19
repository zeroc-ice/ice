//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <exceptions/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"Ice.Warn.Dispatch" value:@"0"];
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010:udp"];
    [[communicator getProperties] setProperty:@"TestAdapter2.Endpoints" value:@"default -p 12011"];
    [[communicator getProperties] setProperty:@"TestAdapter2.MessageSizeMax" value:@"0"];
    [[communicator getProperties] setProperty:@"TestAdapter3.Endpoints" value:@"default -p 12012"];
    [[communicator getProperties] setProperty:@"TestAdapter3.MessageSizeMax" value:@"1"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    id<ICEObjectAdapter> adapter2 = [communicator createObjectAdapter:@"TestAdapter2"];
    id<ICEObjectAdapter> adapter3 = [communicator createObjectAdapter:@"TestAdapter3"];
    ICEObject* object = [ThrowerI thrower];
    [adapter add:object identity:[ICEUtil stringToIdentity:@"thrower"]];
    [adapter2 add:object identity:[ICEUtil stringToIdentity:@"thrower"]];
    [adapter3 add:object identity:[ICEUtil stringToIdentity:@"thrower"]];
    [adapter activate];
    [adapter2 activate];
    [adapter3 activate];

    serverReady(communicator);

    [communicator waitForShutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main exceptionsServer
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
    ICEregisterIceUDP(YES);
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
            [initData.properties setProperty:@"Ice.MessageSizeMax" value:@"10"]; // 10KB max
            [initData.properties setProperty:@"Ice.Warn.Dispatch" value:@"0"];
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];

#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestExceptions", @"::Test",
                                      @"TestExceptionsMod", @"::Test::Mod",
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
