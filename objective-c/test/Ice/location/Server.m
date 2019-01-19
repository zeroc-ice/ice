//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <location/ServerLocator.h>
#import <location/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator, ICEInitializationData* initData)
{
    //
    // Register the server manager. The server manager creates a new
    // 'server' (a server isn't a different process, it's just a new
    // communicator and object adapter).
    //
    [[communicator getProperties] setProperty:@"Ice.ThreadPool.Server.Size" value:@"2"];
    [[communicator getProperties] setProperty:@"ServerManager.Endpoints" value:@"default -p 12010:udp"];

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"ServerManager"];

    //
    // We also register a sample server locator which implements the
    // locator interface, this locator is used by the clients and the
    // 'servers' created with the server manager interface.
    //
    ServerLocatorRegistry* registry = ICE_AUTORELEASE([[ServerLocatorRegistry alloc] init]);
    ServerManagerI* serverManager = ICE_AUTORELEASE([[ServerManagerI alloc] init:registry initData:initData]);

    [registry addObject:[adapter createProxy:[ICEUtil stringToIdentity:@"ServerManager"]]];
    [adapter add:serverManager identity:[ICEUtil stringToIdentity:@"ServerManager"]];

    id<ICELocatorRegistryPrx> registryPrx =
        [ICELocatorRegistryPrx uncheckedCast:[adapter add:registry
                                                      identity:[ICEUtil stringToIdentity:@"registry"]]];

    ServerLocator* locator = ICE_AUTORELEASE([[ServerLocator alloc] init:registry proxy:registryPrx]);
    [adapter add:locator identity:[ICEUtil stringToIdentity:@"locator"]];

    [adapter activate];

    serverReady(communicator);

    [communicator waitForShutdown];
    [serverManager terminate];

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main locationServer
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
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestLocation", @"::Test",
                                      nil];
#endif
            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator, initData);
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
