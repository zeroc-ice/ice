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
#import <Key.h>
#import <Inherit.h>
#import <Clash.h>

// Verify that the expected symbols are present
@interface andbreakI : andbreak<andbreak>
@end

@interface andcharI : andchar<andchar>
@end

@interface andswitchI : andswitch<andswitch>
@end

@interface anddoI : anddo<anddo>
@end

@interface andfriendI : ICELocalObject<andfriend>
@end

//
// This section of the test is present to ensure that the C++ types
// are named correctly. It is not expected to run.
//
static void
testSymbols()
{
    andbreakPrx* prx1  = 0;
    [prx1 case_:0 try:0];
    test(prx1 == 0);

    andcharPrx* prx2 = 0;
    [prx2 explicit];
    test(prx2 == 0);

    andswitchPrx* prx3 = 0;
    [prx3 foo:0 volatile:0];
    [prx3 foo2:0 y:0 nil2:0];
    [prx3 foo3:0 nil_:0];
    test(prx3 == 0);

    anddoPrx* prx4 = 0;
    test(prx4 == 0);

    anddoPrx* prx5 = 0;
    test(prx5 == 0);

    andcontinue c1 = andasm;
    test(c1 == andasm);

    andauto* cl1 = 0;
    test(cl1 == 0);

    anddelete* cl2 = 0;
    test(cl2 == 0);

    andswitch* cl3 = 0;
    test(cl3 == 0);

    anddo* cl4 = 0;
    test(cl4 == 0);

    andreturn* ex1 = 0;
    test(ex1.signed_ == 0);
    test(ex1 == 0);

    andsizeof* ex2 = 0;
    test(ex2.signed_ == 0);
    test(ex2.static_ == 0);
    test(ex2 == 0);

    test(andtemplate == 0);
    test(andthis == 0);
    test(andthrow == 0);
    test(andtypedef == 0);
    test(andtypeid == 0);
    test(andtypename == 0);
    test(andunion == 0);
    test(andunsigned == 0);
    test(andusing == 0);
    test(andvirtual == 0);
    test(andwhile == 0);
    test(andxor == 0);

    TestStruct1* ds1 = 0;
    test(ds1.retainCount_ == 0);

    TestException1 *dex1 = 0;
    test(dex1.isa_ == 0);
    test(dex1.reason_ == 0);
    test(dex1.raise_ == 0);
    test(dex1.name_ == 0);
    test(dex1.callStackReturnAddresses_ == 0);
    test(dex1.userInfo_ == 0);
    test(dex1.reserved_ == 0);

    TestClass1* dcl1 = 0;
    test(dcl1.reason == 0);
    test(dcl1.autorelease_ == 0);
    test(dcl1.isa_ == 0);
    test(dcl1.classForCoder_ == 0);
    test(dcl1.copy_ == 0);
    test(dcl1.dealloc_ == 0);
    test(dcl1.description_ == 0);
    test(dcl1.hash_ == 0);
    test(dcl1.init_ == 0);
    test(dcl1.isProxy_ == 0);
    test(dcl1.mutableCopy_ == 0);
    test(dcl1.release_ == 0);
    test(dcl1.retain_ == 0);
    test(dcl1.retainCount_ == 0);
    test(dcl1.self_ == 0);
    test(dcl1.superclass_ == 0);
    test(dcl1.zone_ == 0);

    TestIntf1Prx* dif1 = 0;
    [dif1 reason];
    [dif1 isa_];
    [dif1 autorelease_];
    [dif1 classForCoder_];
    [dif1 copy_];
    [dif1 dealloc_];
    [dif1 description_];
    [dif1 hash_];
    [dif1 init_];
    [dif1 isProxy_];
    [dif1 mutableCopy_];
    [dif1 release_];
    [dif1 retain_];
    [dif1 retainCount_];
    [dif1 self_];
    [dif1 superclass_];
    [dif1 zone_];

    TestIntf2Prx* dif2 = 0;
    [dif2 reason:0];
    [dif2 isa:0];
    [dif2 autorelease:0];
    [dif2 classForCoder:0];
    [dif2 copy:0];
    [dif2 dealloc:0];
    [dif2 description:0];
    [dif2 hash:0];
    [dif2 init:0];
    [dif2 isProxy:0];
    [dif2 mutableCopy:0];
    [dif2 release:0];
    [dif2 retain:0];
    [dif2 retainCount:0];
    [dif2 self_:0];
    [dif2 superclass:0];
    [dif2 zone:0];
}

static int
run(id<ICECommunicator> communicator)
{
    if(0)
    {
        testSymbols();
    }
    return 0;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
#endif

    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;

        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultClientProperties(&argc, argv);
            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator);
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }
        @catch(TestFailedException* ex)
        {
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
