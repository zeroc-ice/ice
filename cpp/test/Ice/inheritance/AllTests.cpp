// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

InitialPrx
allTests(Ice::CommunicatorPtr communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref("initial:tcp -p 12345 -t 2000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    InitialPrx initial = InitialPrx::checkedCast(base);
    test(initial);
    test(initial == base);
    cout << "ok" << endl;

    cout << "getting proxies for class hierarchy... " << flush;
    M_A::C_APrx ca = initial->c_a();
    M_B::C_BPrx cb = initial->c_b();
    M_A::C_CPrx cc = initial->c_c();
    M_A::C_DPrx cd = initial->c_d();
    test(ca != cb);
    test(ca != cc);
    test(ca != cd);
    test(cb != cc);
    test(cb != cd);
    test(cc != cd);
    cout << "ok" << endl;

    cout << "getting proxies for interface hierarchy... " << flush;
    M_A::I_APrx ia = initial->i_a();
    M_B::I_B1Prx ib1 = initial->i_b1();
    M_B::I_B2Prx ib2 = initial->i_b2();
    M_A::I_CPrx ic = initial->i_c();
    test(ia != ib1);
    test(ia != ib2);
    test(ia != ic);
    test(ib1 != ib2);
    test(ib1 != ic);
    test(ib2 != ic);
    cout << "ok" << endl;

    cout << "invoking proxy operations on class hierarchy... " << flush;
    M_A::C_APrx cao;
    M_B::C_BPrx cbo;
    M_A::C_CPrx cco;

    cao = ca->ca(ca);
    test(cao == ca);
    cao = ca->ca(cb);
    test(cao == cb);
    cao = ca->ca(cc);
    test(cao == cc);
    cao = cb->ca(ca);
    test(cao == ca);
    cao = cb->ca(cb);
    test(cao == cb);
    cao = cb->ca(cc);
    test(cao == cc);
    cao = cc->ca(ca);
    test(cao == ca);
    cao = cc->ca(cb);
    test(cao == cb);
    cao = cc->ca(cc);
    test(cao == cc);

    cao = cb->cb(cb);
    test(cao == cb);
    cbo = cb->cb(cb);
    test(cbo == cb);
    cao = cb->cb(cc);
    test(cao == cc);
    cbo = cb->cb(cc);
    test(cbo == cc);
    cao = cc->cb(cb);
    test(cao == cb);
    cbo = cc->cb(cb);
    test(cbo == cb);
    cao = cc->cb(cc);
    test(cao == cc);
    cbo = cc->cb(cc);
    test(cbo == cc);

    cao = cc->cc(cc);
    test(cao == cc);
    cbo = cc->cc(cc);
    test(cbo == cc);
    cco = cc->cc(cc);
    test(cco == cc);

    cout << "ok" << endl;

    cout << "ditto, but for interface hierarchy... " << flush;
    M_A::I_APrx iao;
    M_B::I_B1Prx ib1o;
    M_B::I_B2Prx ib2o;
    M_A::I_CPrx ico;

    iao = ia->ia(ia);
    test(iao == ia);
    iao = ia->ia(ib1);
    test(iao == ib1);
    iao = ia->ia(ib2);
    test(iao == ib2);
    iao = ia->ia(ic);
    test(iao == ic);
    iao = ib1->ia(ia);
    test(iao == ia);
    iao = ib1->ia(ib1);
    test(iao == ib1);
    iao = ib1->ia(ib2);
    test(iao == ib2);
    iao = ib1->ia(ic);
    test(iao == ic);
    iao = ib2->ia(ia);
    test(iao == ia);
    iao = ib2->ia(ib1);
    test(iao == ib1);
    iao = ib2->ia(ib2);
    test(iao == ib2);
    iao = ib2->ia(ic);
    test(iao == ic);
    iao = ic->ia(ia);
    test(iao == ia);
    iao = ic->ia(ib1);
    test(iao == ib1);
    iao = ic->ia(ib2);
    test(iao == ib2);
    iao = ic->ia(ic);
    test(iao == ic);

    iao = ib1->ib1(ib1);
    test(iao == ib1);
    ib1o = ib1->ib1(ib1);
    test(ib1o == ib1);
    iao = ib1->ib1(ic);
    test(iao == ic);
    ib1o = ib1->ib1(ic);
    test(ib1o == ic);
    iao = ic->ib1(ib1);
    test(iao == ib1);
    ib1o = ic->ib1(ib1);
    test(ib1o == ib1);
    iao = ic->ib1(ic);
    test(iao == ic);
    ib1o = ic->ib1(ic);
    test(ib1o == ic);

    iao = ib2->ib2(ib2);
    test(iao == ib2);
    ib2o = ib2->ib2(ib2);
    test(ib2o == ib2);
    iao = ib2->ib2(ic);
    test(iao == ic);
    ib2o = ib2->ib2(ic);
    test(ib2o == ic);
    iao = ic->ib2(ib2);
    test(iao == ib2);
    ib2o = ic->ib2(ib2);
    test(ib2o == ib2);
    iao = ic->ib2(ic);
    test(iao == ic);
    ib2o = ic->ib2(ic);
    test(ib2o == ic);

    iao = ic->ic(ic);
    test(iao == ic);
    ib1o = ic->ic(ic);
    test(ib1o == ic);
    ib2o = ic->ic(ic);
    test(ib2o == ic);
    ico = ic->ic(ic);
    test(ico == ic);

    cout << "ok" << endl;

    cout << "ditto, but for class implementing interfaces... " << flush;
    M_A::C_DPrx cdo;

    cao = cd->ca(cd);
    test(cao == cd);
    cbo = cd->cb(cd);
    test(cbo == cd);
    cco = cd->cc(cd);
    test(cco == cd);

    iao = cd->ia(cd);
    test(iao == cd);
    ib1o = cd->ib1(cd);
    test(ib1o == cd);
    ib2o = cd->ib2(cd);
    test(ib2o == cd);

    cao = cd->cd(cd);
    test(cao == cd);
    cbo = cd->cd(cd);
    test(cbo == cd);
    cco = cd->cd(cd);
    test(cco == cd);

    iao = cd->cd(cd);
    test(iao == cd);
    ib1o = cd->cd(cd);
    test(ib1o == cd);
    ib2o = cd->cd(cd);
    test(ib2o == cd);

    cout << "ok" << endl;

    return initial;
}
