// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <servantLocator/TestI.h>
#import <servantLocator/ServantLocatorI.h>
#import <TestCommon.h>

@interface SLServantLocatorI : TestServantLocatorI
@end

@implementation SLServantLocatorI
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
-(void) throwTestIntfUserException;
{
    @throw [TestServantLocatorTestIntfUserException testIntfUserException];
}
@end

@interface SLTestActivationI : TestServantLocatorTestActivation<TestServantLocatorTestActivation>
@end

@implementation SLTestActivationI
-(void) activateServantLocator:(BOOL)activate current:(ICECurrent *)current
{
    if(activate)
    {
        [current.adapter addServantLocator:ICE_AUTORELEASE([[SLServantLocatorI alloc] init:@""]) category:@""];
        [current.adapter addServantLocator:ICE_AUTORELEASE([[SLServantLocatorI alloc] init:@"category"])
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
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    [[communicator getProperties] setProperty:@"Ice.Warn.Dispatch" value:@"0"];

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];

    [adapter addServantLocator:ICE_AUTORELEASE([[SLServantLocatorI alloc] init:@""]) category:@""];
    [adapter addServantLocator:ICE_AUTORELEASE([[SLServantLocatorI alloc] init:@"category"]) category:@"category"];
    [adapter add:[TestServantLocatorTestIntfI testIntf] identity:[ICEUtil stringToIdentity:@"asm"]];
    [adapter add:[SLTestActivationI testActivation] identity:[ICEUtil stringToIdentity:@"test/activation"]];

    TestServantLocatorTestIntfPrx* servantLocatorAllTests(id<ICECommunicator>);
    servantLocatorAllTests(communicator);

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main servantLocatorCollocated
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
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
        return status;
    }
}
