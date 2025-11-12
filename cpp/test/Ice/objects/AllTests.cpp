// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Test;

namespace
{
    void clear(const CPtr&);

    void clear(const BPtr& b)
    {
        if (dynamic_pointer_cast<B>(b->theA))
        {
            auto tmp = b->theA;
            b->theA = nullptr;
            clear(dynamic_pointer_cast<B>(tmp));
        }
        if (b->theB)
        {
            auto tmp = b->theB;
            b->theB = nullptr;
            clear(dynamic_pointer_cast<B>(tmp));
        }
        b->theC = nullptr;
    }

    void clear(const CPtr& c)
    {
        clear(c->theB);
        c->theB = nullptr;
    }

    void clear(const DPtr& d)
    {
        if (dynamic_pointer_cast<B>(d->theA))
        {
            clear(dynamic_pointer_cast<B>(d->theA));
        }
        d->theA = nullptr;
        clear(d->theB);
        d->theB = nullptr;
    }
}

InitialPrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    InitialPrx initial(communicator, "initial:" + helper->getTestEndpoint());

    cout << "testing constructor, copy constructor, and assignment operator... " << flush;

    BasePtr ba1 = make_shared<Base>();
    test(ba1->theS.str == "");
    test(ba1->str == "");

    S s;
    s.str = "hello";
    BasePtr ba2 = make_shared<Base>(s, "hi");
    test(ba2->theS.str == "hello");
    test(ba2->str == "hi");

    auto [s2, str2] = ba2->ice_tuple();
    test(s2 == s);
    test(str2 == "hi");

    ba1 = ba2->ice_clone();
    test(ba1->theS.str == "hello");
    test(ba1->str == "hi");

    BasePtr bp1 = make_shared<Base>();
    bp1 = ba2->ice_clone();
    test(bp1->theS.str == "hello");
    test(bp1->str == "hi");

    cout << "ok" << endl;

    cout << "testing ice_clone..." << flush;

    BasePtr bp2 = dynamic_pointer_cast<Base>(bp1->ice_clone());
    test(bp1->theS.str == bp2->theS.str);
    test(bp1->str == bp2->str);

    cout << "ok" << endl;

    cout << "getting B1... " << flush;
    BPtr b1 = initial->getB1();
    test(b1);
    cout << "ok" << endl;

    cout << "getting B2... " << flush;
    BPtr b2 = initial->getB2();
    test(b2);
    cout << "ok" << endl;

    cout << "getting C... " << flush;
    CPtr c = initial->getC();
    test(c);
    cout << "ok" << endl;

    cout << "getting D... " << flush;
    DPtr d = initial->getD();
    test(d);
    cout << "ok" << endl;

    cout << "checking consistency... " << flush;
    test(b1 != b2);
    test(b1 != dynamic_pointer_cast<B>(c));
    test(b1 != dynamic_pointer_cast<B>(d));
    test(b2 != dynamic_pointer_cast<B>(c));
    test(b2 != dynamic_pointer_cast<B>(d));
    test(c != dynamic_pointer_cast<C>(d));

    test(b1->theB == b1);
    test(b1->theC == nullptr);
    test(dynamic_pointer_cast<B>(b1->theA));
    test(dynamic_pointer_cast<B>(b1->theA)->theA == b1->theA);
    test(dynamic_pointer_cast<B>(b1->theA)->theB == b1);
    test(dynamic_pointer_cast<C>(dynamic_pointer_cast<B>(b1->theA)->theC));
    test(dynamic_pointer_cast<C>(dynamic_pointer_cast<B>(b1->theA)->theC)->theB == b1->theA);

    test(b1->preMarshalInvoked);
    test(b1->postUnmarshalInvoked);
    test(b1->theA->preMarshalInvoked);
    test(b1->theA->postUnmarshalInvoked);
    test(dynamic_pointer_cast<B>(b1->theA)->theC->preMarshalInvoked);
    test(dynamic_pointer_cast<B>(b1->theA)->theC->postUnmarshalInvoked);

    // More tests possible for b2 and d, but I think this is already sufficient.
    test(b2->theA == b2);
    test(d->theC == nullptr);

    clear(b1);
    clear(b2);
    clear(c);
    clear(d);
    cout << "ok" << endl;

    cout << "getting B1, B2, C, and D all at once... " << flush;
    initial->getAll(b1, b2, c, d);
    test(b1);
    test(b2);
    test(c);
    test(d);
    cout << "ok" << endl;

    cout << "testing renamed classes... " << flush;
    auto renamed = make_shared<Test::CppClass>("renamed");
    renamed = initial->opRenamedClass(renamed);
    test(renamed != nullptr);
    test(renamed->data == "renamed");

    auto derivedRenamed = make_shared<Test::CppDerivedClass>("renamed", "derived");
    derivedRenamed = dynamic_pointer_cast<Test::CppDerivedClass>(initial->opRenamedClass(derivedRenamed));
    test(derivedRenamed != nullptr);
    test(derivedRenamed->data == "renamed");
    test(derivedRenamed->moreData == "derived");
    cout << "ok" << endl;

    cout << "checking consistency... " << flush;
    test(b1 != b2);
    test(b1 != dynamic_pointer_cast<B>(c));
    test(b1 != dynamic_pointer_cast<B>(d));
    test(b2 != dynamic_pointer_cast<B>(c));
    test(b2 != dynamic_pointer_cast<B>(d));
    test(c != dynamic_pointer_cast<C>(d));
    test(b1->theA == dynamic_pointer_cast<B>(b2));
    test(b1->theB == dynamic_pointer_cast<B>(b1));
    test(b1->theC == nullptr);
    test(b2->theA == b2);
    test(b2->theB == b1);
    test(b2->theC == dynamic_pointer_cast<C>(c));
    test(c->theB == b2);
    test(d->theA == dynamic_pointer_cast<A>(b1));
    test(d->theB == dynamic_pointer_cast<B>(b2));
    test(d->theC == nullptr);

    test(d->preMarshalInvoked);
    test(d->postUnmarshalInvoked);
    test(d->theA->preMarshalInvoked);
    test(d->theA->postUnmarshalInvoked);
    test(d->theB->preMarshalInvoked);
    test(d->theB->postUnmarshalInvoked);
    test(d->theB->theC->preMarshalInvoked);
    test(d->theB->theC->postUnmarshalInvoked);
    clear(b1);
    clear(b2);
    clear(c);
    clear(d);
    cout << "ok" << endl;

    cout << "getting K... " << flush;
    {
        KPtr k = initial->getK();
        LPtr l = dynamic_pointer_cast<L>(k->value);
        test(l);
        test(l->data == "l");
    }
    cout << "ok" << endl;

    cout << "testing Value as parameter..." << flush;
    {
        LPtr v1 = make_shared<L>("l");
        Ice::ValuePtr v2;
        Ice::ValuePtr v3 = initial->opValue(v1, v2);
        test(dynamic_pointer_cast<L>(v2)->data == "l");
        test(dynamic_pointer_cast<L>(v3)->data == "l");
    }

    {
        LPtr l = make_shared<L>("l");
        Test::ValueSeq v1;
        v1.push_back(l);
        Test::ValueSeq v2;
        Test::ValueSeq v3 = initial->opValueSeq(v1, v2);
        test(dynamic_pointer_cast<L>(v2[0])->data == "l");
        test(dynamic_pointer_cast<L>(v3[0])->data == "l");
    }

    {
        LPtr l = make_shared<L>("l");
        Test::ValueMap v1;
        v1["l"] = l;
        Test::ValueMap v2;
        Test::ValueMap v3 = initial->opValueMap(v1, v2);
        test(dynamic_pointer_cast<L>(v2["l"])->data == "l");
        test(dynamic_pointer_cast<L>(v3["l"])->data == "l");
    }
    cout << "ok" << endl;

    cout << "getting D1... " << flush;
    D1Ptr d1 =
        make_shared<D1>(make_shared<A1>("a1"), make_shared<A1>("a2"), make_shared<A1>("a3"), make_shared<A1>("a4"));
    d1 = initial->getD1(d1);
    test(d1->a1->name == "a1");
    test(d1->a2->name == "a2");
    test(d1->a3->name == "a3");
    test(d1->a4->name == "a4");
    cout << "ok" << endl;

    cout << "throw EDerived... " << flush;
    try
    {
        initial->throwEDerived();
        test(false);
    }
    catch (const EDerived& ederived)
    {
        test(ederived.a1->name == "a1");
        test(ederived.a2->name == "a2");
        test(ederived.a3->name == "a3");
        test(ederived.a4->name == "a4");
    }
    cout << "ok" << endl;

    cout << "setting G... " << flush;
    GPtr g = make_shared<G>(s, "g");
    try
    {
        initial->setG(g);
    }
    catch (const Ice::OperationNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing sequences... " << flush;
    BaseSeq inS, outS, retS;
    retS = initial->opBaseSeq(inS, outS);

    inS.resize(1);
    inS[0] = make_shared<Base>();
    retS = initial->opBaseSeq(inS, outS);
    test(retS.size() == 1 && outS.size() == 1);
    cout << "ok" << endl;

    cout << "testing recursive type... " << flush;
    RecursivePtr top = make_shared<Recursive>();
    RecursivePtr bottom = top;
    int maxDepth = 10;
    for (int i = 1; i < maxDepth; i++)
    {
        bottom->v = make_shared<Recursive>();
        bottom = bottom->v;
    }
    initial->setRecursive(top);

    // Adding one more level would exceed the max class graph depth
    bottom->v = make_shared<Recursive>();
    bottom = bottom->v;

    try
    {
        initial->setRecursive(top);
        test(false);
    }
    catch (const Ice::UnknownLocalException&)
    {
        // Expected marshal exception from the server (max class graph depth reached)
    }
    cout << "ok" << endl;

    cout << "testing compact ID..." << flush;
    try
    {
        test(initial->getCompact());
    }
    catch (const Ice::OperationNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing marshaled results..." << flush;
    b1 = initial->getMB();
    test(b1 && b1->theB == b1);
    clear(b1);
    b1 = initial->getAMDMBAsync().get();
    test(b1 && b1->theB == b1);
    clear(b1);
    cout << "ok" << endl;

    cout << "testing UnexpectedObjectException... " << flush;
    UnexpectedObjectExceptionTestPrx uoet(
        communicator,
        "uoet:" + TestHelper::getTestEndpoint(communicator->getProperties()));

    try
    {
        uoet->op();
        test(false);
    }
    catch (const Ice::MarshalException& ex)
    {
        string what = ex.what();
        test(what.find("::Test::AlsoEmpty") != string::npos);
        test(what.find("::Test::Empty") != string::npos);
    }
    catch (const Ice::Exception& ex)
    {
        cout << ex << endl;
        test(false);
    }
    catch (...)
    {
        test(false);
    }
    cout << "ok" << endl;

    try
    {
        Ice::PropertiesPtr properties = communicator->getProperties();
        TestIntfPrx p(communicator, "test:" + TestHelper::getTestEndpoint(properties));

        cout << "testing Object factory registration... " << flush;
        {
            BasePtr basePtr = p->opDerived();
            test(basePtr);
            test(string{basePtr->ice_id()} == "::Test::Derived");
        }
        cout << "ok" << endl;

        cout << "testing Exception factory registration... " << flush;
        {
            try
            {
                p->throwDerived();
            }
            catch (const BaseEx& ex)
            {
                test(string{ex.ice_id()} == "::Test::DerivedEx");
            }
        }
        cout << "ok" << endl;
    }
    catch (const Ice::ObjectNotExistException&)
    {
    }

    cout << "testing class containing complex dictionary... " << flush;
    {
        Test::MPtr m = make_shared<Test::M>();

        Test::StructKey k1;
        k1.i = 1;
        k1.s = "1";
        m->v[k1] = make_shared<L>("one");

        Test::StructKey k2;
        k2.i = 2;
        k2.s = "2";
        m->v[k2] = make_shared<L>("two");

        Test::MPtr m1;
        Test::MPtr m2 = initial->opM(m, m1);

        test(m1->v.size() == 2);
        test(m2->v.size() == 2);

        test(m1->v[k1]->data == "one");
        test(m2->v[k1]->data == "one");

        test(m1->v[k2]->data == "two");
        test(m2->v[k2]->data == "two");
    }
    cout << "ok" << endl;

    cout << "testing forward declarations... " << flush;
    {
        F1Ptr f12;
        F1Ptr f11 = initial->opF1(make_shared<F1>("F11"), f12);
        test(f11->name == "F11");
        test(f12->name == "F12");

        optional<F2Prx> f22;
        optional<F2Prx> f21 = initial->opF2(F2Prx(communicator, "F21:" + helper->getTestEndpoint()), f22);
        test(f21->ice_getIdentity().name == "F21");
        f21->op();
        test(f22->ice_getIdentity().name == "F22");

        if (initial->hasF3())
        {
            F3Ptr f32;
            F3Ptr f31 = initial->opF3(make_shared<F3>(f11, f21), f32);
            test(f31->f1->name == "F11");
            test(f31->f2->ice_getIdentity().name == "F21");

            test(f32->f1->name == "F12");
            test(f32->f2->ice_getIdentity().name == "F22");
        }
    }
    cout << "ok" << endl;

    cout << "testing sending class cycle... " << flush;
    {
        RecursivePtr rec = make_shared<Recursive>();
        rec->v = rec;
        bool acceptsCycles = initial->acceptsClassCycles();
        try
        {
            initial->setCycle(rec);
            test(acceptsCycles);
        }
        catch (const Ice::UnknownLocalException&)
        {
            // expected when the remote server does not accept cycles
            // and throws a MarshalException
            test(!acceptsCycles);
        }
        rec->v = nullptr;
    }
    cout << "ok" << endl;

    return initial;
}
