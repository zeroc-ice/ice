// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

InitialPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    cout << "testing stringToProxy... " << flush;
    string ref = "initial:" + helper->getTestEndpoint();
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    InitialPrxPtr initial = ICE_CHECKED_CAST(InitialPrx, base);
    test(initial);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(initial, base));
#else
    test(initial == base);
#endif
    cout << "ok" << endl;

    cout << "getting proxies for class hierarchy... " << flush;
    MA::CAPrxPtr ca = initial->caop();
    MB::CBPrxPtr cb = initial->cbop();
    MA::CCPrxPtr cc = initial->ccop();
    MA::CDPrxPtr cd = initial->cdop();
#ifdef ICE_CPP11_MAPPING
    test(!Ice::targetEqualTo(ca, cb));
    test(!Ice::targetEqualTo(ca, cc));
    test(!Ice::targetEqualTo(ca, cd));
    test(!Ice::targetEqualTo(cb, cc));
    test(!Ice::targetEqualTo(cb, cd));
    test(!Ice::targetEqualTo(cc, cd));
#else
    test(ca != cb);
    test(ca != cc);
    test(ca != cd);
    test(cb != cc);
    test(cb != cd);
    test(cc != cd);
#endif
    cout << "ok" << endl;

    cout << "getting proxies for interface hierarchy... " << flush;
    MA::IAPrxPtr ia = initial->iaop();
    MB::IB1PrxPtr ib1 = initial->ib1op();
    MB::IB2PrxPtr ib2 = initial->ib2op();
    MA::ICPrxPtr ic = initial->icop();
#ifdef ICE_CPP11_MAPPING
    test(!Ice::targetEqualTo(ia, ib1));
    test(!Ice::targetEqualTo(ia, ib2));
    test(!Ice::targetEqualTo(ia, ic));
    test(!Ice::targetEqualTo(ib1, ic));
    test(!Ice::targetEqualTo(ib2, ic));
#else
    test(ia != ib1);
    test(ia != ib2);
    test(ia != ic);
    test(ib1 != ic);
    test(ib2 != ic);
#endif
    cout << "ok" << endl;

    cout << "invoking proxy operations on class hierarchy... " << flush;
    MA::CAPrxPtr cao;
    MB::CBPrxPtr cbo;
    MA::CCPrxPtr cco;

    cao = ca->caop(ca);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, ca));
#else
    test(cao == ca);
#endif

    cao = ca->caop(cb);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cb));
#else
    test(cao == cb);
#endif

    cao = ca->caop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cc));
#else
    test(cao == cc);
#endif

    cao = cb->caop(ca);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, ca));
#else
    test(cao == ca);
#endif

    cao = cb->caop(cb);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cb));
#else
    test(cao == cb);
#endif

    cao = cb->caop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cc));
#else
    test(cao == cc);
#endif

    cao = cc->caop(ca);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, ca));
#else
    test(cao == ca);
#endif

    cao = cc->caop(cb);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cb));
#else
    test(cao == cb);
#endif

    cao = cc->caop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cc));
#else
    test(cao == cc);
#endif

    cao = cb->cbop(cb);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cb));
#else
    test(cao == cb);
#endif

    cbo = cb->cbop(cb);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cbo, cb));
#else
    test(cbo == cb);
#endif

    cao = cb->cbop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cc));
#else
    test(cao == cc);
#endif

    cbo = cb->cbop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cbo, cc));
#else
    test(cbo == cc);
#endif

    cao = cc->cbop(cb);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cb));
#else
    test(cao == cb);
#endif

    cbo = cc->cbop(cb);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cbo, cb));
#else
    test(cbo == cb);
#endif

    cao = cc->cbop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cc));
#else
    test(cao == cc);
#endif

    cbo = cc->cbop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cbo, cc));
#else
    test(cbo == cc);
#endif

    cao = cc->ccop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cc));
#else
    test(cao == cc);
#endif

    cbo = cc->ccop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cbo, cc));
#else
    test(cbo == cc);
#endif

    cco = cc->ccop(cc);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cco, cc));
#else
    test(cco == cc);
#endif
    cout << "ok" << endl;

    cout << "ditto, but for interface hierarchy... " << flush;
    MA::IAPrxPtr iao;
    MB::IB1PrxPtr ib1o;
    MB::IB2PrxPtr ib2o;
    MA::ICPrxPtr ico;

    iao = ia->iaop(ia);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ia));
#else
    test(iao == ia);
#endif

    iao = ia->iaop(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib1));
#else
    test(iao == ib1);
#endif

    iao = ia->iaop(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib2));
#else
    test(iao == ib2);
#endif

    iao = ia->iaop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    iao = ib1->iaop(ia);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ia));
#else
    test(iao == ia);
#endif

    iao = ib1->iaop(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib1));
#else
    test(iao == ib1);
#endif

    iao = ib1->iaop(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib2));
#else
    test(iao == ib2);
#endif

    iao = ib1->iaop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    iao = ib2->iaop(ia);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ia));
#else
    test(iao == ia);
#endif

    iao = ib2->iaop(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib1));
#else
    test(iao == ib1);
#endif

    iao = ib2->iaop(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib2));
#else
    test(iao == ib2);
#endif

    iao = ib2->iaop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    iao = ic->iaop(ia);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ia));
#else
    test(iao == ia);
#endif

    iao = ic->iaop(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib1));
#else
    test(iao == ib1);
#endif

    iao = ic->iaop(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib2));
#else
    test(iao == ib2);
#endif

    iao = ic->iaop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    iao = ib1->ib1op(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib1));
#else
    test(iao == ib1);
#endif

    ib1o = ib1->ib1op(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib1o, ib1));
#else
    test(ib1o == ib1);
#endif

    iao = ib1->ib1op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    ib1o = ib1->ib1op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib1o, ic));
#else
    test(ib1o == ic);
#endif

    iao = ic->ib1op(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib1));
#else
    test(iao == ib1);
#endif

    ib1o = ic->ib1op(ib1);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib1o, ib1));
#else
    test(ib1o == ib1);
#endif

    iao = ic->ib1op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif
    ib1o = ic->ib1op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib1o, ic));
#else
    test(ib1o == ic);
#endif

    iao = ib2->ib2op(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib2));
#else
    test(iao == ib2);
#endif

    ib2o = ib2->ib2op(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib2o, ib2));
#else
    test(ib2o == ib2);
#endif

    iao = ib2->ib2op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    ib2o = ib2->ib2op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib2o, ic));
#else
    test(ib2o == ic);
#endif

    iao = ic->ib2op(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ib2));
#else
    test(iao == ib2);
#endif

    ib2o = ic->ib2op(ib2);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib2o, ib2));
#else
    test(ib2o == ib2);
#endif

    iao = ic->ib2op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    ib2o = ic->ib2op(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib2o, ic));
#else
    test(ib2o == ic);
#endif

    iao = ic->icop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, ic));
#else
    test(iao == ic);
#endif

    ib1o = ic->icop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib1o, ic));
#else
    test(ib1o == ic);
#endif

    ib2o = ic->icop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib2o, ic));
#else
    test(ib2o == ic);
#endif

    ico = ic->icop(ic);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ico, ic));
#else
    test(ico == ic);
#endif

    cout << "ok" << endl;

    cout << "ditto, but for class implementing interfaces... " << flush;
    MA::CDPrxPtr cdo;

    cao = cd->caop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cd));
#else
    test(cao == cd);
#endif

    cbo = cd->cbop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cbo, cd));
#else
    test(cbo == cd);
#endif

    cco = cd->ccop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cco, cd));
#else
    test(cco == cd);
#endif

    iao = cd->iaop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, cd));
#else
    test(iao == cd);
#endif

    ib1o = cd->ib1op(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib1o, cd));
#else
    test(ib1o == cd);
#endif

    ib2o = cd->ib2op(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib2o, cd));
#else
    test(ib2o == cd);
#endif

    cao = cd->cdop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cao, cd));
#else
    test(cao == cd);
#endif

    cbo = cd->cdop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cbo, cd));
#else
    test(cbo == cd);
#endif

    cco = cd->cdop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(cco, cd));
#else
    test(cco == cd);
#endif

    iao = cd->cdop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(iao, cd));
#else
    test(iao == cd);
#endif

    ib1o = cd->cdop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib1o, cd));
#else
    test(ib1o == cd);
#endif

    ib2o = cd->cdop(cd);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(ib2o, cd));
#else
    test(ib2o == cd);
#endif

    cout << "ok" << endl;

    cout << "testing one shot constructor... " << flush;
    {
        MC::APtr a = ICE_MAKE_SHARED(MC::A, 1);
        test(a->aA == 1);

        MC::BPtr b = ICE_MAKE_SHARED(MC::B, 1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MC::CPtr c = ICE_MAKE_SHARED(MC::C, 1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MC::DPtr d = ICE_MAKE_SHARED(MC::D, 1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MD::APtr a = ICE_MAKE_SHARED(MD::A, 1);
        test(a->aA == 1);

        MD::BPtr b = ICE_MAKE_SHARED(MD::B, 1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MD::CPtr c = ICE_MAKE_SHARED(MD::C, 1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MD::DPtr d = ICE_MAKE_SHARED(MD::D, 1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        ME::APtr a = ICE_MAKE_SHARED(ME::A, 1);
        test(a->aA == 1);

        ME::BPtr b = ICE_MAKE_SHARED(ME::B, 1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        ME::CPtr c = ICE_MAKE_SHARED(ME::C, 1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        ME::DPtr d = ICE_MAKE_SHARED(ME::D, 1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MF::APtr a = ICE_MAKE_SHARED(MF::A, 1);
        test(a->aA == 1);

        MF::BPtr b = ICE_MAKE_SHARED(MF::B, 1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MF::CPtr c = ICE_MAKE_SHARED(MF::C, 1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MF::DPtr d = ICE_MAKE_SHARED(MF::D, 1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MG::APtr a = ICE_MAKE_SHARED(MG::A, 1);
        test(a->aA == 1);

        MG::BPtr b = ICE_MAKE_SHARED(MG::B, 1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MG::CPtr c = ICE_MAKE_SHARED(MG::C, 1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MG::DPtr d = ICE_MAKE_SHARED(MG::D, 1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MH::APtr a = ICE_MAKE_SHARED(MH::A, 1);
        test(a->aA == 1);

        MH::BPtr b = ICE_MAKE_SHARED(MH::B, 1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MH::CPtr c = ICE_MAKE_SHARED(MH::C, 1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MH::DPtr d = ICE_MAKE_SHARED(MH::D, 1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }
    cout << "ok" << endl;

    return initial;
}
