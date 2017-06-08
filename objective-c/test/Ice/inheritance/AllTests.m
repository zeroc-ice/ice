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
#import <InheritanceTest.h>

id<TestInheritanceInitialPrx>
inheritanceAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing stringToProxy... ");
    NSString* ref = @"initial:default -p 12010";
    id<ICEObjectPrx> base = [communicator stringToProxy:ref];
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestInheritanceInitialPrx> initial = [TestInheritanceInitialPrx checkedCast:base];
    test(initial);
    test([initial isEqual:base]);
    tprintf("ok\n");

    tprintf("getting proxies for class hierarchy... ");
    id<TestInheritanceMACAPrx> ca = [initial caop];
    id<TestInheritanceMBCBPrx> cb = [initial cbop];
    id<TestInheritanceMACCPrx> cc = [initial ccop];
    id<TestInheritanceMACDPrx> cd = [initial cdop];
    test(ca != cb);
    test(ca != cc);
    test(ca != cd);
    test(cb != cc);
    test(cb != cd);
    test(cc != cd);
    tprintf("ok\n");

    tprintf("getting proxies for interface hierarchy... ");
    id<TestInheritanceMAIAPrx> ia = [initial iaop];
    id<TestInheritanceMBIB1Prx> ib1 = [initial ib1op];
    id<TestInheritanceMBIB2Prx> ib2 = [initial ib2op];
    id<TestInheritanceMAICPrx> ic = [initial icop];
    test(ia != ib1);
    test(ia != ib2);
    test(ia != ic);
    test(ib1 != ic);
    test(ib2 != ic);
    tprintf("ok\n");

    tprintf("invoking proxy operations on class hierarchy... ");
    id<TestInheritanceMACAPrx> cao;
    id<TestInheritanceMBCBPrx> cbo;
    id<TestInheritanceMACCPrx> cco;

    cao = [ca caop:ca];
    test([cao isEqual:ca]);
    cao = [ca caop:cb];
    test([cao isEqual:cb]);
    cao = [ca caop:cc];
    test([cao isEqual:cc]);
    cao = [cb caop:ca];
    test([cao isEqual:ca]);
    cao = [cb caop:cb];
    test([cao isEqual:cb]);
    cao = [cb caop:cc];
    test([cao isEqual:cc]);
    cao = [cc caop:ca];
    test([cao isEqual:ca]);
    cao = [cc caop:cb];
    test([cao isEqual:cb]);
    cao = [cc caop:cc];
    test([cao isEqual:cc]);

    cao = [cb cbop:cb];
    test([cao isEqual:cb]);
    cbo = [cb cbop:cb];
    test([cbo isEqual:cb]);
    cao = [cb cbop:cc];
    test([cao isEqual:cc]);
    cbo = [cb cbop:cc];
    test([cbo isEqual:cc]);
    cao = [cc cbop:cb];
    test([cao isEqual:cb]);
    cbo = [cc cbop:cb];
    test([cbo isEqual:cb]);
    cao = [cc cbop:cc];
    test([cao isEqual:cc]);
    cbo = [cc cbop:cc];
    test([cbo isEqual:cc]);

    cao = [cc ccop:cc];
    test([cao isEqual:cc]);
    cbo = [cc ccop:cc];
    test([cbo isEqual:cc]);
    cco = [cc ccop:cc];
    test([cco isEqual:cc]);
    tprintf("ok\n");

    tprintf("ditto, but for interface hierarchy... ");
    id<TestInheritanceMAIAPrx> iao;
    id<TestInheritanceMBIB1Prx> ib1o;
    id<TestInheritanceMBIB2Prx> ib2o;
    id<TestInheritanceMAICPrx> ico;

    iao = [ia iaop:ia];
    test([iao isEqual:ia]);
    iao = [ia iaop:ib1];
    test([iao isEqual:ib1]);
    iao = [ia iaop:ib2];
    test([iao isEqual:ib2]);
    iao = [ia iaop:ic];
    test([iao isEqual:ic]);
    iao = [ib1 iaop:ia];
    test([iao isEqual:ia]);
    iao = [ib1 iaop:ib1];
    test([iao isEqual:ib1]);
    iao = [ib1 iaop:ib2];
    test([iao isEqual:ib2]);
    iao = [ib1 iaop:ic];
    test([iao isEqual:ic]);
    iao = [ib2 iaop:ia];
    test([iao isEqual:ia]);
    iao = [ib2 iaop:ib1];
    test([iao isEqual:ib1]);
    iao = [ib2 iaop:ib2];
    test([iao isEqual:ib2]);
    iao = [ib2 iaop:ic];
    test([iao isEqual:ic]);
    iao = [ic iaop:ia];
    test([iao isEqual:ia]);
    iao = [ic iaop:ib1];
    test([iao isEqual:ib1]);
    iao = [ic iaop:ib2];
    test([iao isEqual:ib2]);
    iao = [ic iaop:ic];
    test([iao isEqual:ic]);

    iao = [ib1 ib1op:ib1];
    test([iao isEqual:ib1]);
    ib1o = [ib1 ib1op:ib1];
    test([ib1o isEqual:ib1]);
    iao = [ib1 ib1op:ic];
    test([iao isEqual:ic]);
    ib1o = [ib1 ib1op:ic];
    test([ib1o isEqual:ic]);
    iao = [ic ib1op:ib1];
    test([iao isEqual:ib1]);
    ib1o = [ic ib1op:ib1];
    test([ib1o isEqual:ib1]);
    iao = [ic ib1op:ic];
    test([iao isEqual:ic]);
    ib1o = [ic ib1op:ic];
    test([ib1o isEqual:ic]);

    iao = [ib2 ib2op:ib2];
    test([iao isEqual:ib2]);
    ib2o = [ib2 ib2op:ib2];
    test([ib2o isEqual:ib2]);
    iao = [ib2 ib2op:ic];
    test([iao isEqual:ic]);
    ib2o = [ib2 ib2op:ic];
    test([ib2o isEqual:ic]);
    iao = [ic ib2op:ib2];
    test([iao isEqual:ib2]);
    ib2o = [ic ib2op:ib2];
    test([ib2o isEqual:ib2]);
    iao = [ic ib2op:ic];
    test([iao isEqual:ic]);
    ib2o = [ic ib2op:ic];
    test([ib2o isEqual:ic]);

    iao = [ic icop:ic];
    test([iao isEqual:ic]);
    ib1o = [ic icop:ic];
    test([ib1o isEqual:ic]);
    ib2o = [ic icop:ic];
    test([ib2o isEqual:ic]);
    ico = [ic icop:ic];
    test([ico isEqual:ic]);

    tprintf("ok\n");

    tprintf("ditto, but for class implementing interfaces... ");
//    id<TestInheritanceMACDPrx> cdo;

    cao = [cd caop:cd];
    test([cao isEqual:cd]);
    cbo = [cd cbop:cd];
    test([cbo isEqual:cd]);
    cco = [cd ccop:cd];
    test([cco isEqual:cd]);

    iao = [cd iaop:cd];
    test([iao isEqual:cd]);
    ib1o = [cd ib1op:cd];
    test([ib1o isEqual:cd]);
    ib2o = [cd ib2op:cd];
    test([ib2o isEqual:cd]);

    cao = [cd cdop:cd];
    test([cao isEqual:cd]);
    cbo = [cd cdop:cd];
    test([cbo isEqual:cd]);
    cco = [cd cdop:cd];
    test([cco isEqual:cd]);

    iao = [cd cdop:cd];
    test([iao isEqual:cd]);
    ib1o = [cd cdop:cd];
    test([ib1o isEqual:cd]);
    ib2o = [cd cdop:cd];
    test([ib2o isEqual:cd]);

    tprintf("ok\n");

    return initial;
}
