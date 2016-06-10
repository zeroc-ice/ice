// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <ClientPrivate.h>

using namespace std;
using namespace Test;

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};

class Callback : public CallbackBase, public Ice::EnableSharedFromThis<Callback>
{
public:

    void
    response_SBaseAsObject(const ::Ice::ObjectPtr& o)
    {
        test(o);
        test(o->ice_id() == "::Test::SBase");
        SBasePtr sb = ICE_DYNAMIC_CAST(SBase, o);
        test(sb);
        test(sb->sb == "SBase.sb");
        called();
    }

    void
    response_SBaseAsSBase(const SBasePtr& sb)
    {
        test(sb->sb == "SBase.sb");
        called();
    }

    void
    response_SBSKnownDerivedAsSBase(const SBasePtr& sb)
    {
        SBSKnownDerivedPtr sbskd = ICE_DYNAMIC_CAST(SBSKnownDerived, sb);
        test(sbskd);
        test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        called();
    }

    void
    response_SBSKnownDerivedAsSBSKnownDerived(const SBSKnownDerivedPtr& sbskd)
    {
        test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        called();
    }

    void
    response_SBSUnknownDerivedAsSBase(const SBasePtr& sb)
    {
        test(sb->sb == "SBSUnknownDerived.sb");
        called();
    }

    void
    response_SBSUnknownDerivedAsSBaseCompact(const SBasePtr&)
    {
        test(false);
    }

    void
    exception_SBSUnknownDerivedAsSBaseCompact(const Ice::Exception& exc)
    {
        test(exc.ice_id() == "::Ice::NoValueFactoryException");
        called();
    }

    void
    response_SUnknownAsObject10(const Ice::ObjectPtr&)
    {
        test(false);
    }

    void
    exception_SUnknownAsObject10(const Ice::Exception& exc)
    {
        test(exc.ice_id() == "::Ice::NoValueFactoryException");
        called();
    }

    void
    response_SUnknownAsObject11(const Ice::ObjectPtr& o)
    {
        test(ICE_DYNAMIC_CAST(Ice::UnknownSlicedValue, o));
        test(ICE_DYNAMIC_CAST(Ice::UnknownSlicedValue, o)->getUnknownTypeId() == "::Test::SUnknown");
        called();
    }

    void
    exception_SUnknownAsObject11(const Ice::Exception&)
    {
        test(false);
    }

    void
    response_oneElementCycle(const BPtr& b)
    {
        test(b);
        test(b->ice_id() == "::Test::B");
        test(b->sb == "B1.sb");
        test(b->pb == b);
        called();
    }

    void
    response_twoElementCycle(const BPtr& b1)
    {
        test(b1);
        test(b1->ice_id() == "::Test::B");
        test(b1->sb == "B1.sb");

        BPtr b2 = b1->pb;
        test(b2);
        test(b2->ice_id() == "::Test::B");
        test(b2->sb == "B2.sb");
        test(b2->pb == b1);
        called();
    }

    void
    response_D1AsB(const BPtr& b1)
    {
        test(b1);
        test(b1->ice_id() == "::Test::D1");
        test(b1->sb == "D1.sb");
        test(b1->pb);
        test(b1->pb != b1);
        D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
        test(d1);
        test(d1->sd1 == "D1.sd1");
        test(d1->pd1);
        test(d1->pd1 != b1);
        test(b1->pb == d1->pd1);

        BPtr b2 = b1->pb;
        test(b2);
        test(b2->pb == b1);
        test(b2->sb == "D2.sb");
        test(b2->ice_id() == "::Test::B");
        called();
    }

    void
    response_D1AsD1(const D1Ptr& d1)
    {
        test(d1);
        test(d1->ice_id() == "::Test::D1");
        test(d1->sb == "D1.sb");
        test(d1->pb);
        test(d1->pb != d1);

        BPtr b2 = d1->pb;
        test(b2);
        test(b2->ice_id() == "::Test::B");
        test(b2->sb == "D2.sb");
        test(b2->pb == d1);
        called();
    }

    void
    response_D2AsB(const BPtr& b2)
    {
        test(b2);
        test(b2->ice_id() == "::Test::B");
        test(b2->sb == "D2.sb");
        test(b2->pb);
        test(b2->pb != b2);

        BPtr b1 = b2->pb;
        test(b1);
        test(b1->ice_id() == "::Test::D1");
        test(b1->sb == "D1.sb");
        test(b1->pb == b2);
        D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
        test(d1);
        test(d1->sd1 == "D1.sd1");
        test(d1->pd1 == b2);
        called();
    }

    void
    response_paramTest1(const BPtr& b1, const BPtr& b2)
    {
        test(b1);
        test(b1->ice_id() == "::Test::D1");
        test(b1->sb == "D1.sb");
        test(b1->pb == b2);
        D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
        test(d1);
        test(d1->sd1 == "D1.sd1");
        test(d1->pd1 == b2);

        test(b2);
        test(b2->ice_id() == "::Test::B");      // No factory, must be sliced
        test(b2->sb == "D2.sb");
        test(b2->pb == b1);
        called();
    }

    void
    response_returnTest1(const BPtr& r, const BPtr& p1, const BPtr&)
    {
        test(r == p1);
        called();
    }

    void
    response_returnTest2(const BPtr& r, const BPtr& p1, const BPtr&)
    {
        test(r == p1);
        called();
    }

    void
    response_returnTest3(const BPtr& b)
    {
        rb = b;
        called();
    }

    void
    response_paramTest3(const BPtr& ret, const BPtr& p1, const BPtr& p2)
    {
        test(p1);
        test(p1->sb == "D2.sb (p1 1)");
        test(p1->pb == 0);
        test(p1->ice_id() == "::Test::B");

        test(p2);
        test(p2->sb == "D2.sb (p2 1)");
        test(p2->pb == 0);
        test(p2->ice_id() == "::Test::B");

        test(ret);
        test(ret->sb == "D1.sb (p2 2)");
        test(ret->pb == 0);
        test(ret->ice_id() == "::Test::D1");
        called();
    }

    void
    response_paramTest4(const BPtr& ret, const BPtr& b)
    {
        test(b);
        test(b->sb == "D4.sb (1)");
        test(b->pb == 0);
        test(b->ice_id() == "::Test::B");

        test(ret);
        test(ret->sb == "B.sb (2)");
        test(ret->pb == 0);
        test(ret->ice_id() == "::Test::B");
        called();
    }

    void
    response_sequenceTest(const SS3& ss)
    {
        rss3 = ss;
        called();
    }

    void
    response_dictionaryTest(const BDict& r, const BDict& bout)
    {
        rbdict = r;
        obdict = bout;
        called();
    }

    void
    exception_throwBaseAsBase(const ::Ice::Exception& ex)
    {
        test(ex.ice_id() == "::Test::BaseException");
        const BaseException& e = dynamic_cast<const BaseException&>(ex);
        test(e.sbe == "sbe");
        test(e.pb);
        test(e.pb->sb == "sb");
        test(e.pb->pb == e.pb);
        called();
    }

    void
    exception_throwDerivedAsBase(const ::Ice::Exception& ex)
    {
        test(ex.ice_id() == "::Test::DerivedException");
        const DerivedException& e = dynamic_cast<const DerivedException&>(ex);
        test(e.sbe == "sbe");
        test(e.pb);
        test(e.pb->sb == "sb1");
        test(e.pb->pb == e.pb);
        test(e.sde == "sde1");
        test(e.pd1);
        test(e.pd1->sb == "sb2");
        test(e.pd1->pb == e.pd1);
        test(e.pd1->sd1 == "sd2");
        test(e.pd1->pd1 == e.pd1);
        called();
    }

    void
    exception_throwDerivedAsDerived(const ::Ice::Exception& ex)
    {
        test(ex.ice_id() == "::Test::DerivedException");
        const DerivedException& e = dynamic_cast<const DerivedException&>(ex);
        test(e.sbe == "sbe");
        test(e.pb);
        test(e.pb->sb == "sb1");
        test(e.pb->pb == e.pb);
        test(e.sde == "sde1");
        test(e.pd1);
        test(e.pd1->sb == "sb2");
        test(e.pd1->pb == e.pd1);
        test(e.pd1->sd1 == "sd2");
        test(e.pd1->pd1 == e.pd1);
        called();
    }


    void
    exception_throwUnknownDerivedAsBase(const ::Ice::Exception& ex)
    {
        test(ex.ice_id() == "::Test::BaseException");
        const BaseException& e = dynamic_cast<const BaseException&>(ex);
        test(e.sbe == "sbe");
        test(e.pb);
        test(e.pb->sb == "sb d2");
        test(e.pb->pb == e.pb);
        called();
    }

    void
    response_preserved1(const PBasePtr& r)
    {
        PDerivedPtr pd = ICE_DYNAMIC_CAST(PDerived, r);
        test(pd);
        test(pd->pi == 3);
        test(pd->ps == "preserved");
        test(pd->pb == pd);
        called();
    }

    void
    response_preserved2(const PBasePtr& r)
    {
        PCUnknownPtr p2 = ICE_DYNAMIC_CAST(PCUnknown, r);
        test(!p2);
        test(r->pi == 3);
        called();
    }

    void
    response_preserved3(const PBasePtr& r)
    {
        //
        // Encoding 1.0
        //
        PCDerivedPtr p2 = ICE_DYNAMIC_CAST(PCDerived, r);
        test(!p2);
        test(r->pi == 3);
        called();
    }

    void
    response_preserved4(const PBasePtr& r)
    {
        //
        // Encoding > 1.0
        //
        PCDerivedPtr p2 = ICE_DYNAMIC_CAST(PCDerived, r);
        test(p2);
        test(p2->pi == 3);
        test(p2->pbs[0] == p2);
        called();
    }

    void
    response_preserved5(const PBasePtr& r)
    {
        PCDerived3Ptr p3 = ICE_DYNAMIC_CAST(PCDerived3, r);
        test(p3);
        test(p3->pi == 3);
        for(int i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = ICE_DYNAMIC_CAST(PCDerived2, p3->pbs[i]);
            test(p2->pi == i);
            test(p2->pbs.size() == 1);
            test(!p2->pbs[0]);
            test(p2->pcd2 == i);
        }
        test(p3->pcd2 == p3->pi);
        test(p3->pcd3 == p3->pbs[10]);
        called();
    }

    void
    response_compactPreserved1(const PBasePtr& r)
    {
        //
        // Encoding 1.0
        //
        CompactPCDerivedPtr p2 = ICE_DYNAMIC_CAST(CompactPCDerived, r);
        test(!p2);
        test(r->pi == 3);
        called();
    }

    void
    response_compactPreserved2(const PBasePtr& r)
    {
        //
        // Encoding > 1.0
        //
        CompactPCDerivedPtr p2 = ICE_DYNAMIC_CAST(CompactPCDerived, r);
        test(p2);
        test(p2->pi == 3);
        test(p2->pbs[0] == p2);
        called();
    }

    void
    response()
    {
        test(false);
    }

    void
    exception(const ::Ice::Exception& ex)
    {
        if(!dynamic_cast<const Ice::OperationNotExistException*>(&ex))
        {
            test(false);
        }
        else
        {
            called();
        }
    }

    BPtr rb;
    SS3 rss3;
    BDict rbdict;
    BDict obdict;
};
ICE_DEFINE_PTR(CallbackPtr, Callback);

class PNodeI : public virtual PNode
{
public:

    PNodeI()
    {
        ++counter;
    }

    virtual ~PNodeI()
    {
        --counter;
    }

    static int counter;
};

int PNodeI::counter = 0;

#ifndef ICE_CPP11_MAPPING
class NodeFactoryI : public Ice::ValueFactory
{
public:

    virtual Ice::ObjectPtr create(const string& id)
    {
        if(id == PNode::ice_staticId())
        {
            return new PNodeI;
        }
        return 0;
    }

    virtual void destroy()
    {
    }
};
#endif

class PreservedI : public virtual Preserved
{
public:

    PreservedI()
    {
        ++counter;
    }

    virtual ~PreservedI()
    {
        --counter;
    }

    static int counter;
};

int PreservedI::counter = 0;

#ifndef ICE_CPP11_MAPPING
class PreservedFactoryI : public Ice::ValueFactory
{
public:

    virtual Ice::ObjectPtr create(const string& id)
    {
        if(id == Preserved::ice_staticId())
        {
            return new PreservedI;
        }
        return 0;
    }

    virtual void destroy()
    {
    }
};
#endif

void
testUOO(const TestIntfPrxPtr& test)
{
    Ice::ValuePtr o;
    try
    {
        o = test->SUnknownAsObject();
        test(test->ice_getEncodingVersion() != Ice::Encoding_1_0);
        test(ICE_DYNAMIC_CAST(Ice::UnknownSlicedValue, o));
        test(ICE_DYNAMIC_CAST(Ice::UnknownSlicedValue, o)->getUnknownTypeId() == "::Test::SUnknown");
        test->checkSUnknown(o);
    }
    catch(const Ice::NoValueFactoryException&)
    {
        test(test->ice_getEncodingVersion() == Ice::Encoding_1_0);
    }
    catch(const std::exception& ex)
    {
        cout << ex.what() << endl;
        test(false);
    }
    catch(...)
    {
        test(false);
    }
}

TestIntfPrxPtr
allTests(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectPrxPtr obj = communicator->stringToProxy("Test:" + getTestEndpoint(communicator, 0));
    TestIntfPrxPtr test = ICE_CHECKED_CAST(TestIntfPrx, obj);

    cout << "base as Object... " << flush;
    {
        Ice::ValuePtr o;
        try
        {
            o = test->SBaseAsObject();
            test(o);
            test(o->ice_id() == "::Test::SBase");
        }
        catch(const std::exception& ex)
        {
            cerr << ex.what() << endl;
            test(false);
        }
        catch(...)
        {
            test(false);
        }
        SBasePtr sb = ICE_DYNAMIC_CAST(SBase, o);
        test(sb);
        test(sb->sb == "SBase.sb");
    }
    cout << "ok" << endl;

    cout << "base as Object (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->SBaseAsObjectAsync();
        try
        {
            auto o = f.get();
            test(o);
            test(o->ice_id() == "::Test::SBase");
            auto sb = dynamic_pointer_cast<SBase>(o);
            test(sb);
            test(sb->sb == "SBase.sb");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback();
        test->begin_SBaseAsObject(
            newCallback_TestIntf_SBaseAsObject(cb, &Callback::response_SBaseAsObject, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "base as base... " << flush;
    {
        SBasePtr sb;
        try
        {
            sb = test->SBaseAsSBase();
            test(sb->sb == "SBase.sb");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        try
        {
            auto sb = test->SBaseAsSBaseAsync().get();
            test(sb->sb == "SBase.sb");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_SBaseAsSBase(
            newCallback_TestIntf_SBaseAsSBase(cb, &Callback::response_SBaseAsSBase, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "base with known derived as base... " << flush;
    {
        SBasePtr sb;
        try
        {
            sb = test->SBSKnownDerivedAsSBase();
            test(sb->sb == "SBSKnownDerived.sb");
        }
        catch(...)
        {
            test(false);
        }
        SBSKnownDerivedPtr sbskd = ICE_DYNAMIC_CAST(SBSKnownDerived, sb);
        test(sbskd);
        test(sbskd->sbskd == "SBSKnownDerived.sbskd");
    }
    cout << "ok" << endl;

    cout << "base with known derived as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->SBSKnownDerivedAsSBaseAsync();
        try
        {
            shared_ptr<SBase> sb = f.get();
            sb = test->SBSKnownDerivedAsSBase();
            test(sb->sb == "SBSKnownDerived.sb");
            SBSKnownDerivedPtr sbskd = ICE_DYNAMIC_CAST(SBSKnownDerived, sb);
            test(sbskd);
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_SBSKnownDerivedAsSBase(
            newCallback_TestIntf_SBSKnownDerivedAsSBase(
                cb, &Callback::response_SBSKnownDerivedAsSBase, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "base with known derived as known derived... " << flush;
    {
        SBSKnownDerivedPtr sbskd;
        try
        {
            sbskd = test->SBSKnownDerivedAsSBSKnownDerived();
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base with known derived as known derived (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->SBSKnownDerivedAsSBSKnownDerivedAsync();
        try
        {
            SBSKnownDerivedPtr sbskd = test->SBSKnownDerivedAsSBSKnownDerived();
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_SBSKnownDerivedAsSBSKnownDerived(
            newCallback_TestIntf_SBSKnownDerivedAsSBSKnownDerived(
                cb, &Callback::response_SBSKnownDerivedAsSBSKnownDerived, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "base with unknown derived as base... " << flush;
    {
        SBasePtr sb;
        try
        {
            sb = test->SBSUnknownDerivedAsSBase();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch(...)
        {
            test(false);
        }
    }
    if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
    {
        try
        {
            //
            // This test succeeds for the 1.0 encoding.
            //
            SBasePtr sb = test->SBSUnknownDerivedAsSBaseCompact();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }
    else
    {
        try
        {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            SBasePtr sb = test->SBSUnknownDerivedAsSBaseCompact();
            test(false);
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(const Ice::NoValueFactoryException&)
        {
            // Expected.
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base with unknown derived as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->SBSUnknownDerivedAsSBaseAsync();
        try
        {
            SBasePtr sb = f.get();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_SBSUnknownDerivedAsSBase(
            newCallback_TestIntf_SBSUnknownDerivedAsSBase(
                cb, &Callback::response_SBSUnknownDerivedAsSBase, &Callback::exception));
        cb->check();
#endif
    }
    if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
    {
        //
        // This test succeeds for the 1.0 encoding.
        //
#ifdef ICE_CPP11_MAPPING
        auto f = test->SBSUnknownDerivedAsSBaseCompactAsync();
        try
        {
            SBasePtr sb = f.get();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_SBSUnknownDerivedAsSBaseCompact(
            newCallback_TestIntf_SBSUnknownDerivedAsSBaseCompact(
                cb, &Callback::response_SBSUnknownDerivedAsSBase, &Callback::exception));
        cb->check();
#endif
    }
    else
    {
        //
        // This test fails when using the compact format because the instance cannot
        // be sliced to a known type.
        //
#ifdef ICE_CPP11_MAPPING
        auto f = test->SBSUnknownDerivedAsSBaseCompactAsync();
        try
        {
            f.get();
            test(false);
        }
        catch(const Ice::NoValueFactoryException&)
        {
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_SBSUnknownDerivedAsSBaseCompact(
            newCallback_TestIntf_SBSUnknownDerivedAsSBaseCompact(
                cb, &Callback::response_SBSUnknownDerivedAsSBaseCompact,
                &Callback::exception_SBSUnknownDerivedAsSBaseCompact));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "unknown with Object as Object... " << flush;
    {
        testUOO(test);
    }
    cout << "ok" << endl;

    cout << "unknown with Object as Object (AMI)... " << flush;
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                auto f = test->SUnknownAsObjectAsync();
                try
                {
                    f.get();
                    test(false);
                }
                catch(const Ice::NoValueFactoryException&)
                {
                }
                catch(...)
                {
                    test(false);
                }
            }
            else
            {
                auto f = test->SUnknownAsObjectAsync();
                try
                {
                    shared_ptr<Ice::Value> v = f.get();
                    test(dynamic_pointer_cast<Ice::UnknownSlicedValue>(v));
                    test(dynamic_pointer_cast<Ice::UnknownSlicedValue>(v)->getUnknownTypeId() == "::Test::SUnknown");
                }
                catch(...)
                {
                    test(false);
                }

            }
#else
            CallbackPtr cb = new Callback;
            if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test->begin_SUnknownAsObject(
                    newCallback_TestIntf_SUnknownAsObject(
                        cb, &Callback::response_SUnknownAsObject10, &Callback::exception_SUnknownAsObject10));
            }
            else
            {
                test->begin_SUnknownAsObject(
                    newCallback_TestIntf_SUnknownAsObject(
                        cb, &Callback::response_SUnknownAsObject11, &Callback::exception_SUnknownAsObject11));
            }
            cb->check();
#endif
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "one-element cycle... " << flush;
    {
        try
        {
            BPtr b = test->oneElementCycle();
            test(b);
            test(b->ice_id() == "::Test::B");
            test(b->sb == "B1.sb");
            test(b->pb == b);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "one-element cycle (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->oneElementCycleAsync();
        try
        {
            auto b = f.get();
            test(b);
            test(b->ice_id() == "::Test::B");
            test(b->sb == "B1.sb");
            test(b->pb == b);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_oneElementCycle(
            newCallback_TestIntf_oneElementCycle(
                cb, &Callback::response_oneElementCycle, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "two-element cycle... " << flush;
    {
        try
        {
            BPtr b1 = test->twoElementCycle();
            test(b1);
            test(b1->ice_id() == "::Test::B");
            test(b1->sb == "B1.sb");

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "B2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "two-element cycle (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->twoElementCycleAsync();
        try
        {
            auto b1 = f.get();
            test(b1);
            test(b1->ice_id() == "::Test::B");
            test(b1->sb == "B1.sb");

            auto b2 = b1->pb;
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "B2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_twoElementCycle(
            newCallback_TestIntf_twoElementCycle(
                cb, &Callback::response_twoElementCycle, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as base... " << flush;
    {
        try
        {
            BPtr b1;
            b1 = test->D1AsB();
            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb);
            test(b1->pb != b1);
            D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1);
            test(d1->pd1 != b1);
            test(b1->pb == d1->pd1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->pb == b1);
            test(b2->sb == "D2.sb");
            test(b2->ice_id() == "::Test::B");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->D1AsBAsync();
        try
        {
            auto b1 = f.get();
            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb);
            test(b1->pb != b1);
            auto d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1);
            test(d1->pd1 != b1);
            test(b1->pb == d1->pd1);

            auto b2 = b1->pb;
            test(b2);
            test(b2->pb == b1);
            test(b2->sb == "D2.sb");
            test(b2->ice_id() == "::Test::B");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_D1AsB(newCallback_TestIntf_D1AsB(cb, &Callback::response_D1AsB, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as derived... " << flush;
    {
        try
        {
            D1Ptr d1;
            d1 = test->D1AsD1();
            test(d1);
            test(d1->ice_id() == "::Test::D1");
            test(d1->sb == "D1.sb");
            test(d1->pb);
            test(d1->pb != d1);

            BPtr b2 = d1->pb;
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb == d1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "known derived pointer slicing as derived (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->D1AsD1Async();
        try
        {
            auto d1 = f.get();
            test(d1);
            test(d1->ice_id() == "::Test::D1");
            test(d1->sb == "D1.sb");
            test(d1->pb);
            test(d1->pb != d1);

            auto b2 = d1->pb;
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb == d1);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_D1AsD1(newCallback_TestIntf_D1AsD1(cb, &Callback::response_D1AsD1, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "unknown derived pointer slicing as base... " << flush;
    {
        try
        {
            BPtr b2;
            b2 = test->D2AsB();
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb);
            test(b2->pb != b2);

            BPtr b1 = b2->pb;
            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "unknown derived pointer slicing as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->D2AsBAsync();
        try
        {
            auto b2 = test->D2AsB();
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb);
            test(b2->pb != b2);

            auto b1 = b2->pb;
            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            auto d1 = dynamic_pointer_cast<D1>(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_D2AsB(newCallback_TestIntf_D2AsB(cb, &Callback::response_D2AsB, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "param ptr slicing with known first... " << flush;
    {
        try
        {
            BPtr b1;
            BPtr b2;
            test->paramTest1(b1, b2);

            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(b2->ice_id() == "::Test::B");  // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing with known first (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->paramTest1Async();
        try
        {
            auto result = f.get();
            auto b1 = move(result.p1);
            auto b2 = move(result.p2);

            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(b2->ice_id() == "::Test::B");  // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_paramTest1(
            newCallback_TestIntf_paramTest1(cb, &Callback::response_paramTest1, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "param ptr slicing with unknown first... " << flush;
    {
        try
        {
            BPtr b2;
            BPtr b1;
            test->paramTest2(b2, b1);

            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(b2->ice_id() == "::Test::B");  // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity with known first... " << flush;
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr r = test->returnTest1(p1, p2);
            test(r == p1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity with known first (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->returnTest1Async();
        try
        {
            auto result = f.get();
            test(result.returnValue == result.p1);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_returnTest1(
            newCallback_TestIntf_returnTest1(cb, &Callback::response_returnTest1, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "return value identity with unknown first... " << flush;
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr r = test->returnTest2(p1, p2);
            test(r == p1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity with unknown first (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->returnTest2Async();
        try
        {
            auto result = f.get();
            test(result.returnValue == result.p2);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_returnTest2(
            newCallback_TestIntf_returnTest2(cb, &Callback::response_returnTest2, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "return value identity for input params known first... " << flush;
    {
        try
        {
            D1Ptr d1 = ICE_MAKE_SHARED(D1);
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
#ifndef ICE_CPP11_MAPPING
            d1->ice_collectable(true);
#endif

            BPtr b1 = test->returnTest3(d1, d3);

            test(b1);
            test(b1->sb == "D1.sb");
            test(b1->ice_id() == "::Test::D1");
            D1Ptr p1 = ICE_DYNAMIC_CAST(D1, b1);
            test(p1);
            test(p1->sd1 == "D1.sd1");
            test(p1->pd1 == b1->pb);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D3.sb");
            test(b2->ice_id() == "::Test::B");  // Sliced by server
            test(b2->pb == b1);
            D3Ptr p3 = ICE_DYNAMIC_CAST(D3, b2);
            test(!p3);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity for input params known first (AMI)... " << flush;
    {
        try
        {
            D1Ptr d1 = ICE_MAKE_SHARED(D1);
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
#ifndef ICE_CPP11_MAPPING
            d1->ice_collectable(true);
#endif

#ifdef ICE_CPP11_MAPPING
            auto f = test->returnTest3Async(d1, d3);
            auto b1 = f.get();
#else
            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d1, d3,
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr b1 = cb->rb;
#endif

            test(b1);
            test(b1->sb == "D1.sb");
            test(b1->ice_id() == "::Test::D1");
            D1Ptr p1 = ICE_DYNAMIC_CAST(D1, b1);
            test(p1);
            test(p1->sd1 == "D1.sd1");
            test(p1->pd1 == b1->pb);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D3.sb");
            test(b2->ice_id() == "::Test::B");  // Sliced by server
            test(b2->pb == b1);
            D3Ptr p3 = ICE_DYNAMIC_CAST(D3, b2);
            test(!p3);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity for input params unknown first... " << flush;
    {
        try
        {
            D1Ptr d1 = ICE_MAKE_SHARED(D1);
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
#ifndef ICE_CPP11_MAPPING
            d1->ice_collectable(true);
#endif
            BPtr b1 = test->returnTest3(d3, d1);

            test(b1);
            test(b1->sb == "D3.sb");
            test(b1->ice_id() == "::Test::B");  // Sliced by server

            D3Ptr p1 = ICE_DYNAMIC_CAST(D3, b1);
            test(!p1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D1.sb");
            test(b2->ice_id() == "::Test::D1");
            test(b2->pb == b1);

            D1Ptr p3 = ICE_DYNAMIC_CAST(D1, b2);
            test(p3);
            test(p3->sd1 == "D1.sd1");
            test(p3->pd1 == b1);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "return value identity for input params unknown first (AMI)... " << flush;
    {
        try
        {
            D1Ptr d1 = ICE_MAKE_SHARED(D1);
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
#ifndef ICE_CPP11_MAPPING
            d1->ice_collectable(true);
#endif

#ifdef ICE_CPP11_MAPPING
            auto f = test->returnTest3Async(d3, d1);
            auto b1 = f.get();
#else
            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d3, d1,
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr b1 = cb->rb;
#endif

            test(b1);
            test(b1->sb == "D3.sb");
            test(b1->ice_id() == "::Test::B");  // Sliced by server
            D3Ptr p1 = ICE_DYNAMIC_CAST(D3, b1);
            test(!p1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D1.sb");
            test(b2->ice_id() == "::Test::D1");
            test(b2->pb == b1);
            D1Ptr p3 = ICE_DYNAMIC_CAST(D1, b2);
            test(p3);
            test(p3->sd1 == "D1.sd1");
            test(p3->pd1 == b1);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (3 instances)... " << flush;
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr ret = test->paramTest3(p1, p2);

            test(p1);
            test(p1->sb == "D2.sb (p1 1)");
            test(p1->pb == 0);
            test(p1->ice_id() == "::Test::B");

            test(p2);
            test(p2->sb == "D2.sb (p2 1)");
            test(p2->pb == 0);
            test(p2->ice_id() == "::Test::B");

            test(ret);
            test(ret->sb == "D1.sb (p2 2)");
            test(ret->pb == 0);
            test(ret->ice_id() == "::Test::D1");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (3 instances) (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->paramTest3Async();
        try
        {
            auto result = f.get();
            auto ret = result.returnValue;
            auto p1 = result.p1;
            auto p2 = result.p2;

            test(p1);
            test(p1->sb == "D2.sb (p1 1)");
            test(p1->pb == 0);
            test(p1->ice_id() == "::Test::B");

            test(p2);
            test(p2->sb == "D2.sb (p2 1)");
            test(p2->pb == 0);
            test(p2->ice_id() == "::Test::B");

            test(ret);
            test(ret->sb == "D1.sb (p2 2)");
            test(ret->pb == 0);
            test(ret->ice_id() == "::Test::D1");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_paramTest3(
            newCallback_TestIntf_paramTest3(cb, &Callback::response_paramTest3, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (4 instances)... " << flush;
    {
        try
        {
            BPtr b;
            BPtr ret = test->paramTest4(b);

            test(b);
            test(b->sb == "D4.sb (1)");
            test(b->pb == 0);
            test(b->ice_id() == "::Test::B");

            test(ret);
            test(ret->sb == "B.sb (2)");
            test(ret->pb == 0);
            test(ret->ice_id() == "::Test::B");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "remainder unmarshaling (4 instances) (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto f = test->paramTest4Async();
        try
        {
            auto result = f.get();
            auto ret = move(result.returnValue);
            auto b = move(result.p);

            test(b);
            test(b->sb == "D4.sb (1)");
            test(b->pb == nullptr);
            test(b->ice_id() == "::Test::B");

            test(ret);
            test(ret->sb == "B.sb (2)");
            test(ret->pb == nullptr);
            test(ret->ice_id() == "::Test::B");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_paramTest4(
            newCallback_TestIntf_paramTest4(cb, &Callback::response_paramTest4, &Callback::exception));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as base... " << flush;
    {
        try
        {
            BPtr b1 = ICE_MAKE_SHARED(B);
            b1->sb = "B.sb(1)";
            b1->pb = b1;

            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = b1;
#ifndef ICE_CPP11_MAPPING
            d3->ice_collectable(true);
#endif

            BPtr b2 = ICE_MAKE_SHARED(B);
            b2->sb = "B.sb(2)";
            b2->pb = b1;

            BPtr r = test->returnTest3(d3, b2);

            test(r);
            test(r->ice_id() == "::Test::B");
            test(r->sb == "D3.sb");
            test(r->pb == r);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as base (AMI)... " << flush;
    {
        try
        {
            BPtr b1 = ICE_MAKE_SHARED(B);
            b1->sb = "B.sb(1)";
            b1->pb = b1;

            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = b1;
#ifndef ICE_CPP11_MAPPING
            d3->ice_collectable(true);
#endif

            BPtr b2 = ICE_MAKE_SHARED(B);
            b2->sb = "B.sb(2)";
            b2->pb = b1;

#ifdef ICE_CPP11_MAPPING
            auto r = test->returnTest3Async(d3, b2).get();
#else
            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d3, b2,
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr r = cb->rb;
#endif
            test(r);
            test(r->ice_id() == "::Test::B");
            test(r->sb == "D3.sb");
            test(r->pb == r);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as derived... "
         << flush;
    {
        try
        {
            D1Ptr d11 = ICE_MAKE_SHARED(D1);
            d11->sb = "D1.sb(1)";
            d11->pb = d11;
            d11->sd1 = "D1.sd1(1)";

            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = d11;
#ifndef ICE_CPP11_MAPPING
            d3->ice_collectable(true);
#endif
            D1Ptr d12 = ICE_MAKE_SHARED(D1);
            d12->sb = "D1.sb(2)";
            d12->pb = d12;
            d12->sd1 = "D1.sd1(2)";
            d12->pd1 = d11;
#ifndef ICE_CPP11_MAPPING
            d12->ice_collectable(true);
#endif
            BPtr r = test->returnTest3(d3, d12);
            test(r);
            test(r->ice_id() == "::Test::B");
            test(r->sb == "D3.sb");
            test(r->pb == r);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as derived (AMI)... "
         << flush;
    {
        try
        {
            D1Ptr d11 = ICE_MAKE_SHARED(D1);
            d11->sb = "D1.sb(1)";
            d11->pb = d11;
            d11->sd1 = "D1.sd1(1)";

            D3Ptr d3 = ICE_MAKE_SHARED(D3);
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = d11;
#ifndef ICE_CPP11_MAPPING
            d3->ice_collectable(true);
#endif
            D1Ptr d12 = ICE_MAKE_SHARED(D1);
            d12->sb = "D1.sb(2)";
            d12->pb = d12;
            d12->sd1 = "D1.sd1(2)";
            d12->pd1 = d11;
#ifndef ICE_CPP11_MAPPING
            d12->ice_collectable(true);
#endif

#ifdef ICE_CPP11_MAPPING
            auto r = test->returnTest3Async(d3, d12).get();
#else
            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d3, d12,
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr r = cb->rb;
#endif
            test(r);
            test(r->ice_id() == "::Test::B");
            test(r->sb == "D3.sb");
            test(r->pb == r);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "sequence slicing... " << flush;
    {
        try
        {
            SS3 ss;
            {
                BPtr ss1b = ICE_MAKE_SHARED(B);
                ss1b->sb = "B.sb";
                ss1b->pb = ss1b;

                D1Ptr ss1d1 = ICE_MAKE_SHARED(D1);
                ss1d1->sb = "D1.sb";
                ss1d1->sd1 = "D1.sd1";
                ss1d1->pb = ss1b;

                D3Ptr ss1d3 = ICE_MAKE_SHARED(D3);
                ss1d3->sb = "D3.sb";
                ss1d3->sd3 = "D3.sd3";
                ss1d3->pb = ss1b;
#ifndef ICE_CPP11_MAPPING
                ss1d3->ice_collectable(true);
#endif
                BPtr ss2b = ICE_MAKE_SHARED(B);
                ss2b->sb = "B.sb";
                ss2b->pb = ss1b;

                D1Ptr ss2d1 = ICE_MAKE_SHARED(D1);
                ss2d1->sb = "D1.sb";
                ss2d1->sd1 = "D1.sd1";
                ss2d1->pb = ss2b;

                D3Ptr ss2d3 = ICE_MAKE_SHARED(D3);
                ss2d3->sb = "D3.sb";
                ss2d3->sd3 = "D3.sd3";
                ss2d3->pb = ss2b;

                ss1d1->pd1 = ss2b;
                ss1d3->pd3 = ss2d1;

                ss2d1->pd1 = ss1d3;
                ss2d3->pd3 = ss1d1;

                SS1Ptr ss1 = ICE_MAKE_SHARED(SS1);
                ss1->s.push_back(ss1b);
                ss1->s.push_back(ss1d1);
                ss1->s.push_back(ss1d3);

                SS2Ptr ss2 = ICE_MAKE_SHARED(SS2);
                ss2->s.push_back(ss2b);
                ss2->s.push_back(ss2d1);
                ss2->s.push_back(ss2d3);

#ifndef ICE_CPP11_MAPPING
                ss1->ice_collectable(true);
                ss2->ice_collectable(true);
#endif
                ss = test->sequenceTest(ss1, ss2);
            }

            test(ss.c1);
            BPtr ss1b = ss.c1->s[0];
            BPtr ss1d1 = ss.c1->s[1];
            test(ss.c2);
            BPtr ss1d3 = ss.c1->s[2];

            test(ss.c2);
            BPtr ss2b = ss.c2->s[0];
            BPtr ss2d1 = ss.c2->s[1];
            BPtr ss2d3 = ss.c2->s[2];

            test(ss1b->pb == ss1b);
            test(ss1d1->pb == ss1b);
            test(ss1d3->pb == ss1b);

            test(ss2b->pb == ss1b);
            test(ss2d1->pb == ss2b);
            test(ss2d3->pb == ss2b);

            test(ss1b->ice_id() == "::Test::B");
            test(ss1d1->ice_id() == "::Test::D1");
            test(ss1d3->ice_id() == "::Test::B");

            test(ss2b->ice_id() == "::Test::B");
            test(ss2d1->ice_id() == "::Test::D1");
            test(ss2d3->ice_id() == "::Test::B");
        }
        catch(const ::Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "sequence slicing (AMI)... " << flush;
    {
        try
        {
            SS3 ss;
            {
                BPtr ss1b = ICE_MAKE_SHARED(B);
                ss1b->sb = "B.sb";
                ss1b->pb = ss1b;

                D1Ptr ss1d1 = ICE_MAKE_SHARED(D1);
                ss1d1->sb = "D1.sb";
                ss1d1->sd1 = "D1.sd1";
                ss1d1->pb = ss1b;

                D3Ptr ss1d3 = ICE_MAKE_SHARED(D3);
                ss1d3->sb = "D3.sb";
                ss1d3->sd3 = "D3.sd3";
                ss1d3->pb = ss1b;
#ifndef ICE_CPP11_MAPPING
                ss1d3->ice_collectable(true);
#endif

                BPtr ss2b = ICE_MAKE_SHARED(B);
                ss2b->sb = "B.sb";
                ss2b->pb = ss1b;

                D1Ptr ss2d1 = ICE_MAKE_SHARED(D1);
                ss2d1->sb = "D1.sb";
                ss2d1->sd1 = "D1.sd1";
                ss2d1->pb = ss2b;

                D3Ptr ss2d3 = ICE_MAKE_SHARED(D3);
                ss2d3->sb = "D3.sb";
                ss2d3->sd3 = "D3.sd3";
                ss2d3->pb = ss2b;

                ss1d1->pd1 = ss2b;
                ss1d3->pd3 = ss2d1;

                ss2d1->pd1 = ss1d3;
                ss2d3->pd3 = ss1d1;

                SS1Ptr ss1 = ICE_MAKE_SHARED(SS1);
                ss1->s.push_back(ss1b);
                ss1->s.push_back(ss1d1);
                ss1->s.push_back(ss1d3);

                SS2Ptr ss2 = ICE_MAKE_SHARED(SS2);
                ss2->s.push_back(ss2b);
                ss2->s.push_back(ss2d1);
                ss2->s.push_back(ss2d3);
#ifndef ICE_CPP11_MAPPING
                ss1->ice_collectable(true);
                ss2->ice_collectable(true);
#endif

#ifdef ICE_CPP11_MAPPING
                ss = test->sequenceTestAsync(ss1, ss2).get();
#else
                CallbackPtr cb = new Callback;
                test->begin_sequenceTest(ss1, ss2,
                    newCallback_TestIntf_sequenceTest(cb, &Callback::response_sequenceTest, &Callback::exception));
                cb->check();
                ss = cb->rss3;
#endif
            }

            test(ss.c1);
            BPtr ss1b = ss.c1->s[0];
            BPtr ss1d1 = ss.c1->s[1];
            test(ss.c2);
            BPtr ss1d3 = ss.c1->s[2];

            test(ss.c2);
            BPtr ss2b = ss.c2->s[0];
            BPtr ss2d1 = ss.c2->s[1];
            BPtr ss2d3 = ss.c2->s[2];

            test(ss1b->pb == ss1b);
            test(ss1d1->pb == ss1b);
            test(ss1d3->pb == ss1b);

            test(ss2b->pb == ss1b);
            test(ss2d1->pb == ss2b);
            test(ss2d3->pb == ss2b);

            test(ss1b->ice_id() == "::Test::B");
            test(ss1d1->ice_id() == "::Test::D1");
            test(ss1d3->ice_id() == "::Test::B");

            test(ss2b->ice_id() == "::Test::B");
            test(ss2d1->ice_id() == "::Test::D1");
            test(ss2d3->ice_id() == "::Test::B");
        }
        catch(const ::Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "dictionary slicing... " << flush;
    {
        try
        {
            BDict bin;
            BDict bout;
            BDict r;
            int i;
            for(i = 0; i < 10; ++i)
            {
                ostringstream s;
                s << "D1." << i;
                D1Ptr d1 = ICE_MAKE_SHARED(D1);
                d1->sb = s.str();
                d1->pb = d1;
                d1->sd1 = s.str();
                bin[i] = d1;
#ifndef ICE_CPP11_MAPPING
                d1->ice_collectable(true);
#endif
            }

            r = test->dictionaryTest(bin, bout);

            test(bout.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                BPtr b = bout.find(i * 10)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i;
                test(b->sb == s.str());
                test(b->pb);
                test(b->pb != b);
                test(b->pb->sb == s.str());
                test(b->pb->pb == b->pb);
            }

            test(r.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                BPtr b = r.find(i * 20)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i * 20;
                test(b->sb == s.str());
                test(b->pb == (i == 0 ? BPtr(0) : r.find((i - 1) * 20)->second));
                D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b);
                test(d1);
                test(d1->sd1 == s.str());
                test(d1->pd1 == d1);
            }
        }
        catch(const ::Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "dictionary slicing (AMI)... " << flush;
    {
        try
        {
            BDict bin;
            BDict bout;
            BDict r;
            int i;
            for(i = 0; i < 10; ++i)
            {
                ostringstream s;
                s << "D1." << i;
                D1Ptr d1 = ICE_MAKE_SHARED(D1);
                d1->sb = s.str();
                d1->pb = d1;
                d1->sd1 = s.str();
                bin[i] = d1;
#ifndef ICE_CPP11_MAPPING
                d1->ice_collectable(true);
#endif
            }

#ifdef ICE_CPP11_MAPPING
            auto result = test->dictionaryTestAsync(bin).get();
            r = result.returnValue;
            bout = result.bout;
#else
            CallbackPtr cb = new Callback;
            test->begin_dictionaryTest(bin,
                newCallback_TestIntf_dictionaryTest(cb, &Callback::response_dictionaryTest, &Callback::exception));
            cb->check();
            bout = cb->obdict;
            r = cb->rbdict;
#endif

            test(bout.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                BPtr b = bout.find(i * 10)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i;
                test(b->sb == s.str());
                test(b->pb);
                test(b->pb != b);
                test(b->pb->sb == s.str());
                test(b->pb->pb == b->pb);
            }

            test(r.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                BPtr b = r.find(i * 20)->second;
                test(b);
                std::ostringstream s;
                s << "D1." << i * 20;
                test(b->sb == s.str());
                test(b->pb == (i == 0 ? BPtr(0) : r.find((i - 1) * 20)->second));
                D1Ptr d1 = ICE_DYNAMIC_CAST(D1, b);
                test(d1);
                test(d1->sd1 == s.str());
                test(d1->pd1 == d1);
            }
        }
        catch(const ::Ice::Exception&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base exception thrown as base exception... " << flush;
    {
        try
        {
            test->throwBaseAsBase();
            test(false);
        }
        catch(const BaseException& e)
        {
            test(e.ice_id() == "::Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb");
            test(e.pb->pb == e.pb);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base exception thrown as base exception (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        try
        {
            test->throwBaseAsBaseAsync().get();
            test(false);
        }
        catch(const BaseException& ex)
        {
            test(ex.ice_id() == "::Test::BaseException");
            test(ex.sbe == "sbe");
            test(ex.pb);
            test(ex.pb->sb == "sb");
            test(ex.pb->pb == ex.pb);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_throwBaseAsBase(
            newCallback_TestIntf_throwBaseAsBase(cb, &Callback::response, &Callback::exception_throwBaseAsBase));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as base exception... " << flush;
    {
        try
        {
            test->throwDerivedAsBase();
            test(false);
        }
        catch(const DerivedException& e)
        {
            test(e.ice_id() == "::Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as base exception (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        try
        {
            test->throwDerivedAsBaseAsync().get();
            test(false);
        }
        catch(const DerivedException& ex)
        {
            test(ex.ice_id() == "::Test::DerivedException");
            test(ex.sbe == "sbe");
            test(ex.pb);
            test(ex.pb->sb == "sb1");
            test(ex.pb->pb == ex.pb);
            test(ex.sde == "sde1");
            test(ex.pd1);
            test(ex.pd1->sb == "sb2");
            test(ex.pd1->pb == ex.pd1);
            test(ex.pd1->sd1 == "sd2");
            test(ex.pd1->pd1 == ex.pd1);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_throwDerivedAsBase(
            newCallback_TestIntf_throwDerivedAsBase(cb, &Callback::response, &Callback::exception_throwDerivedAsBase));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as derived exception... " << flush;
    {
        try
        {
            test->throwDerivedAsDerived();
            test(false);
        }
        catch(const DerivedException& e)
        {
            test(e.ice_id() == "::Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "derived exception thrown as derived exception (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        try
        {
            test->throwDerivedAsDerivedAsync().get();
            test(false);
        }
        catch(const DerivedException& e)
        {
            test(e.ice_id() == "::Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_throwDerivedAsDerived(
            newCallback_TestIntf_throwDerivedAsDerived(
                cb, &Callback::response, &Callback::exception_throwDerivedAsDerived));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "unknown derived exception thrown as base exception... " << flush;
    {
        try
        {
            test->throwUnknownDerivedAsBase();
            test(false);
        }
        catch(const BaseException& e)
        {
            test(e.ice_id() == "::Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb d2");
            test(e.pb->pb == e.pb);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "unknown derived exception thrown as base exception (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        try
        {
            test->throwUnknownDerivedAsBaseAsync().get();
            test(false);
        }
        catch(const BaseException& e)
        {
            test(e.ice_id() == "::Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb d2");
            test(e.pb->pb == e.pb);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_throwUnknownDerivedAsBase(
            newCallback_TestIntf_throwUnknownDerivedAsBase(
                cb, &Callback::response, &Callback::exception_throwUnknownDerivedAsBase));
        cb->check();
#endif
    }
    cout << "ok" << endl;

    cout << "preserved classes... " << flush;
    try
    {
        //
        // Server knows the most-derived class PDerived.
        //
        PDerivedPtr pd = ICE_MAKE_SHARED(PDerived);
        pd->pi = 3;
        pd->ps = "preserved";
        pd->pb = pd;
#ifndef ICE_CPP11_MAPPING
        pd->ice_collectable(true);
#endif
        PBasePtr r = test->exchangePBase(pd);
        PDerivedPtr p2 = ICE_DYNAMIC_CAST(PDerived, r);
        test(p2);
        test(p2->pi == 3);
        test(p2->ps == "preserved");
        test(p2->pb == p2);
    }
    catch(const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Server only knows the base (non-preserved) type, so the object is sliced.
        //
        PCUnknownPtr pu = ICE_MAKE_SHARED(PCUnknown);
        pu->pi = 3;
        pu->pu = "preserved";
        PBasePtr r = test->exchangePBase(pu);
        PCUnknownPtr p2 = ICE_DYNAMIC_CAST(PCUnknown, r);
        test(!p2);
        test(r->pi == 3);
    }
    catch(const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        PCDerivedPtr pcd = ICE_MAKE_SHARED(PCDerived);
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
#ifndef ICE_CPP11_MAPPING
        pcd->ice_collectable(true);
#endif

        PBasePtr r = test->exchangePBase(pcd);
        PCDerivedPtr p2 = ICE_DYNAMIC_CAST(PCDerived, r);
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p2);
            test(r->pi == 3);
        }
        else
        {
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
        }
    }
    catch(const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Server only knows the intermediate type CompactPDerived. The object will be sliced to
        // CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
        //
        CompactPCDerivedPtr pcd = ICE_MAKE_SHARED(CompactPCDerived);
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
#ifndef ICE_CPP11_MAPPING
        pcd->ice_collectable(true);
#endif

        PBasePtr r = test->exchangePBase(pcd);
        CompactPCDerivedPtr p2 = ICE_DYNAMIC_CAST(CompactPCDerived, r);
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p2);
            test(r->pi == 3);
        }
        else
        {
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
        }
    }
    catch(const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Send an object that will have multiple preserved slices in the server.
        // The object will be sliced to Preserved for the 1.0 encoding.
        //
        PCDerived3Ptr pcd = ICE_MAKE_SHARED(PCDerived3);
        pcd->pi = 3;
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        int i;
        for(i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = ICE_MAKE_SHARED(PCDerived2);
            p2->pi = i;
            p2->pbs.push_back(0); // Nil reference. This slice should not have an indirection table.
            p2->pcd2 = i;
            pcd->pbs.push_back(p2);
        }
        pcd->pcd2 = pcd->pi;
        pcd->pcd3 = pcd->pbs[10];

        PBasePtr r = test->exchangePBase(pcd);
        PCDerived3Ptr p3 = ICE_DYNAMIC_CAST(PCDerived3, r);
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p3);
            test(ICE_DYNAMIC_CAST(Preserved, r));
            test(r->pi == 3);
        }
        else
        {
            test(p3);
            test(p3->pi == 3);
            for(i = 0; i < 300; ++i)
            {
                PCDerived2Ptr p2 = ICE_DYNAMIC_CAST(PCDerived2, p3->pbs[i]);
                test(p2->pi == i);
                test(p2->pbs.size() == 1);
                test(!p2->pbs[0]);
                test(p2->pcd2 == i);
            }
            test(p3->pcd2 == p3->pi);
            test(p3->pcd3 == p3->pbs[10]);
        }
    }
    catch(const Ice::OperationNotExistException&)
    {
    }

    try
    {
        //
        // Obtain an object with preserved slices and send it back to the server.
        // The preserved slices should be excluded for the 1.0 encoding, otherwise
        // they should be included.
        //
        PreservedPtr p = test->PBSUnknownAsPreserved();
        test->checkPBSUnknown(p);
        if(test->ice_getEncodingVersion() != Ice::Encoding_1_0)
        {
            test->ice_encodingVersion(Ice::Encoding_1_0)->checkPBSUnknown(p);
        }
    }
    catch(const Ice::OperationNotExistException&)
    {
    }

    cout << "ok" << endl;

    cout << "preserved classes (AMI)... " << flush;
    {
        //
        // Server knows the most-derived class PDerived.
        //
        PDerivedPtr pd = ICE_MAKE_SHARED(PDerived);
        pd->pi = 3;
        pd->ps = "preserved";
        pd->ps = "preserved";
        pd->pb = pd;

#ifndef ICE_CPP11_MAPPING
        pd->ice_collectable(true);
#endif

#ifdef ICE_CPP11_MAPPING
        try
        {
            pd = dynamic_pointer_cast<PDerived>(test->exchangePBaseAsync(pd).get());
            test(pd);
            test(pd->pi == 3);
            test(pd->ps == "preserved");
            test(pd->pb == pd);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_exchangePBase(
            pd, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved1, &Callback::exception));
        cb->check();
#endif
    }

    {
        //
        // Server only knows the base (non-preserved) type, so the object is sliced.
        //
        PCUnknownPtr pu = ICE_MAKE_SHARED(PCUnknown);
        pu->pi = 3;
        pu->pu = "preserved";
#ifdef ICE_CPP11_MAPPING
        try
        {
            auto r = test->exchangePBaseAsync(pu).get();
            auto p2 = dynamic_pointer_cast<PCUnknown>(r);
            test(!p2);
            test(r->pi == 3);
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_exchangePBase(
            pu, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved2, &Callback::exception));
        cb->check();
#endif
    }

    {
        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        PCDerivedPtr pcd = ICE_MAKE_SHARED(PCDerived);
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
#ifdef ICE_CPP11_MAPPING
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            auto r = test->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<PCDerived>(r);
            test(!p2);
            test(r->pi == 3);
        }
        else
        {
            auto r = test->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<PCDerived>(r);
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
        }
#else
        pcd->ice_collectable(true);

        CallbackPtr cb = new Callback;
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test->begin_exchangePBase(
                pcd, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved3, &Callback::exception));
        }
        else
        {
            test->begin_exchangePBase(
                pcd, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved4, &Callback::exception));
        }
        cb->check();
#endif
    }

    {
        //
        // Server only knows the intermediate type CompactPDerived. The object will be sliced to
        // CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
        //
        CompactPCDerivedPtr pcd = ICE_MAKE_SHARED(CompactPCDerived);
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);

#ifdef ICE_CPP11_MAPPING
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            auto r = test->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<CompactPCDerived>(r);
            test(!p2);
            test(r->pi == 3);
        }
        else
        {
            auto r = test->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<CompactPCDerived>(r);
            test(p2);
            test(p2->pi == 3);
            test(p2->pbs[0] == p2);
        }
#else
        pcd->ice_collectable(true);

        CallbackPtr cb = new Callback;
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test->begin_exchangePBase(pcd, newCallback_TestIntf_exchangePBase(cb,
                                                                              &Callback::response_compactPreserved1,
                                                                              &Callback::exception));
        }
        else
        {
            test->begin_exchangePBase(pcd, newCallback_TestIntf_exchangePBase(cb,
                                                                              &Callback::response_compactPreserved2,
                                                                              &Callback::exception));
        }
        cb->check();
#endif
    }

    {
        //
        // Send an object that will have multiple preserved slices in the server.
        // The object will be sliced to Preserved for the 1.0 encoding.
        //
        PCDerived3Ptr pcd = ICE_MAKE_SHARED(PCDerived3);
        pcd->pi = 3;
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        int i;
        for(i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = ICE_MAKE_SHARED(PCDerived2);
            p2->pi = i;
            p2->pbs.push_back(0); // Nil reference. This slice should not have an indirection table.
            p2->pcd2 = i;
            pcd->pbs.push_back(p2);
        }
        pcd->pcd2 = pcd->pi;
        pcd->pcd3 = pcd->pbs[10];

#ifdef ICE_CPP11_MAPPING
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            auto r = test->exchangePBaseAsync(pcd).get();
            auto p2 = dynamic_pointer_cast<PCDerived>(r);
            test(!p2);
            test(r->pi == 3);
        }
        else
        {
            auto r = test->exchangePBaseAsync(pcd).get();
            auto p3 = dynamic_pointer_cast<PCDerived3>(r);
            test(p3);
            test(p3->pi == 3);
            for(int i = 0; i < 300; ++i)
            {
                auto p2 = dynamic_pointer_cast<PCDerived2>(p3->pbs[i]);
                test(p2->pi == i);
                test(p2->pbs.size() == 1);
                test(!p2->pbs[0]);
                test(p2->pcd2 == i);
            }
            test(p3->pcd2 == p3->pi);
            test(p3->pcd3 == p3->pbs[10]);
        }
#else
        CallbackPtr cb = new Callback;
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test->begin_exchangePBase(
                pcd, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved3, &Callback::exception));
        }
        else
        {
            test->begin_exchangePBase(
                pcd, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved5, &Callback::exception));
        }
        cb->check();
#endif
    }
    cout << "ok" << endl;

#ifndef ICE_CPP11_MAPPING
    cout << "garbage collection for preserved classes... " << flush;
    try
    {
        //
        // Register a factory in order to substitute our own subclass of PNode. This provides
        // an easy way to determine how many unmarshaled instances currently exist.
        //
        communicator->getValueFactoryManager()->add(new NodeFactoryI, PNode::ice_staticId());

        //
        // Relay a graph through the server. This test uses a preserved class
        // with a class member, so the preserved class already supports GC.
        //
        {
            PNodePtr c = new PNode;
            c->next = new PNode;
            c->next->next = new PNode;
            c->next->next->next = c;

            test(PNodeI::counter == 0);
            PNodePtr n = test->exchangePNode(c);
            c->next->next->next = 0; // Break the cycle.

            test(PNodeI::counter == 3);
            n = 0;                          // Release reference.
            test(PNodeI::counter == 0);
        }

        //
        // Obtain a preserved object from the server where the most-derived
        // type is unknown. The preserved slice refers to a graph of PNode
        // objects.
        //
        {
            test(PNodeI::counter == 0);
            PreservedPtr p = test->PBSUnknownAsPreservedWithGraph();
            test->checkPBSUnknownWithGraph(p);
            if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
            {
                test(PNodeI::counter == 0);
            }
            else
            {
                test(PNodeI::counter == 3);
                p = 0;                          // Release reference.
                test(PNodeI::counter == 0);
            }
        }

        //
        // Register a factory in order to substitute our own subclass of Preserved. This provides
        // an easy way to determine how many unmarshaled instances currently exist.
        //
        communicator->getValueFactoryManager()->add(new PreservedFactoryI, Preserved::ice_staticId());

        //
        // Obtain a preserved object from the server where the most-derived
        // type is unknown. A data member in the preserved slice refers to the
        // outer object, so the chain of references looks like this:
        //
        // outer->slicedData->outer
        //
        {
            test(PreservedI::counter == 0);
            PreservedPtr p = test->PBSUnknown2AsPreservedWithGraph();
            test->checkPBSUnknown2WithGraph(p);
            test(PreservedI::counter == 1);
            p = 0;                          // Release reference.
            test(PreservedI::counter == 0);
        }

        //
        // Throw a preserved exception where the most-derived type is unknown.
        // The preserved exception slice contains a class data member. This
        // object is also preserved, and its most-derived type is also unknown.
        // The preserved slice of the object contains a class data member that
        // refers to itself.
        //
        // The chain of references looks like this:
        //
        // ex->slicedData->obj->slicedData->obj
        //
        try
        {
            test(PreservedI::counter == 0);

            try
            {
                test->throwPreservedException();
            }
            catch(const PreservedException&)
            {
                //
                // The class instance is only retained when the encoding is > 1.0.
                //
                if(test->ice_getEncodingVersion() != Ice::Encoding_1_0)
                {
                    test(PreservedI::counter == 1);
                }
            }

            //
            // Exception has gone out of scope.
            //
            test(PreservedI::counter == 0);
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    cout << "ok" << endl;
#endif
    return test;
}
