// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <ServantLocatorTest.h>

void
testExceptions(id<TestServantLocatorTestIntfPrx> obj)
{
    @try
    {
        [obj requestFailedException];
        test(NO);
    }
    @catch(ICEObjectNotExistException* ex)
    {
        test([ex.id_ isEqual:[obj ice_getIdentity]]);
        test([ex.facet isEqual:[obj ice_getFacet]]);
        test([ex.operation isEqual:@"requestFailedException"]);
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj unknownUserException];
        test(NO);
    }
    @catch(ICEUnknownUserException* ex)
    {
        test([ex.unknown isEqual:@"reason"]);
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj unknownLocalException];
        test(NO);
    }
    @catch(ICEUnknownLocalException* ex)
    {
        test([ex.unknown isEqual:@"reason"]);
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj unknownException];
        test(NO);
    }
    @catch(ICEUnknownException* ex)
    {
        test([ex.unknown isEqual:@"reason"]);
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj userException];
        test(NO);
    }
    @catch(ICEUnknownUserException* ex)
    {
        test([ex.unknown isEqual:@"::Test::TestIntfUserException"]);
    }
    @catch(ICEOperationNotExistException*)
    {
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj localException];
        test(NO);
    }
    @catch(ICEUnknownLocalException* ex)
    {
        test([ex.unknown rangeOfString:@"Ice::SocketException"].location != NSNotFound ||
             [ex.unknown rangeOfString:@"Ice.SocketException"].location != NSNotFound);
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj unknownExceptionWithServantException];
        test(NO);
    }
    @catch(ICEUnknownException* ex)
    {
        test([ex.unknown isEqual:@"reason"]);
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj impossibleException:NO];
        test(NO);
    }
    @catch(ICEUnknownUserException*)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj impossibleException:YES];
        test(NO);
    }
    @catch(ICEUnknownUserException*)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj intfUserException:NO];
        test(NO);
    }
    @catch(TestServantLocatorTestImpossibleException*)
    {
        // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        [obj intfUserException:YES];
        test(NO);
    }
    @catch(TestServantLocatorTestImpossibleException*)
    {
        // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
    }
    @catch(id)
    {
        test(NO);
    }
}

TestServantLocatorTestIntfPrx*
servantLocatorAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing stringToProxy... ");
    NSString* sref = @"asm:default -p 12010";
    id<ICEObjectPrx> base = [communicator stringToProxy:sref];
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestServantLocatorTestIntfPrx> obj = [TestServantLocatorTestIntfPrx uncheckedCast:base];
    test(obj);
    tprintf("ok\n");

    tprintf("testing ice_ids... ");
    @try
    {
        id<ICEObjectPrx> o = [communicator stringToProxy:@"category/locate:default -p 12010"];
        [o ice_ids];
        test(NO);
    }
    @catch(ICEUnknownUserException* ex)
    {
        test([ex.unknown isEqual:@"::Test::TestIntfUserException"]);
    }
    @catch(id)
    {
        test(NO);
    }

    @try
    {
        id<ICEObjectPrx> o = [communicator stringToProxy:@"category/finished:default -p 12010"];
        [o ice_ids];
        test(NO);
    }
    @catch(ICEUnknownUserException* ex)
    {
        test([ex.unknown isEqual:@"::Test::TestIntfUserException"]);
    }
    @catch(id)
    {
        test(NO);
    }
    tprintf("ok\n");

    tprintf("testing servant locator... ");
    base = [communicator stringToProxy:@"category/locate:default -p 12010"];
    obj = [TestServantLocatorTestIntfPrx checkedCast:base];
    @try
    {
        [TestServantLocatorTestIntfPrx checkedCast:[communicator stringToProxy:@"category/unknown:default -p 12010"]];
    }
    @catch(ICEObjectNotExistException*)
    {
    }
    tprintf("ok\n");

    tprintf("testing default servant locator... ");
    base = [communicator stringToProxy:@"anothercategory/locate:default -p 12010"];
    obj = [TestServantLocatorTestIntfPrx checkedCast:base];
    base = [communicator stringToProxy:@"locate:default -p 12010"];
    obj = [TestServantLocatorTestIntfPrx checkedCast:base];
    @try
    {
        [TestServantLocatorTestIntfPrx checkedCast:[communicator stringToProxy:@"anothercategory/unknown:default -p 12010"]];
    }
    @catch(ICEObjectNotExistException*)
    {
    }
    @try
    {
        [TestServantLocatorTestIntfPrx checkedCast:[communicator stringToProxy:@"unknown:default -p 12010"]];
    }
    @catch(ICEObjectNotExistException*)
    {
    }
    tprintf("ok\n");

    tprintf("testing locate exceptions... ");
    base = [communicator stringToProxy:@"category/locate:default -p 12010"];
    obj = [TestServantLocatorTestIntfPrx checkedCast:base];
    testExceptions(obj);
    tprintf("ok\n");

    tprintf("testing finished exceptions... ");
    base = [communicator stringToProxy:@"category/finished:default -p 12010"];
    obj = [TestServantLocatorTestIntfPrx checkedCast:base];
    testExceptions(obj);

    //
    // Only call these for category/finished.
    //
    @try
    {
        [obj asyncResponse];
    }
    @catch(TestServantLocatorTestIntfUserException*)
    {
        test(NO);
    }
    @catch(TestServantLocatorTestImpossibleException*)
    {
        //
        // Called by finished().
        //
    }

    @try
    {
        [obj asyncException];
    }
    @catch(TestServantLocatorTestIntfUserException*)
    {
        test(NO);
    }
    @catch(TestServantLocatorTestImpossibleException*)
    {
        //
        // Called by finished().
        //
    }
    tprintf("ok\n");

    tprintf("testing servant locator removal... ");
    base = [communicator stringToProxy:@"test/activation:default -p 12010"];
    id<TestServantLocatorTestActivationPrx> activation = [TestServantLocatorTestActivationPrx checkedCast:base];
    [activation activateServantLocator:NO];
    @try
    {
        [obj ice_ping];
        test(NO);
    }
    @catch(ICEObjectNotExistException*)
    {
        tprintf("ok\n");
    }

    tprintf("testing servant locator addition... ");
    [activation activateServantLocator:YES];
    @try
    {
        [obj ice_ping];
        tprintf("ok\n");
    }
    @catch(id)
    {
        test(NO);
    }

    return obj;
}
