// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <ami/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAMIAdapter.Endpoints" value:@"default -p 12010:udp"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.Endpoints" value:@"default -p 12011"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.ThreadPool.Size" value:@"1"];

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAMIAdapter"];
    id<ICEObjectAdapter> adapter2 = [communicator createObjectAdapter:@"ControllerAdapter"];

    TestAMITestIntfControllerI* testController
        = ICE_AUTORELEASE([[TestAMITestIntfControllerI alloc] initWithAdapter:adapter]);

    [adapter add:[TestAMITestIntfI testIntf] identity:[ICEUtil stringToIdentity:@"test"]];
    [adapter add:[TestAMITestOuterInnerTestIntfI testIntf] identity:[ICEUtil stringToIdentity:@"test2"]];
    //[adapter activate]; // Collocated test doesn't need to activate the OA

    [adapter2 add:testController identity:[ICEUtil stringToIdentity:@"testController"]];
    //[adapter2 activate]; // Collocated test doesn't need to activate the OA

    void amiAllTests(id<ICECommunicator>, BOOL);
    amiAllTests(communicator, true);

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main amiCollocated
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
            [initData.properties setProperty:@"Ice.Warn.AMICallback" value:@"0"];
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                  @"TestAMI", @"::Test",
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
