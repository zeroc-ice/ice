// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <TestI.h>

#ifdef _MSC_VER
// For 'Ice::Communicator::addObjectFactory()' deprecation
#pragma warning( disable : 4996 )
#endif

#if defined(__GNUC__)
// For 'Ice::Communicator::addObjectFactory()' deprecation
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;
using namespace Test;

namespace
{

class AbstractBaseI : public AbstractBase
{
public:

    virtual void op(const Ice::Current&)
    {
    }
};

void
testUOE(const Ice::CommunicatorPtr& communicator)
{
    string ref = "uoet:" + getTestEndpoint(communicator, 0);
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
#ifdef ICE_CPP11_MAPPING
    // No GC with the C++11 mapping
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
#endif
}

void
clear(const CPtr& c)
{
#ifdef ICE_CPP11_MAPPING
    // No GC with the C++11 mapping
    clear(c->theB);
    c->theB = nullptr;
#endif
}

void
clear(const DPtr& d)
{
#ifdef ICE_CPP11_MAPPING
    // No GC with the C++11 mapping
    if(dynamic_pointer_cast<B>(d->theA))
    {
        clear(dynamic_pointer_cast<B>(d->theA));
    }
    d->theA = nullptr;
    clear(d->theB);
    d->theB = nullptr;
#endif
}

}

InitialPrxPtr
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "initial:" + getTestEndpoint(communicator, 0);
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

    cout << "testing constructor, copy constructor, and assignment operator... " << flush;

    BasePtr ba1 = ICE_MAKE_SHARED(Base);
    test(ba1->theS.str == "");
    test(ba1->str == "");

    S s;
    s.str = "hello";
    BasePtr ba2 = ICE_MAKE_SHARED(Base, s, "hi");
    test(ba2->theS.str == "hello");
    test(ba2->str == "hi");

#ifdef ICE_CPP11_MAPPING
    test(*ba1 < *ba2);
    test(*ba2 > *ba1);
    test(*ba1 != *ba2);
#endif

    *ba1 = *ba2;
    test(ba1->theS.str == "hello");
    test(ba1->str == "hi");

#ifdef ICE_CPP11_MAPPING
    test(*ba1 == *ba2);
    test(*ba1 >= *ba2);
    test(*ba1 <= *ba2);
#endif

    BasePtr bp1 = ICE_MAKE_SHARED(Base);
    *bp1 = *ba2;
    test(bp1->theS.str == "hello");
    test(bp1->str == "hi");

    cout << "ok" << endl;

    cout << "testing ice_clone..." << flush;

    BasePtr bp2 = ICE_DYNAMIC_CAST(Base, bp1->ice_clone());
    test(bp1->theS.str == bp2->theS.str);
    test(bp1->str == bp2->str);

#ifndef ICE_CPP11_MAPPING
    //
    // With C++11 mapping value classes are never abstracts.
    //
    AbstractBasePtr abp1 = new AbstractBaseI();
    try
    {
        abp1->ice_clone();
        test(false);
    }
    catch(const Ice::CloneNotImplementedException&)
    {
    }
#endif
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
#ifdef ICE_CPP11_MAPPING
    test(b1 != dynamic_pointer_cast<B>(c));
    test(b1 != dynamic_pointer_cast<B>(d));
    test(b2 != dynamic_pointer_cast<B>(c));
    test(b2 != dynamic_pointer_cast<B>(d));
    test(c != dynamic_pointer_cast<C>(d));
#else
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
#endif
    test(b1->theB == b1);
    test(b1->theC == ICE_NULLPTR);
    test(ICE_DYNAMIC_CAST(B, b1->theA));
    test(ICE_DYNAMIC_CAST(B, b1->theA)->theA == b1->theA);
    test(ICE_DYNAMIC_CAST(B, b1->theA)->theB == b1);
    test(ICE_DYNAMIC_CAST(C, ICE_DYNAMIC_CAST(B, b1->theA)->theC));
    test(ICE_DYNAMIC_CAST(C, ICE_DYNAMIC_CAST(B, b1->theA)->theC)->theB == b1->theA);

    test(b1->preMarshalInvoked);
    test(b1->postUnmarshalInvoked);
    test(b1->theA->preMarshalInvoked);
    test(b1->theA->postUnmarshalInvoked);
#ifdef ICE_CPP11_MAPPING
    test(dynamic_pointer_cast<B>(b1->theA)->theC->preMarshalInvoked);
    test(dynamic_pointer_cast<B>(b1->theA)->theC->postUnmarshalInvoked);
#else
    test(BPtr::dynamicCast(b1->theA)->theC->preMarshalInvoked);
    test(BPtr::dynamicCast(b1->theA)->theC->postUnmarshalInvoked);
#endif
    // More tests possible for b2 and d, but I think this is already sufficient.
    test(b2->theA == b2);
    test(d->theC == ICE_NULLPTR);

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
#ifdef ICE_CPP11_MAPPING
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
#else
    test(b1 != b2);
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
    test(b1->theA == b2);
    test(b1->theB == b1);
    test(b1->theC == ICE_NULLPTR);
    test(b2->theA == b2);
    test(b2->theB == b1);
    test(b2->theC == c);
    test(c->theB == b2);
    test(d->theA == b1);
    test(d->theB == b2);
    test(d->theC == ICE_NULLPTR);
#endif
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
#ifndef ICE_CPP11_MAPPING
    test(e->checkValues());
    test(f->checkValues());
    test(ICE_DYNAMIC_CAST(EI, f->e2)->checkValues());
#endif
    cout << "ok" << endl;

    cout << "getting I, J and H... " << flush;
#ifdef ICE_CPP11_MAPPING
    shared_ptr<Ice::Value> i = initial->getI();
    test(i->ice_id() == "::Test::I");
    shared_ptr<Ice::Value> j = initial->getJ();
    test(j->ice_id() == "::Test::J");
    shared_ptr<Ice::Value> h = initial->getH();
    test(h && dynamic_pointer_cast<H>(h));
#else
    IPtr i = initial->getI();
    test(i);
    IPtr j = initial->getJ();
    test(j && JPtr::dynamicCast(j));
    IPtr h = initial->getH();
    test(h && HPtr::dynamicCast(h));
#endif
    cout << "ok" << endl;

    cout << "getting D1... " << flush;
    D1Ptr d1 = ICE_MAKE_SHARED(D1,
                               ICE_MAKE_SHARED(A1, "a1"),
                               ICE_MAKE_SHARED(A1, "a2"),
                               ICE_MAKE_SHARED(A1, "a3"),
                               ICE_MAKE_SHARED(A1, "a4"));
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

    cout << "setting I... " << flush;
    initial->setI(i);
    initial->setI(j);
    initial->setI(h);
    cout << "ok" << endl;

    cout << "testing sequences... " << flush;
    BaseSeq inS, outS, retS;
    retS = initial->opBaseSeq(inS, outS);

    inS.resize(1);
    inS[0] = ICE_MAKE_SHARED(Base);
    retS = initial->opBaseSeq(inS, outS);
    test(retS.size() == 1 && outS.size() == 1);
    cout << "ok" << endl;

    cout << "testing recursive type... " << flush;
    RecursivePtr top = ICE_MAKE_SHARED(Recursive);
    RecursivePtr p = top;
    int depth = 0;
    try
    {
#if defined(NDEBUG) || !defined(__APPLE__)
        const int maxDepth = 2000;
#else
        // With debug, marshalling a graph of 2000 elements can cause a stack overflow on macOS
        const int maxDepth = 1500;
#endif
        for(; depth <= maxDepth; ++depth)
        {
            p->v = ICE_MAKE_SHARED(Recursive);
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
    initial->setRecursive(ICE_MAKE_SHARED(Recursive));
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
#ifdef ICE_CPP11_MAPPING
    b1 = initial->getAMDMBAsync().get();
#else
    b1 = initial->end_getAMDMB(initial->begin_getAMDMB());
#endif
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
        TestIntfPrxPtr p = ICE_CHECKED_CAST(TestIntfPrx,
                                            communicator->stringToProxy("test:" + getTestEndpoint(communicator, 0)));

        cout << "testing Object factory registration... " << flush;
        {
            BasePtr base = p->opDerived();
            test(base);
            test(base->ice_id() == "::Test::Derived");
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

    return initial;
}
