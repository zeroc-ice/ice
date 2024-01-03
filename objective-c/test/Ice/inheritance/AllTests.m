//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <InheritanceTest.h>

id<TestInheritanceInitialPrx>
inheritanceAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing stringToProxy... ");
    NSString* ref = [NSString stringWithFormat:@"initial:%@", getTestEndpoint(communicator, 0)];
    id<ICEObjectPrx> base = [communicator stringToProxy:ref];
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestInheritanceInitialPrx> initial = [TestInheritanceInitialPrx checkedCast:base];
    test(initial);
    test([initial isEqual:base]);
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

    tprintf("invoking proxy operations on interface hierarchy... ");
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

    return initial;
}
