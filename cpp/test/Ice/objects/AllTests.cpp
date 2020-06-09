//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

// For 'Ice::Communicator::addObjectFactory()' deprecation
#if defined(_MSC_VER)
#   pragma warning( disable : 4996 )
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;
using namespace Test;

namespace
{

void
testUOE(const Ice::CommunicatorPtr& communicator)
{
    string ref = "uoet:" + TestHelper::getTestEndpoint(communicator->getProperties());
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);
    UnexpectedObjectExceptionTestPrxPtr uoet = ICE_UNCHECKED_CAST(UnexpectedObjectExceptionTestPrx, base);
    test(uoet);
    try
    {
        uoet->op();
        test(false);
    }
    catch(const Ice::UnexpectedObjectException& ex)
    {
        test(ex.type == "::Test::AlsoEmpty");
        test(ex.expectedType == "::Test::Empty");
    }
    catch(const Ice::Exception& ex)
    {
        cout << ex << endl;
        test(false);
    }
    catch(...)
    {
        test(false);
    }
}

void clear(const CPtr&);

void
clear(const BPtr& b)
{
    if(dynamic_pointer_cast<B>(b->theA))
    {
        auto tmp = b->theA;
        b->theA = nullptr;
        clear(dynamic_pointer_cast<B>(tmp));
    }
    if(b->theB)
    {
        auto tmp = b->theB;
        b->theB = nullptr;
        clear(dynamic_pointer_cast<B>(tmp));
    }
    b->theC = nullptr;
}

void
clear(const CPtr& c)
{
    clear(c->theB);
    c->theB = nullptr;
}

void
clear(const DPtr& d)
{
    if(dynamic_pointer_cast<B>(d->theA))
    {
        clear(dynamic_pointer_cast<B>(d->theA));
    }
    d->theA = nullptr;
    clear(d->theB);
    d->theB = nullptr;
}

}

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

    cout << "testing constructor, copy constructor, and assignment operator... " << flush;

    BasePtr ba1 = std::make_shared<Base>();
    test(ba1->theS.str == "");
    test(ba1->str == "");

    S s;
    s.str = "hello";
    BasePtr ba2 = std::make_shared<Base>(s, "hi");
    test(ba2->theS.str == "hello");
    test(ba2->str == "hi");

    test(*ba1 < *ba2);
    test(*ba2 > *ba1);
    test(*ba1 != *ba2);

    *ba1 = *ba2;
    test(ba1->theS.str == "hello");
    test(ba1->str == "hi");

    test(*ba1 == *ba2);
    test(*ba1 >= *ba2);
    test(*ba1 <= *ba2);

    BasePtr bp1 = std::make_shared<Base>();
    *bp1 = *ba2;
    test(bp1->theS.str == "hello");
    test(bp1->str == "hi");

    cout << "ok" << endl;

    cout << "testing ice_clone..." << flush;

    BasePtr bp2 = ICE_DYNAMIC_CAST(Base, bp1->ice_clone());
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
    test(ICE_DYNAMIC_CAST(B, b1->theA));
    test(ICE_DYNAMIC_CAST(B, b1->theA)->theA == b1->theA);
    test(ICE_DYNAMIC_CAST(B, b1->theA)->theB == b1);
    test(ICE_DYNAMIC_CAST(C, ICE_DYNAMIC_CAST(B, b1->theA)->theC));
    test(ICE_DYNAMIC_CAST(C, ICE_DYNAMIC_CAST(B, b1->theA)->theC)->theB == b1->theA);

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

    cout << "testing protected members... " << flush;

    EIPtr e = ICE_DYNAMIC_CAST(EI, initial->getE());
    FIPtr f = ICE_DYNAMIC_CAST(FI, initial->getF());
    cout << "ok" << endl;

    cout << "getting K... " << flush;
    {
        KPtr k = initial->getK();
        LPtr l = ICE_DYNAMIC_CAST(L, k->value);
        test(l);
        test(l->data == "l");
    }
    cout << "ok" << endl;

    cout << "testing Value as parameter..." << flush;
    {
        LPtr v1 = std::make_shared<L>("l");
        Ice::ValuePtr v2;
        Ice::ValuePtr v3 = initial->opClass(v1, v2);
        test(ICE_DYNAMIC_CAST(L, v2)->data == "l");
        test(ICE_DYNAMIC_CAST(L, v3)->data == "l");
    }

    {
        LPtr l = std::make_shared<L>("l");
        Test::ClassSeq v1;
        v1.push_back(l);
        Test::ClassSeq v2;
        Test::ClassSeq v3 = initial->opClassSeq(v1, v2);
        test(ICE_DYNAMIC_CAST(L, v2[0])->data == "l");
        test(ICE_DYNAMIC_CAST(L, v3[0])->data == "l");
    }

    {
        LPtr l = std::make_shared<L>("l");
        Test::ClassMap v1;
        v1["l"] = l;
        Test::ClassMap v2;
        Test::ClassMap v3 = initial->opClassMap(v1, v2);
        test(ICE_DYNAMIC_CAST(L, v2["l"])->data == "l");
        test(ICE_DYNAMIC_CAST(L, v3["l"])->data == "l");
    }
    cout << "ok" << endl;

    cout << "getting D1... " << flush;
    D1Ptr d1 = std::make_shared<D1>(
                               std::make_shared<A1>("a1"),
                               std::make_shared<A1>("a2"),
                               std::make_shared<A1>("a3"),
                               std::make_shared<A1>("a4"));
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
    catch(const EDerived& ederived)
    {
        test(ederived.a1->name == "a1");
        test(ederived.a2->name == "a2");
        test(ederived.a3->name == "a3");
        test(ederived.a4->name == "a4");
    }
    cout << "ok" << endl;

    cout << "setting G... " << flush;
    GPtr g = std::make_shared<G>(s, "g");
    try
    {
        initial->setG(g);
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing sequences... " << flush;
    BaseSeq inS, outS, retS;
    retS = initial->opBaseSeq(inS, outS);

    inS.resize(1);
    inS[0] = std::make_shared<Base>();
    retS = initial->opBaseSeq(inS, outS);
    test(retS.size() == 1 && outS.size() == 1);
    cout << "ok" << endl;

    cout << "testing recursive type... " << flush;
    RecursivePtr top = std::make_shared<Recursive>();
    int depth = 0;
    try
    {
        RecursivePtr p = top;
#if defined(NDEBUG) || !defined(__APPLE__)
        const int maxDepth = 2000;
#else
        // With debug, marshalling a graph of 2000 elements can cause a stack overflow on macOS
        const int maxDepth = 1500;
#endif
        for(; depth <= maxDepth; ++depth)
        {
            p->v = std::make_shared<Recursive>();
            p = p->v;
            if((depth < 10 && (depth % 10) == 0) ||
               (depth < 1000 && (depth % 100) == 0) ||
               (depth < 10000 && (depth % 1000) == 0) ||
               (depth % 10000) == 0)
            {
                initial->setRecursive(top);
            }
        }
        test(!initial->supportsClassGraphDepthMax());
    }
    catch(const Ice::UnknownLocalException&)
    {
        // Expected marshal exception from the server (max class graph depth reached)
        test(depth == 100); // The default is 100.
    }
    catch(const Ice::UnknownException&)
    {
        // Expected stack overflow from the server (Java only)
    }
    initial->setRecursive(std::make_shared<Recursive>());
    cout << "ok" << endl;

    cout << "testing compact ID..." << flush;
    try
    {
        test(initial->getCompact());
    }
    catch(const Ice::OperationNotExistException&)
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
    testUOE(communicator);
    cout << "ok" << endl;

    cout << "testing getting ObjectFactory... " << flush;
    test(communicator->findObjectFactory("TestOF"));
    cout << "ok" << endl;
    cout << "testing getting ObjectFactory as ValueFactory... " << flush;
    test(communicator->getValueFactoryManager()->find("TestOF"));
    cout << "ok" << endl;

    try
    {
        Ice::PropertiesPtr properties = communicator->getProperties();
        TestIntfPrxPtr p =
            ICE_CHECKED_CAST(TestIntfPrx,
                             communicator->stringToProxy("test:" + TestHelper::getTestEndpoint(properties)));

        cout << "testing Object factory registration... " << flush;
        {
            BasePtr basePtr = p->opDerived();
            test(basePtr);
            test(basePtr->ice_id() == "::Test::Derived");
        }
        cout << "ok" << endl;

        cout << "testing Exception factory registration... " << flush;
        {
            try
            {
                p->throwDerived();
            }
            catch(const BaseEx& ex)
            {
                test(ex.ice_id() == "::Test::DerivedEx");
            }
        }
        cout << "ok" << endl;
    }
    catch(const Ice::ObjectNotExistException&)
    {
    }

    cout << "testing class containing complex dictionary... " << flush;
    {
        Test::MPtr m = std::make_shared<Test::M>();

        Test::StructKey k1;
        k1.i = 1;
        k1.s = "1";
        m->v[k1] = std::make_shared<L>("one");

        Test::StructKey k2;
        k2.i = 2;
        k2.s = "2";
        m->v[k2] = std::make_shared<L>("two");

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
        F1Ptr f11 = initial->opF1(std::make_shared<F1>("F11"), f12);
        test(f11->name == "F11");
        test(f12->name == "F12");

        F2PrxPtr f22;
        F2PrxPtr f21 = initial->opF2(ICE_UNCHECKED_CAST(F2Prx,
            communicator->stringToProxy("F21:" + helper->getTestEndpoint())), f22);
        test(f21->ice_getIdentity().name == "F21");
        f21->op();
        test(f22->ice_getIdentity().name == "F22");

        if(initial->hasF3())
        {
            F3Ptr f32;
            F3Ptr f31 = initial->opF3(std::make_shared<F3>(f11, f21), f32);
            test(f31->f1->name == "F11");
            test(f31->f2->ice_getIdentity().name == "F21");

            test(f32->f1->name == "F12");
            test(f32->f2->ice_getIdentity().name == "F22");
        }
    }
    cout << "ok" << endl;

    return initial;
}
