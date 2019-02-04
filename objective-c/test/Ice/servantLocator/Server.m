//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <servantLocator/TestI.h>
#import <servantLocator/ServantLocatorI.h>
#import <TestCommon.h>

@interface ServantLocatorI : TestServantLocatorI
@end

@implementation ServantLocatorI
-(ICEObject*) newServantAndCookie:(id*)cookie
{
    *cookie = ICE_AUTORELEASE([[TestServantLocatorCookieI alloc] init]);
    return [TestServantLocatorTestIntfI testIntf];
}
-(void) checkCookie:(id)cookie
{
    test(cookie != nil);
    id<TestServantLocatorCookie> co = cookie;
    test([[co message] isEqual:@"blahblah"]);

}
-(void) throwTestIntfUserException
{
    @throw [TestServantLocatorTestIntfUserException testIntfUserException];
}
@end

@interface TestActivationI : TestServantLocatorTestActivation<TestServantLocatorTestActivation>
@end

@implementation TestActivationI
-(void) activateServantLocator:(BOOL)activate current:(ICECurrent *)current
{
    if(activate)
    {
        [current.adapter addServantLocator:ICE_AUTORELEASE([[ServantLocatorI alloc] init:@""]) category:@""];
        [current.adapter addServantLocator:ICE_AUTORELEASE([[ServantLocatorI alloc] init:@"category"])
                                  category:@"category"];
    }
    else
    {
        id<ICEServantLocator> locator = [current.adapter removeServantLocator:@""];
        [locator deactivate:@""];
        locator = [current.adapter removeServantLocator:@"category"];
        [locator deactivate:@"category"];
    }
}
@end

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010:udp"];
    [[communicator getProperties] setProperty:@"Ice.Warn.Dispatch" value:@"0"];

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];

    [adapter addServantLocator:ICE_AUTORELEASE([[ServantLocatorI alloc] init:@""]) category:@""];
    [adapter addServantLocator:ICE_AUTORELEASE([[ServantLocatorI alloc] init:@"category"]) category:@"category"];
    [adapter add:[TestServantLocatorTestIntfI testIntf] identity:[ICEUtil stringToIdentity:@"asm"]];
    [adapter add:[TestActivationI testActivation] identity:[ICEUtil stringToIdentity:@"test/activation"]];
    [adapter activate];

    serverReady(communicator);

    [adapter waitForDeactivate];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main servantLocatorServer
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

    @autoreleasepool
    {
        int status;
        id<ICECommunicator> communicator = nil;

        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestServantLocator", @"::Test",
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
        return status;
    }
}
