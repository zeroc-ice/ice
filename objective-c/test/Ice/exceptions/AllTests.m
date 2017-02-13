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
#import <ExceptionsTest.h>

@interface ExceptionsEmptyI : TestExceptionsEmpty<TestExceptionsEmpty>
@end

@implementation ExceptionsEmptyI
@end

id<TestExceptionsThrowerPrx>
exceptionsAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing object adapter registration exceptions... ");
    {
        id<ICEObjectAdapter> first;
	@try
	{
	    first = [communicator createObjectAdapter:@"TestAdapter0"];
	    test(false);
	}
	@catch(ICEInitializationException *ex)
	{
	    // Expeccted
	}

	[[communicator getProperties] setProperty:@"TestAdapter0.Endpoints" value:@"default"];
	first = [communicator createObjectAdapter:@"TestAdapter0"];
	@try
	{
	    [communicator createObjectAdapter:@"TestAdapter0"];
	    test(false);
	}
	@catch(ICEAlreadyRegisteredException *ex)
	{
	    // Expected
	}

	@try
	{
	}
	@catch(ICEAlreadyRegisteredException *ex)
	{
	    // Expected
	}

	//
	// Properties must remain unaffected if an exception occurs.
	//
	test([[[communicator getProperties] getProperty:@"TestAdapter0.Endpoints"] isEqualToString:@"default"]);
	[first deactivate];
    }
    tprintf("ok\n");

    tprintf("testing servant registration exceptions... ");
    {
        [[communicator getProperties] setProperty:@"TestAdapter1.Endpoints" value:@"default"];
	id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter1"];
	ICEObject* obj = [ExceptionsEmptyI empty];
	[adapter add:obj identity:[communicator stringToIdentity:@"x"]];
	@try
	{
	    [adapter add:obj identity:[communicator stringToIdentity:@"x"]];
	    test(false);
	}
	@catch(ICEAlreadyRegisteredException *ex)
	{
	}

 	[adapter remove:[communicator stringToIdentity:@"x"]];
 	@try
 	{
 	    [adapter remove:[communicator stringToIdentity:@"x"]];
 	    test(false);
 	}
 	@catch(ICENotRegisteredException *ex)
 	{
 	}

	[adapter deactivate];
    }
    tprintf("ok\n");

    tprintf("testing stringToProxy... ");
    NSString *ref = @"thrower:default -p 12010";
    id<ICEObjectPrx> base = [communicator stringToProxy:ref];
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestExceptionsThrowerPrx> thrower = [TestExceptionsThrowerPrx checkedCast:base];
    test(thrower);
    test([thrower isEqual:base]);
    tprintf("ok\n");

    tprintf("catching exact types... ");

    @try
    {
        [thrower throwAasA:1];
	test(false);
    }
    @catch(TestExceptionsA *ex)
    {
        test(ex.aMem == 1);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwAorDasAorD:1];
        test(false);
    }
    @catch(TestExceptionsA *ex)
    {
        test(ex.aMem == 1);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwAorDasAorD:-1];
        test(false);
    }
    @catch(TestExceptionsD *ex)
    {
        test(ex.dMem == -1);
    }
    @catch(NSException* ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwBasB:1 b:2];
        test(false);
    }
    @catch(TestExceptionsB *ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwCasC:1 b:2 c:3];
        test(false);
    }
    @catch(TestExceptionsC *ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
        test(ex.cMem == 3);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwModA:1 a2:2];
        test(false);
    }
    @catch(TestExceptionsModA *ex)
    {
        test(ex.aMem == 1);
        test(ex.a2Mem == 2);
    }
    @catch(ICEOperationNotExistException *ex)
    {
        //
        // This operation is not supported in Java.
        //
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    tprintf("ok\n");

    tprintf("catching base types... ");

    @try
    {
        [thrower throwBasB:1 b:2];
        test(false);
    }
    @catch(TestExceptionsA *ex)
    {
        test(ex.aMem == 1);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwCasC:1 b:2 c:3];
        test(false);
    }
    @catch(TestExceptionsB *ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwModA:1 a2:2];
        test(false);
    }
    @catch(TestExceptionsA *ex)
    {
        test(ex.aMem == 1);
    }
    @catch(ICEOperationNotExistException *ex)
    {
        //
        // This operation is not supported in Java.
        //
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    tprintf("ok\n");

    tprintf("catching derived types... ");

    @try
    {
        [thrower throwBasA:1 b:2];
        test(false);
    }
    @catch(TestExceptionsB *ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwCasA:1 b:2 c:3];
        test(false);
    }
    @catch(TestExceptionsC *ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
        test(ex.cMem == 3);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    @try
    {
        [thrower throwCasB:1 b:2 c:3];
        test(false);
    }
    @catch(TestExceptionsC *ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
        test(ex.cMem == 3);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    tprintf("ok\n");

    if([thrower supportsUndeclaredExceptions])
    {
        tprintf("catching unknown user exception... ");

        @try
        {
            [thrower throwUndeclaredA:1];
            test(false);
        }
        @catch(ICEUnknownUserException *ex)
        {
        }
        @catch(NSException *ex)
        {
            test(false);
        }

        @try
        {
            [thrower throwUndeclaredB:1 b:2];
            test(false);
        }
        @catch(ICEUnknownUserException *ex)
        {
        }
        @catch(NSException *ex)
        {
            test(false);
        }

        @try
        {
            [thrower throwUndeclaredC:1 b:2 c:3];
            test(false);
        }
        @catch(ICEUnknownUserException *ex)
        {
        }
        @catch(NSException *ex)
        {
            test(false);
        }

        tprintf("ok\n");
    }

    if([thrower ice_getConnection])
    {
        tprintf("testing memory limit marshal exception...");
        @try
        {
            ICEByteSeq *bs = [NSMutableData dataWithLength:0];
            [thrower throwMemoryLimitException:bs];
            test(false);
        }
        @catch(ICEMemoryLimitException *ex)
        {
        }
        @catch(NSException *ex)
        {
            test(false);
        }

        @try
        {
            [thrower throwMemoryLimitException:[NSMutableData dataWithLength:20 * 1024]]; // 20KB
            test(false);
        }
        @catch(ICEConnectionLostException *ex)
        {
        }
        @catch(NSException *ex)
        {
            test(false);
        }

        id<TestExceptionsThrowerPrx> thrower2 =
            [TestExceptionsThrowerPrx checkedCast:[communicator stringToProxy:@"thrower:default -p 12011"]];
        @try
        {
            [thrower2 throwMemoryLimitException:[NSMutableData dataWithLength:20 * 1024 * 1024]]; // 2MB (no limits)
        }
        @catch(ICEMemoryLimitException *ex)
        {
        }
        id<TestExceptionsThrowerPrx> thrower3 =
            [TestExceptionsThrowerPrx checkedCast:[communicator stringToProxy:@"thrower:default -p 12012"]];
        @try
        {
            [thrower3 throwMemoryLimitException:[NSMutableData dataWithLength:1024]]; // 1KB limit
            test(NO);
        }
        @catch(ICEConnectionLostException *ex)
        {
        }

        tprintf("ok\n");
    }

    tprintf("catching object not exist exception... ");

    ICEIdentity *id_ = [communicator stringToIdentity:@"does not exist"];
    @try
    {
	id<TestExceptionsThrowerPrx> thrower2 = [TestExceptionsThrowerPrx uncheckedCast:[thrower ice_identity:id_]];
        [thrower2 throwAasA:1];
//      //[thrower2 ice_ping];
        test(false);
    }
    @catch(ICEObjectNotExistException *ex)
    {
        test([ex.id_ isEqual:id_]);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    tprintf("ok\n");

    tprintf("catching facet not exist exception... ");

    @try
    {
        id<TestExceptionsThrowerPrx> thrower2 = [TestExceptionsThrowerPrx uncheckedCast:thrower facet:@"no such facet"];
        @try
        {
            [thrower2 ice_ping];
            test(false);
        }
        @catch(ICEFacetNotExistException *ex)
        {
            test([ex.facet isEqualToString:@"no such facet"]);
        }
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    tprintf("ok\n");

    tprintf("catching operation not exist exception... ");

    @try
    {
        id<TestExceptionsWrongOperationPrx> thrower2 = [TestExceptionsWrongOperationPrx uncheckedCast:thrower];
        [thrower2 noSuchOperation];
        test(false);
    }
    @catch(ICEOperationNotExistException *ex)
    {
        test([ex.operation isEqualToString:@"noSuchOperation"]);
    }
    @catch(NSException *ex)
    {
        test(false);
    }

    tprintf("ok\n");

    tprintf("catching unknown local exception... ");

    @try
    {
        [thrower throwLocalException];
        test(false);
    }
    @catch(ICEUnknownLocalException *ex)
    {
    }
    @catch(NSException *ex)
    {
        test(false);
    }
    @try
    {
        [thrower throwLocalExceptionIdempotent];
        test(false);
    }
    @catch(ICEUnknownLocalException*)
    {
    }
    @catch(ICEOperationNotExistException*)
    {
    }
    @catch(NSException* ex)
    {
        test(false);
    }


    tprintf("ok\n");

    tprintf("catching unknown non-Ice exception... ");

    @try
    {
        [thrower throwNonIceException];
        test(false);
    }
    @catch(ICEUnknownException *)
    {
    }
    @catch(NSException *)
    {
        test(false);
    }

    tprintf("ok\n");

    return thrower;
}
