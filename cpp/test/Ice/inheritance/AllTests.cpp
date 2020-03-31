//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    test(Ice::targetEqualTo(initial, base));
    cout << "ok" << endl;

    cout << "getting proxies for class hierarchy... " << flush;
    MA::CAPrxPtr ca = initial->caop();
    MB::CBPrxPtr cb = initial->cbop();
    MA::CCPrxPtr cc = initial->ccop();
    MA::CDPrxPtr cd = initial->cdop();
    test(!Ice::targetEqualTo(ca, cb));
    test(!Ice::targetEqualTo(ca, cc));
    test(!Ice::targetEqualTo(ca, cd));
    test(!Ice::targetEqualTo(cb, cc));
    test(!Ice::targetEqualTo(cb, cd));
    test(!Ice::targetEqualTo(cc, cd));
    cout << "ok" << endl;

    cout << "getting proxies for interface hierarchy... " << flush;
    MA::IAPrxPtr ia = initial->iaop();
    MB::IB1PrxPtr ib1 = initial->ib1op();
    MB::IB2PrxPtr ib2 = initial->ib2op();
    MA::ICPrxPtr ic = initial->icop();
    test(!Ice::targetEqualTo(ia, ib1));
    test(!Ice::targetEqualTo(ia, ib2));
    test(!Ice::targetEqualTo(ia, ic));
    test(!Ice::targetEqualTo(ib1, ic));
    test(!Ice::targetEqualTo(ib2, ic));
    cout << "ok" << endl;

    cout << "invoking proxy operations on class hierarchy... " << flush;
    MA::CAPrxPtr cao;
    MB::CBPrxPtr cbo;
    MA::CCPrxPtr cco;

    cao = ca->caop(ca);
    test(Ice::targetEqualTo(cao, ca));

    cao = ca->caop(cb);
    test(Ice::targetEqualTo(cao, cb));

    cao = ca->caop(cc);
    test(Ice::targetEqualTo(cao, cc));

    cao = cb->caop(ca);
    test(Ice::targetEqualTo(cao, ca));

    cao = cb->caop(cb);
    test(Ice::targetEqualTo(cao, cb));

    cao = cb->caop(cc);
    test(Ice::targetEqualTo(cao, cc));

    cao = cc->caop(ca);
    test(Ice::targetEqualTo(cao, ca));

    cao = cc->caop(cb);
    test(Ice::targetEqualTo(cao, cb));

    cao = cc->caop(cc);
    test(Ice::targetEqualTo(cao, cc));

    cao = cb->cbop(cb);
    test(Ice::targetEqualTo(cao, cb));

    cbo = cb->cbop(cb);
    test(Ice::targetEqualTo(cbo, cb));

    cao = cb->cbop(cc);
    test(Ice::targetEqualTo(cao, cc));

    cbo = cb->cbop(cc);
    test(Ice::targetEqualTo(cbo, cc));

    cao = cc->cbop(cb);
    test(Ice::targetEqualTo(cao, cb));

    cbo = cc->cbop(cb);
    test(Ice::targetEqualTo(cbo, cb));

    cao = cc->cbop(cc);
    test(Ice::targetEqualTo(cao, cc));

    cbo = cc->cbop(cc);
    test(Ice::targetEqualTo(cbo, cc));

    cao = cc->ccop(cc);
    test(Ice::targetEqualTo(cao, cc));

    cbo = cc->ccop(cc);
    test(Ice::targetEqualTo(cbo, cc));

    cco = cc->ccop(cc);
    test(Ice::targetEqualTo(cco, cc));
    cout << "ok" << endl;

    cout << "ditto, but for interface hierarchy... " << flush;
    MA::IAPrxPtr iao;
    MB::IB1PrxPtr ib1o;
    MB::IB2PrxPtr ib2o;
    MA::ICPrxPtr ico;

    iao = ia->iaop(ia);
    test(Ice::targetEqualTo(iao, ia));

    iao = ia->iaop(ib1);
    test(Ice::targetEqualTo(iao, ib1));

    iao = ia->iaop(ib2);
    test(Ice::targetEqualTo(iao, ib2));

    iao = ia->iaop(ic);
    test(Ice::targetEqualTo(iao, ic));

    iao = ib1->iaop(ia);
    test(Ice::targetEqualTo(iao, ia));

    iao = ib1->iaop(ib1);
    test(Ice::targetEqualTo(iao, ib1));

    iao = ib1->iaop(ib2);
    test(Ice::targetEqualTo(iao, ib2));

    iao = ib1->iaop(ic);
    test(Ice::targetEqualTo(iao, ic));

    iao = ib2->iaop(ia);
    test(Ice::targetEqualTo(iao, ia));

    iao = ib2->iaop(ib1);
    test(Ice::targetEqualTo(iao, ib1));

    iao = ib2->iaop(ib2);
    test(Ice::targetEqualTo(iao, ib2));

    iao = ib2->iaop(ic);
    test(Ice::targetEqualTo(iao, ic));

    iao = ic->iaop(ia);
    test(Ice::targetEqualTo(iao, ia));

    iao = ic->iaop(ib1);
    test(Ice::targetEqualTo(iao, ib1));

    iao = ic->iaop(ib2);
    test(Ice::targetEqualTo(iao, ib2));

    iao = ic->iaop(ic);
    test(Ice::targetEqualTo(iao, ic));

    iao = ib1->ib1op(ib1);
    test(Ice::targetEqualTo(iao, ib1));

    ib1o = ib1->ib1op(ib1);
    test(Ice::targetEqualTo(ib1o, ib1));

    iao = ib1->ib1op(ic);
    test(Ice::targetEqualTo(iao, ic));

    ib1o = ib1->ib1op(ic);
    test(Ice::targetEqualTo(ib1o, ic));

    iao = ic->ib1op(ib1);
    test(Ice::targetEqualTo(iao, ib1));

    ib1o = ic->ib1op(ib1);
    test(Ice::targetEqualTo(ib1o, ib1));

    iao = ic->ib1op(ic);
    test(Ice::targetEqualTo(iao, ic));
    ib1o = ic->ib1op(ic);
    test(Ice::targetEqualTo(ib1o, ic));

    iao = ib2->ib2op(ib2);
    test(Ice::targetEqualTo(iao, ib2));

    ib2o = ib2->ib2op(ib2);
    test(Ice::targetEqualTo(ib2o, ib2));

    iao = ib2->ib2op(ic);
    test(Ice::targetEqualTo(iao, ic));

    ib2o = ib2->ib2op(ic);
    test(Ice::targetEqualTo(ib2o, ic));

    iao = ic->ib2op(ib2);
    test(Ice::targetEqualTo(iao, ib2));

    ib2o = ic->ib2op(ib2);
    test(Ice::targetEqualTo(ib2o, ib2));

    iao = ic->ib2op(ic);
    test(Ice::targetEqualTo(iao, ic));

    ib2o = ic->ib2op(ic);
    test(Ice::targetEqualTo(ib2o, ic));

    iao = ic->icop(ic);
    test(Ice::targetEqualTo(iao, ic));

    ib1o = ic->icop(ic);
    test(Ice::targetEqualTo(ib1o, ic));

    ib2o = ic->icop(ic);
    test(Ice::targetEqualTo(ib2o, ic));

    ico = ic->icop(ic);
    test(Ice::targetEqualTo(ico, ic));

    cout << "ok" << endl;

    cout << "ditto, but for class implementing interfaces... " << flush;
    MA::CDPrxPtr cdo;

    cao = cd->caop(cd);
    test(Ice::targetEqualTo(cao, cd));

    cbo = cd->cbop(cd);
    test(Ice::targetEqualTo(cbo, cd));

    cco = cd->ccop(cd);
    test(Ice::targetEqualTo(cco, cd));

    iao = cd->iaop(cd);
    test(Ice::targetEqualTo(iao, cd));

    ib1o = cd->ib1op(cd);
    test(Ice::targetEqualTo(ib1o, cd));

    ib2o = cd->ib2op(cd);
    test(Ice::targetEqualTo(ib2o, cd));

    cao = cd->cdop(cd);
    test(Ice::targetEqualTo(cao, cd));

    cbo = cd->cdop(cd);
    test(Ice::targetEqualTo(cbo, cd));

    cco = cd->cdop(cd);
    test(Ice::targetEqualTo(cco, cd));

    iao = cd->cdop(cd);
    test(Ice::targetEqualTo(iao, cd));

    ib1o = cd->cdop(cd);
    test(Ice::targetEqualTo(ib1o, cd));

    ib2o = cd->cdop(cd);
    test(Ice::targetEqualTo(ib2o, cd));

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
