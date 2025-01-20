// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

InitialPrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    InitialPrx initial(communicator, "initial:" + helper->getTestEndpoint());

    cout << "getting proxies for interface hierarchy... " << flush;
    optional<MA::IAPrx> ia = initial->iaop();
    optional<MB::IB1Prx> ib1 = initial->ib1op();
    optional<MB::IB2Prx> ib2 = initial->ib2op();
    optional<MA::ICPrx> ic = initial->icop();
    test(ia != ib1);
    test(ia != ib2);
    test(ia != ic);
    test(ib1 != ic);
    test(ib2 != ic);
    cout << "ok" << endl;

    cout << "invoking proxy operations on interface hierarchy... " << flush;
    optional<MA::IAPrx> iao;
    optional<MB::IB1Prx> ib1o;
    optional<MB::IB2Prx> ib2o;
    optional<MA::ICPrx> ico;

    iao = ia->iaop(ia);
    test(iao == ia);

    iao = ia->iaop(ib1);
    test(iao == ib1);

    iao = ia->iaop(ib2);
    test(iao == ib2);

    iao = ia->iaop(ic);
    test(iao == ic);

    iao = ib1->iaop(ia);
    test(iao == ia);

    iao = ib1->iaop(ib1);
    test(iao == ib1);

    iao = ib1->iaop(ib2);
    test(iao == ib2);

    iao = ib1->iaop(ic);
    test(iao == ic);

    iao = ib2->iaop(ia);
    test(iao == ia);

    iao = ib2->iaop(ib1);
    test(iao == ib1);

    iao = ib2->iaop(ib2);
    test(iao == ib2);

    iao = ib2->iaop(ic);
    test(iao == ic);

    iao = ic->iaop(ia);
    test(iao == ia);

    iao = ic->iaop(ib1);
    test(iao == ib1);

    iao = ic->iaop(ib2);
    test(iao == ib2);

    iao = ic->iaop(ic);
    test(iao == ic);

    iao = ib1->ib1op(ib1);
    test(iao == ib1);

    ib1o = ib1->ib1op(ib1);
    test(ib1o == ib1);

    iao = ib1->ib1op(ic);
    test(iao == ic);

    ib1o = ib1->ib1op(ic);
    test(ib1o == ic);

    iao = ic->ib1op(ib1);
    test(iao == ib1);

    ib1o = ic->ib1op(ib1);
    test(ib1o == ib1);

    iao = ic->ib1op(ic);
    test(iao == ic);
    ib1o = ic->ib1op(ic);
    test(ib1o == ic);

    iao = ib2->ib2op(ib2);
    test(iao == ib2);

    ib2o = ib2->ib2op(ib2);
    test(ib2o == ib2);

    iao = ib2->ib2op(ic);
    test(iao == ic);

    ib2o = ib2->ib2op(ic);
    test(ib2o == ic);

    iao = ic->ib2op(ib2);
    test(iao == ib2);

    ib2o = ic->ib2op(ib2);
    test(ib2o == ib2);

    iao = ic->ib2op(ic);
    test(iao == ic);

    ib2o = ic->ib2op(ic);
    test(ib2o == ic);

    iao = ic->icop(ic);
    test(iao == ic);

    ib1o = ic->icop(ic);
    test(ib1o == ic);

    ib2o = ic->icop(ic);
    test(ib2o == ic);

    ico = ic->icop(ic);
    test(ico == ic);

    cout << "ok" << endl;

    cout << "testing one shot constructor... " << flush;
    {
        MC::APtr a = make_shared<MC::A>(1);
        test(a->aA == 1);

        MC::BPtr b = make_shared<MC::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MC::CPtr c = make_shared<MC::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MC::DPtr d = make_shared<MC::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MD::APtr a = make_shared<MD::A>(1);
        test(a->aA == 1);

        MD::BPtr b = make_shared<MD::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MD::CPtr c = make_shared<MD::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MD::DPtr d = make_shared<MD::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        ME::APtr a = make_shared<ME::A>(1);
        test(a->aA == 1);

        ME::BPtr b = make_shared<ME::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        ME::CPtr c = make_shared<ME::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        ME::DPtr d = make_shared<ME::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MF::APtr a = make_shared<MF::A>(1);
        test(a->aA == 1);

        MF::BPtr b = make_shared<MF::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MF::CPtr c = make_shared<MF::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MF::DPtr d = make_shared<MF::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MG::APtr a = make_shared<MG::A>(1);
        test(a->aA == 1);

        MG::BPtr b = make_shared<MG::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MG::CPtr c = make_shared<MG::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MG::DPtr d = make_shared<MG::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }

    {
        MH::APtr a = make_shared<MH::A>(1);
        test(a->aA == 1);

        MH::BPtr b = make_shared<MH::B>(1, 2);
        test(b->aA == 1);
        test(b->bB == 2);

        MH::CPtr c = make_shared<MH::C>(1, 2, 3);
        test(c->aA == 1);
        test(c->bB == 2);
        test(c->cC == 3);

        MH::DPtr d = make_shared<MH::D>(1, 2, 3, 4);
        test(d->aA == 1);
        test(d->bB == 2);
        test(d->cC == 3);
        test(d->dD == 4);
    }
    cout << "ok" << endl;

    return initial;
}
