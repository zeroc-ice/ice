// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <SlicingExceptionsTestClient.h>

#import <Foundation/Foundation.h>

@interface RelayI : TestSlicingExceptionsClientRelay<TestSlicingExceptionsClientRelay>
@end

@implementation RelayI

-(void) knownPreservedAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsClientKnownPreservedDerived knownPreservedDerived:@"base"
                                                                                kp:@"preserved"
                                                                               kpd:@"derived"];
}

-(void) knownPreservedAsKnownPreserved:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsClientKnownPreservedDerived knownPreservedDerived:@"base"
                                                                                kp:@"preserved"
                                                                               kpd:@"derived"];
}

-(void) unknownPreservedAsBase:(ICECurrent*)__unused current
{
    TestSlicingExceptionsClientPreserved2* ex = ICE_AUTORELEASE([TestSlicingExceptionsClientPreserved2 alloc]);
    ex.b = @"base";
    ex.kp = @"preserved";
    ex.kpd = @"derived";
    ex.p1 = [TestSlicingExceptionsClientPreservedClass preservedClass:@"bc" pc:@"pc"];
    ex.p2 = ex.p1;
    @throw ex;
}

-(void) unknownPreservedAsKnownPreserved:(ICECurrent*)__unused current
{
    TestSlicingExceptionsClientPreserved2* ex = ICE_AUTORELEASE([TestSlicingExceptionsClientPreserved2 alloc]);
    ex.b = @"base";
    ex.kp = @"preserved";
    ex.kpd = @"derived";
    ex.p1 = [TestSlicingExceptionsClientPreservedClass preservedClass:@"bc" pc:@"pc"];
    ex.p2 = ex.p1;
    @throw ex;
}

@end

@interface TestSlicingExceptionsClientCallback : NSObject
{
    BOOL called;
    NSCondition* cond;
}
-(void) check;
-(void) called;
@end

@implementation TestSlicingExceptionsClientCallback
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cond = [[NSCondition alloc] init];
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [cond release];
    [super dealloc];
}
#endif

+(id) create
{
    return ICE_AUTORELEASE([[TestSlicingExceptionsClientCallback alloc] init]);
}

-(void) check
{
    [cond lock];
    while(!called)
    {
        [cond wait];
    }
    called = NO;
    [cond unlock];
}

-(void) called
{
    [cond lock];
    called = YES;
    [cond signal];
    [cond unlock];
}

-(void) response
{
    test(NO);
}

-(void) baseAsBaseException:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientBase* b)
    {
        test([b.b isEqualToString:@"Base.b"]);
        test([[b ice_id] isEqualToString:@"::Test::Base"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) unknownDerivedAsBaseException:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientBase* b)
    {
        test([b.b isEqualToString:@"UnknownDerived.b"]);
        test([[b ice_id] isEqualToString:@"::Test::Base"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) knownDerivedException:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientKnownDerived* k)
    {
        test([k.b isEqualToString:@"KnownDerived.b"]);
        test([k.kd isEqualToString:@"KnownDerived.kd"]);
        test([[k ice_id] isEqualToString:@"::Test::KnownDerived"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) unknownIntermediateAsBaseException:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientBase* b)
    {
        test([b.b isEqualToString:@"UnknownIntermediate.b"]);
        test([[b ice_id] isEqualToString:@"::Test::Base"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) knownIntermediateAsBaseException:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientKnownIntermediate* ki)
    {
        test([ki.b isEqualToString:@"KnownIntermediate.b"]);
        test([ki.ki isEqualToString:@"KnownIntermediate.ki"]);
        test([[ki ice_id] isEqualToString:@"::Test::KnownIntermediate"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) knownMostDerivedException:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientKnownMostDerived* kmd)
    {
        test([kmd.b isEqualToString:@"KnownMostDerived.b"]);
        test([kmd.ki isEqualToString:@"KnownMostDerived.ki"]);
        test([kmd.kmd isEqualToString:@"KnownMostDerived.kmd"]);
        test([[kmd ice_id] isEqualToString:@"::Test::KnownMostDerived"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) knownIntermediateAsKnownIntermediateException:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientKnownIntermediate* ki)
    {
        test([ki.b isEqualToString:@"KnownIntermediate.b"]);
        test([ki.ki isEqualToString:@"KnownIntermediate.ki"]);
        test([[ki ice_id] isEqualToString:@"::Test::KnownIntermediate"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) unknownMostDerived1Exception:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientKnownIntermediate* ki)
    {
        test([ki.b isEqualToString:@"UnknownMostDerived1.b"]);
        test([ki.ki isEqualToString:@"UnknownMostDerived1.ki"]);
        test([[ki ice_id] isEqualToString:@"::Test::KnownIntermediate"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}

-(void) unknownMostDerived2Exception:(ICEException*)exc
{
    @try
    {
        @throw exc;
    }
    @catch(TestSlicingExceptionsClientBase* b)
    {
        test([b.b isEqualToString:@"UnknownMostDerived2.b"]);
        test([[b ice_id] isEqualToString:@"::Test::Base"]);
    }
    @catch(...)
    {
        test(NO);
    }
    [self called];
}
@end

id<TestSlicingExceptionsClientTestIntfPrx>
slicingExceptionsAllTests(id<ICECommunicator> communicator)
{
    id<ICEObjectPrx> obj = [communicator stringToProxy:@"Test:default -p 12010"];
    id<TestSlicingExceptionsClientTestIntfPrx> test = [TestSlicingExceptionsClientTestIntfPrx checkedCast:obj];
    tprintf("base... ");
    {
        @try
        {
            [test baseAsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientBase* b)
        {
            test([b.b isEqual:@"Base.b"]);
            test([[b ice_id] isEqualToString:@"::Test::Base"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("base (AMI)... ");
    {
        TestSlicingExceptionsClientCallback *cb = [TestSlicingExceptionsClientCallback create];
        [test begin_baseAsBase:^ { test(NO); } exception:^(ICEException* e) { [cb baseAsBaseException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("slicing of unknown derived... ");
    {
        @try
        {
            [test unknownDerivedAsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientBase* b)
        {
            test([b.b isEqualToString:@"UnknownDerived.b"]);
            test([[b ice_id] isEqualToString:@"::Test::Base"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("slicing of unknown derived (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_unknownDerivedAsBase:^ { test(NO); } exception:^(ICEException* e) { [cb unknownDerivedAsBaseException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("non-slicing of known derived as base... ");
    {
        @try
        {
            [test knownDerivedAsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownDerived* k)
        {
            test([k.b isEqualToString:@"KnownDerived.b"]);
            test([k.kd isEqualToString:@"KnownDerived.kd"]);
            test([[k ice_id] isEqualToString:@"::Test::KnownDerived"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("non-slicing of known derived as base (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_knownDerivedAsBase:^ { test(NO); } exception:^(ICEException* e) { [cb knownDerivedException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("non-slicing of known derived as derived... ");
    {
        @try
        {
            [test knownDerivedAsKnownDerived];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownDerived* k)
        {
            test([k.b isEqualToString:@"KnownDerived.b"]);
            test([k.kd isEqualToString:@"KnownDerived.kd"]);
            test([[k ice_id] isEqualToString:@"::Test::KnownDerived"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("non-slicing of known derived as derived (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_knownDerivedAsKnownDerived:^ { test(NO); } exception:^(ICEException* e) { [cb knownDerivedException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("slicing of unknown intermediate as base... ");
    {
        @try
        {
            [test unknownIntermediateAsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientBase* b)
        {
            test([b.b isEqualToString:@"UnknownIntermediate.b"]);
            test([[b ice_id] isEqualToString:@"::Test::Base"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("slicing of unknown intermediate as base (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_unknownIntermediateAsBase:^ { test(NO); } exception:^(ICEException* e) { [cb unknownIntermediateAsBaseException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("slicing of known intermediate as base... ");
    {
        @try
        {
            [test knownIntermediateAsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownIntermediate* ki)
        {
            test([ki.b isEqualToString:@"KnownIntermediate.b"]);
            test([ki.ki isEqualToString:@"KnownIntermediate.ki"]);
            test([[ki ice_id] isEqualToString:@"::Test::KnownIntermediate"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("slicing of known intermediate as base (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_knownIntermediateAsBase:^ { test(NO); } exception:^(ICEException* e) { [cb knownIntermediateAsBaseException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("slicing of known most derived as base... ");
    {
        @try
        {
            [test knownMostDerivedAsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownMostDerived* kmd)
        {
            test([kmd.b isEqualToString:@"KnownMostDerived.b"]);
            test([kmd.ki isEqualToString:@"KnownMostDerived.ki"]);
            test([kmd.kmd isEqualToString:@"KnownMostDerived.kmd"]);
            test([[kmd ice_id] isEqualToString:@"::Test::KnownMostDerived"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("slicing of known most derived as base (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_knownMostDerivedAsBase:^ { test(NO); } exception:^(ICEException* e) { [cb knownMostDerivedException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("non-slicing of known intermediate as intermediate... ");
    {
        @try
        {
            [test knownIntermediateAsKnownIntermediate];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownIntermediate* ki)
        {
            test([ki.b isEqualToString:@"KnownIntermediate.b"]);
            test([ki.ki isEqualToString:@"KnownIntermediate.ki"]);
            test([[ki ice_id] isEqualToString:@"::Test::KnownIntermediate"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("non-slicing of known intermediate as intermediate (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_knownIntermediateAsKnownIntermediate:^ { test(NO); } exception:^(ICEException* e) { [cb knownIntermediateAsKnownIntermediateException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("non-slicing of known most derived exception as intermediate... ");
    {
        @try
        {
            [test knownMostDerivedAsKnownIntermediate];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownMostDerived* kmd)
        {
            test([kmd.b isEqualToString:@"KnownMostDerived.b"]);
            test([kmd.ki isEqualToString:@"KnownMostDerived.ki"]);
            test([kmd.kmd isEqualToString:@"KnownMostDerived.kmd"]);
            test([[kmd ice_id] isEqualToString:@"::Test::KnownMostDerived"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("non-slicing of known most derived as intermediate (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_knownMostDerivedAsKnownIntermediate:^ { test(NO); } exception:^(ICEException* e) { [cb knownMostDerivedException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("non-slicing of known most derived as most derived... ");
    {
        @try
        {
            [test knownMostDerivedAsKnownMostDerived];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownMostDerived* kmd)
        {
            test([kmd.b isEqualToString:@"KnownMostDerived.b"]);
            test([kmd.ki isEqualToString:@"KnownMostDerived.ki"]);
            test([kmd.kmd isEqualToString:@"KnownMostDerived.kmd"]);
            test([[kmd ice_id] isEqualToString:@"::Test::KnownMostDerived"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("non-slicing of known most derived as most derived (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_knownMostDerivedAsKnownMostDerived:^ { test(NO); } exception:^(ICEException* e) { [cb knownMostDerivedException:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, known intermediate as base... ");
    {
        @try
        {
            [test unknownMostDerived1AsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownIntermediate* ki)
        {
            test([ki.b isEqualToString:@"UnknownMostDerived1.b"]);
            test([ki.ki isEqualToString:@"UnknownMostDerived1.ki"]);
            test([[ki ice_id] isEqualToString:@"::Test::KnownIntermediate"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, known intermediate as base (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_unknownMostDerived1AsBase:^ { test(NO); } exception:^(ICEException* e) { [cb unknownMostDerived1Exception:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, known intermediate as intermediate... ");
    {
        @try
        {
            [test unknownMostDerived1AsKnownIntermediate];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownIntermediate* ki)
        {
            test([ki.b isEqualToString:@"UnknownMostDerived1.b"]);
            test([ki.ki isEqualToString:@"UnknownMostDerived1.ki"]);
            test([[ki ice_id] isEqualToString:@"::Test::KnownIntermediate"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_unknownMostDerived1AsKnownIntermediate:^ { test(NO); } exception:^(ICEException* e) { [cb unknownMostDerived1Exception:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, unknown intermediate as base... ");
    {
        @try
        {
            [test unknownMostDerived2AsBase];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientBase* b)
        {
            test([b.b isEqualToString:@"UnknownMostDerived2.b"]);
            test([[b ice_id] isEqualToString:@"::Test::Base"]);
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, unknown intermediate as base (AMI)... ");
    {
        TestSlicingExceptionsClientCallback* cb = [TestSlicingExceptionsClientCallback create];
        [test begin_unknownMostDerived2AsBase:^ { test(NO); } exception:^(ICEException* e) { [cb unknownMostDerived2Exception:e]; }];
        [cb check];
    }
    tprintf("ok\n");

    tprintf("unknown most derived in compact format... ");
    {
        @try
        {
            [test unknownMostDerived2AsBaseCompact];
            test(false);
        }
        @catch(TestSlicingExceptionsClientBase*)
        {
            //
            // For the 1.0 encoding, the unknown exception is sliced to Base.
            //
            test([[test ice_getEncodingVersion] isEqual:ICEEncoding_1_0]);
        }
        @catch(ICEUnknownUserException*)
        {
            //
            // An UnknownUserException is raised for the compact format because the
            // most-derived type is unknown and the exception cannot be sliced.
            //
            test(![[test ice_getEncodingVersion] isEqual:ICEEncoding_1_0]);
        }
        @catch(ICEOperationNotExistException*)
        {
        }
        @catch(...)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("preserved exceptions... ");
    {
        @try
        {
            [test unknownPreservedAsBase];
            test(false);
        }
        @catch(TestSlicingExceptionsClientBase* ex)
        {
            if([[test ice_getEncodingVersion] isEqual:ICEEncoding_1_0])
            {
                test([ex ice_getSlicedData] == nil);
            }
            else
            {
                id<ICESlicedData> slicedData = [ex ice_getSlicedData];
                test(slicedData != nil);
            }
        }

        @try
        {
            [test unknownPreservedAsKnownPreserved];
            test(false);
        }
        @catch(TestSlicingExceptionsClientKnownPreserved* ex)
        {
            test([ex.kp isEqualToString:@"preserved"]);
            if([[test ice_getEncodingVersion] isEqual:ICEEncoding_1_0])
            {
                test([ex ice_getSlicedData] == nil);
            }
            else
            {
                id<ICESlicedData> slicedData = [ex ice_getSlicedData];
                test(slicedData != nil);
            }
        }

        id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@""];
        TestSlicingExceptionsClientRelayPrx* relay =
            [TestSlicingExceptionsClientRelayPrx uncheckedCast:[adapter addWithUUID:[RelayI relay]]];
        [adapter activate];
        [[test ice_getConnection] setAdapter:adapter];
        @try
        {
            [test relayKnownPreservedAsBase:relay];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownPreservedDerived* ex)
        {
            test([ex.b isEqualToString:@"base"]);
            test([ex.kp isEqualToString:@"preserved"]);
            test([ex.kpd isEqualToString:@"derived"]);
        }
        @catch(ICEOperationNotExistException*)
        {
        }
        @catch(NSException*)
        {
            test(NO);
        }

        @try
        {
            [test relayKnownPreservedAsKnownPreserved:relay];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientKnownPreservedDerived* ex)
        {
            test([ex.b isEqualToString:@"base"]);
            test([ex.kp isEqualToString:@"preserved"]);
            test([ex.kpd isEqualToString:@"derived"]);
        }
        @catch(const ICEOperationNotExistException*)
        {
        }
        @catch(NSException*)
        {
            test(NO);
        }

        @try
        {
            [test relayUnknownPreservedAsBase:relay];
            test(NO);
        }
        @catch(TestSlicingExceptionsClientPreserved2* ex)
        {
            test([ex.b isEqualToString:@"base"]);
            test([ex.kp isEqualToString:@"preserved"]);
            test([ex.kpd isEqualToString:@"derived"]);
            test([[ex.p1 ice_id] isEqualToString:[TestSlicingExceptionsClientPreservedClass ice_staticId]]);

            test([ex.p1 isKindOfClass:[TestSlicingExceptionsClientPreservedClass class]]);
            TestSlicingExceptionsClientPreservedClass* pc = (TestSlicingExceptionsClientPreservedClass*)ex.p1;
            test([pc.bc isEqualToString:@"bc"]);
            test([pc.pc isEqualToString:@"pc"]);
            test([ex.p2 isEqual:ex.p1]);
        }
        @catch(ICEOperationNotExistException*)
        {
        }
        @catch(TestSlicingExceptionsClientKnownPreservedDerived* ex)
        {
            //
            // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
            //
            test([[test ice_getEncodingVersion] isEqual:ICEEncoding_1_0]);
            test([ex.b isEqualToString:@"base"]);
            test([ex.kp isEqualToString:@"preserved"]);
            test([ex.kpd isEqualToString:@"derived"]);
        }
        @catch(NSException*)
        {
            test(NO);
        }

        @try
        {
            [test relayUnknownPreservedAsKnownPreserved:relay];
            test(NO);
        }
        @catch(ICEOperationNotExistException*)
        {
        }
        @catch(TestSlicingExceptionsClientPreserved2* ex)
        {
            test([ex.b isEqualToString:@"base"]);
            test([ex.kp isEqualToString:@"preserved"]);
            test([ex.kpd isEqualToString:@"derived"]);
            test([[ex.p1 ice_id] isEqualToString:[TestSlicingExceptionsClientPreservedClass ice_staticId]]);
            test([ex.p1 isKindOfClass:[TestSlicingExceptionsClientPreservedClass class]]);
            TestSlicingExceptionsClientPreservedClass* pc = (TestSlicingExceptionsClientPreservedClass*)ex.p1;
            test([pc.bc isEqualToString:@"bc"]);
            test([pc.pc isEqualToString:@"pc"]);
            test([ex.p2 isEqual:ex.p1]);
        }
        @catch(TestSlicingExceptionsClientKnownPreservedDerived* ex)
        {
            //
            // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
            //
            test([[test ice_getEncodingVersion] isEqual:ICEEncoding_1_0]);
            test([ex.b isEqualToString:@"base"]);
            test([ex.kp isEqualToString:@"preserved"]);
            test([ex.kpd isEqualToString:@"derived"]);
        }
        @catch(NSException*)
        {
            test(NO);
        }

        [adapter destroy];
    }
    tprintf("ok\n");
    return test;
}
