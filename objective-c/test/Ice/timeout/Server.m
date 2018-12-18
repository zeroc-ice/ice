// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <timeout/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.Endpoints" value:@"default -p 12011"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.ThreadPool.Size" value:@"1"];

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    [adapter add:[TimeoutI timeout] identity:[ICEUtil stringToIdentity:@"timeout"]];
    [adapter activate];

    id<ICEObjectAdapter> controllerAdapter = [communicator createObjectAdapter:@"ControllerAdapter"];
    [controllerAdapter add:[TimeoutControllerI controller:adapter] identity:[ICEUtil stringToIdentity:@"controller"]];
    [controllerAdapter activate];

    serverReady(communicator);

    [communicator waitForShutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main timeoutServer
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

            //
            // COMPILERFIX: Disable connect timeout introduced for
            // workaround to iOS device hangs when using SSL
            //
            [initData.properties setProperty:@"Ice.Override.ConnectTimeout" value:@""];

            //
            // This test kills connections, so we don't want warnings.
            //
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];

            //
            // The client sends large messages to cause the transport
            // buffers to fill up.
            //
            [initData.properties setProperty:@"Ice.MessageSizeMax" value:@"20000"];

            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            [initData.properties setProperty:@"Ice.TCP.RcvSize" value:@"50000"];

#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestTimeout", @"::Test",
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
