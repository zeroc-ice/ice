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

    cout << "invoking proxy operations for interface hierarchy... " << flush;
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

    cout << "testing one shot constructor... " << flush;
    {
        MC::APtr a = std::make_shared<MC::A>(1);
        test(a->aA == 1);

        MC::BPtr b = std::make_shared<MC::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MC::CPtr c = std::make_shared<MC::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MC::DPtr d = std::make_shared<MC::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MD::APtr a = std::make_shared<MD::A>(1);
        test(a->aA == 1);

        MD::BPtr b = std::make_shared<MD::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MD::CPtr c = std::make_shared<MD::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MD::DPtr d = std::make_shared<MD::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        ME::APtr a = std::make_shared<ME::A>(1);
        test(a->aA == 1);

        ME::BPtr b = std::make_shared<ME::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        ME::CPtr c = std::make_shared<ME::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        ME::DPtr d = std::make_shared<ME::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MF::APtr a = std::make_shared<MF::A>(1);
        test(a->aA == 1);

        MF::BPtr b = std::make_shared<MF::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MF::CPtr c = std::make_shared<MF::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MF::DPtr d = std::make_shared<MF::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MG::APtr a = std::make_shared<MG::A>(1);
        test(a->aA == 1);

        MG::BPtr b = std::make_shared<MG::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MG::CPtr c = std::make_shared<MG::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MG::DPtr d = std::make_shared<MG::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MH::APtr a = std::make_shared<MH::A>(1);
        test(a->aA == 1);

        MH::BPtr b = std::make_shared<MH::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MH::CPtr c = std::make_shared<MH::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MH::DPtr d = std::make_shared<MH::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }
    cout << "ok" << endl;

    return initial;
}
