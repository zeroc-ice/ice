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

class Callback : public CallbackBase, public IceUtil::Shared
{
public:

    void
    response_SBaseAsObject(const ::Ice::ObjectPtr& o)
    {
        test(o);
        test(o->ice_id() == "::Test::SBase");
        SBasePtr sb = SBasePtr::dynamicCast(o);
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
        SBSKnownDerivedPtr sbskd = SBSKnownDerivedPtr::dynamicCast(sb);
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
        test(exc.ice_name() == "Ice::NoObjectFactoryException");
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
        test(exc.ice_name() == "Ice::NoObjectFactoryException");
        called();
    }

    void
    response_SUnknownAsObject11(const Ice::ObjectPtr& o)
    {
        test(Ice::UnknownSlicedObjectPtr::dynamicCast(o));
        test(Ice::UnknownSlicedObjectPtr::dynamicCast(o)->getUnknownTypeId() == "::Test::SUnknown");
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
        D1Ptr d1 = D1Ptr::dynamicCast(b1);
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
        D1Ptr d1 = D1Ptr::dynamicCast(b1);
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
        D1Ptr d1 = D1Ptr::dynamicCast(b1);
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
        test(ex.ice_name() == "Test::BaseException");
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
        test(ex.ice_name() == "Test::DerivedException");
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
        test(ex.ice_name() == "Test::DerivedException");
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
        test(ex.ice_name() == "Test::BaseException");
        const BaseException& e = dynamic_cast<const BaseException&>(ex);
        test(e.sbe == "sbe");
        test(e.pb);
        test(e.pb->sb == "sb d2");
        test(e.pb->pb == e.pb);
        called();
    }

    void
    response_useForward(const ForwardPtr& f)
    {
        test(f);
        called();
    }

    void
    response_preserved1(const PBasePtr& r)
    {
        PDerivedPtr pd = PDerivedPtr::dynamicCast(r);
        test(pd);
        test(pd->pi == 3);
        test(pd->ps == "preserved");
        test(pd->pb == pd);
        called();
    }

    void
    response_preserved2(const PBasePtr& r)
    {
        PCUnknownPtr p2 = PCUnknownPtr::dynamicCast(r);
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
        PCDerivedPtr p2 = PCDerivedPtr::dynamicCast(r);
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
        PCDerivedPtr p2 = PCDerivedPtr::dynamicCast(r);
        test(p2);
        test(p2->pi == 3);
        test(p2->pbs[0] == p2);
        called();
    }

    void
    response_preserved5(const PBasePtr& r)
    {
        PCDerived3Ptr p3 = PCDerived3Ptr::dynamicCast(r);
        test(p3);
        test(p3->pi == 3);
        for(int i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = PCDerived2Ptr::dynamicCast(p3->pbs[i]);
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
        CompactPCDerivedPtr p2 = CompactPCDerivedPtr::dynamicCast(r);
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
        CompactPCDerivedPtr p2 = CompactPCDerivedPtr::dynamicCast(r);
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
typedef IceUtil::Handle<Callback> CallbackPtr;

class PNodeI : virtual public PNode
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

class NodeFactoryI : public Ice::ObjectFactory
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

class PreservedI : virtual public Preserved
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

class PreservedFactoryI : public Ice::ObjectFactory
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

void
testUOO(const TestIntfPrx& test)
{
    Ice::ObjectPtr o;
    try
    {
        o = test->SUnknownAsObject();
        test(test->ice_getEncodingVersion() != Ice::Encoding_1_0);
        test(Ice::UnknownSlicedObjectPtr::dynamicCast(o));
        test(Ice::UnknownSlicedObjectPtr::dynamicCast(o)->getUnknownTypeId() == "::Test::SUnknown");
        test->checkSUnknown(o);
    }
    catch(const Ice::NoObjectFactoryException&)
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

TestIntfPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectPrx obj = communicator->stringToProxy("Test:default -p 12010");
    TestIntfPrx test = TestIntfPrx::checkedCast(obj);

    cout << "base as Object... " << flush;
    {
        Ice::ObjectPtr o;
        try
        {
            o = test->SBaseAsObject();
            test(o);
            test(o->ice_id() == "::Test::SBase");
        }
        catch(...)
        {
            test(false);
        }
        SBasePtr sb = SBasePtr::dynamicCast(o);
        test(sb);
        test(sb->sb == "SBase.sb");
    }
    cout << "ok" << endl;

    cout << "base as Object (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_SBaseAsObject(
            newCallback_TestIntf_SBaseAsObject(cb, &Callback::response_SBaseAsObject, &Callback::exception));
        cb->check();
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
        CallbackPtr cb = new Callback;
        test->begin_SBaseAsSBase(
            newCallback_TestIntf_SBaseAsSBase(cb, &Callback::response_SBaseAsSBase, &Callback::exception));
        cb->check();
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
        SBSKnownDerivedPtr sbskd = SBSKnownDerivedPtr::dynamicCast(sb);
        test(sbskd);
        test(sbskd->sbskd == "SBSKnownDerived.sbskd");
    }
    cout << "ok" << endl;

    cout << "base with known derived as base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_SBSKnownDerivedAsSBase(
            newCallback_TestIntf_SBSKnownDerivedAsSBase(
                cb, &Callback::response_SBSKnownDerivedAsSBase, &Callback::exception));
        cb->check();
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
        CallbackPtr cb = new Callback;
        test->begin_SBSKnownDerivedAsSBSKnownDerived(
            newCallback_TestIntf_SBSKnownDerivedAsSBSKnownDerived(
                cb, &Callback::response_SBSKnownDerivedAsSBSKnownDerived, &Callback::exception));
        cb->check();
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
        catch(const Ice::NoObjectFactoryException&)
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
        CallbackPtr cb = new Callback;
        test->begin_SBSUnknownDerivedAsSBase(
            newCallback_TestIntf_SBSUnknownDerivedAsSBase(
                cb, &Callback::response_SBSUnknownDerivedAsSBase, &Callback::exception));
        cb->check();
    }
    if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
    {
        //
        // This test succeeds for the 1.0 encoding.
        //
        CallbackPtr cb = new Callback;
        test->begin_SBSUnknownDerivedAsSBaseCompact(
            newCallback_TestIntf_SBSUnknownDerivedAsSBaseCompact(
                cb, &Callback::response_SBSUnknownDerivedAsSBase, &Callback::exception));
        cb->check();
    }
    else
    {
        //
        // This test fails when using the compact format because the instance cannot
        // be sliced to a known type.
        //
        CallbackPtr cb = new Callback;
        test->begin_SBSUnknownDerivedAsSBaseCompact(
            newCallback_TestIntf_SBSUnknownDerivedAsSBaseCompact(
                cb, &Callback::response_SBSUnknownDerivedAsSBaseCompact,
                &Callback::exception_SBSUnknownDerivedAsSBaseCompact));
        cb->check();
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
        CallbackPtr cb = new Callback;
        test->begin_oneElementCycle(
            newCallback_TestIntf_oneElementCycle(
                cb, &Callback::response_oneElementCycle, &Callback::exception));
        cb->check();
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
        CallbackPtr cb = new Callback;
        test->begin_twoElementCycle(
            newCallback_TestIntf_twoElementCycle(
                cb, &Callback::response_twoElementCycle, &Callback::exception));
        cb->check();
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
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
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
        CallbackPtr cb = new Callback;
        test->begin_D1AsB(newCallback_TestIntf_D1AsB(cb, &Callback::response_D1AsB, &Callback::exception));
        cb->check();
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
        CallbackPtr cb = new Callback;
        test->begin_D1AsD1(newCallback_TestIntf_D1AsD1(cb, &Callback::response_D1AsD1, &Callback::exception));
        cb->check();
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
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
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
        CallbackPtr cb = new Callback;
        test->begin_D2AsB(newCallback_TestIntf_D2AsB(cb, &Callback::response_D2AsB, &Callback::exception));
        cb->check();
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
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
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
        CallbackPtr cb = new Callback;
        test->begin_paramTest1(
            newCallback_TestIntf_paramTest1(cb, &Callback::response_paramTest1, &Callback::exception));
        cb->check();
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
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
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
        CallbackPtr cb = new Callback;
        test->begin_returnTest1(
            newCallback_TestIntf_returnTest1(cb, &Callback::response_returnTest1, &Callback::exception));
        cb->check();
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
        CallbackPtr cb = new Callback;
        test->begin_returnTest2(
            newCallback_TestIntf_returnTest2(cb, &Callback::response_returnTest2, &Callback::exception));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "return value identity for input params known first... " << flush;
    {
        try
        {
            D1Ptr d1 = new D1;
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = new D3;
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
            d1->ice_collectable(true);

            BPtr b1 = test->returnTest3(d1, d3);

            test(b1);
            test(b1->sb == "D1.sb");
            test(b1->ice_id() == "::Test::D1");
            D1Ptr p1 = D1Ptr::dynamicCast(b1);
            test(p1);
            test(p1->sd1 == "D1.sd1");
            test(p1->pd1 == b1->pb);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D3.sb");
            test(b2->ice_id() == "::Test::B");  // Sliced by server
            test(b2->pb == b1);
            D3Ptr p3 = D3Ptr::dynamicCast(b2);
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
            D1Ptr d1 = new D1;
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = new D3;
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
            d1->ice_collectable(true);
            
            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d1, d3,
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr b1 = cb->rb;

            test(b1);
            test(b1->sb == "D1.sb");
            test(b1->ice_id() == "::Test::D1");
            D1Ptr p1 = D1Ptr::dynamicCast(b1);
            test(p1);
            test(p1->sd1 == "D1.sd1");
            test(p1->pd1 == b1->pb);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D3.sb");
            test(b2->ice_id() == "::Test::B");  // Sliced by server
            test(b2->pb == b1);
            D3Ptr p3 = D3Ptr::dynamicCast(b2);
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
            D1Ptr d1 = new D1;
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = new D3;
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
            d1->ice_collectable(true);
            
            BPtr b1 = test->returnTest3(d3, d1);

            test(b1);
            test(b1->sb == "D3.sb");
            test(b1->ice_id() == "::Test::B");  // Sliced by server
            D3Ptr p1 = D3Ptr::dynamicCast(b1);
            test(!p1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D1.sb");
            test(b2->ice_id() == "::Test::D1");
            test(b2->pb == b1);
            D1Ptr p3 = D1Ptr::dynamicCast(b2);
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
            D1Ptr d1 = new D1;
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = new D3;
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;
            d1->ice_collectable(true);
            
            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d3, d1, 
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr b1 = cb->rb;

            test(b1);
            test(b1->sb == "D3.sb");
            test(b1->ice_id() == "::Test::B");  // Sliced by server
            D3Ptr p1 = D3Ptr::dynamicCast(b1);
            test(!p1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D1.sb");
            test(b2->ice_id() == "::Test::D1");
            test(b2->pb == b1);
            D1Ptr p3 = D1Ptr::dynamicCast(b2);
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
        CallbackPtr cb = new Callback;
        test->begin_paramTest3(
            newCallback_TestIntf_paramTest3(cb, &Callback::response_paramTest3, &Callback::exception));
        cb->check();
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
        CallbackPtr cb = new Callback;
        test->begin_paramTest4(
            newCallback_TestIntf_paramTest4(cb, &Callback::response_paramTest4, &Callback::exception));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "param ptr slicing, instance marshaled in unknown derived as base... " << flush;
    {
        try
        {
            BPtr b1 = new B;
            b1->sb = "B.sb(1)";
            b1->pb = b1;

            D3Ptr d3 = new D3;
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = b1;
            d3->ice_collectable(true);

            BPtr b2 = new B;
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
            BPtr b1 = new B;
            b1->sb = "B.sb(1)";
            b1->pb = b1;

            D3Ptr d3 = new D3;
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = b1;
            d3->ice_collectable(true);

            BPtr b2 = new B;
            b2->sb = "B.sb(2)";
            b2->pb = b1;

            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d3, b2,
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr r = cb->rb;

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
            D1Ptr d11 = new D1;
            d11->sb = "D1.sb(1)";
            d11->pb = d11;
            d11->sd1 = "D1.sd1(1)";

            D3Ptr d3 = new D3;
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = d11;
            d3->ice_collectable(true);
            
            D1Ptr d12 = new D1;
            d12->sb = "D1.sb(2)";
            d12->pb = d12;
            d12->sd1 = "D1.sd1(2)";
            d12->pd1 = d11;
            d12->ice_collectable(true);
            
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
            D1Ptr d11 = new D1;
            d11->sb = "D1.sb(1)";
            d11->pb = d11;
            d11->sd1 = "D1.sd1(1)";

            D3Ptr d3 = new D3;
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = d11;
            d3->ice_collectable(true);
            
            D1Ptr d12 = new D1;
            d12->sb = "D1.sb(2)";
            d12->pb = d12;
            d12->sd1 = "D1.sd1(2)";
            d12->pd1 = d11;
            d12->ice_collectable(true);

            CallbackPtr cb = new Callback;
            test->begin_returnTest3(d3, d12,
                newCallback_TestIntf_returnTest3(cb, &Callback::response_returnTest3, &Callback::exception));
            cb->check();
            BPtr r = cb->rb;
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
                BPtr ss1b = new B;
                ss1b->sb = "B.sb";
                ss1b->pb = ss1b;

                D1Ptr ss1d1 = new D1;
                ss1d1->sb = "D1.sb";
                ss1d1->sd1 = "D1.sd1";
                ss1d1->pb = ss1b;

                D3Ptr ss1d3 = new D3;
                ss1d3->sb = "D3.sb";
                ss1d3->sd3 = "D3.sd3";
                ss1d3->pb = ss1b;
                ss1d3->ice_collectable(true);
                
                BPtr ss2b = new B;
                ss2b->sb = "B.sb";
                ss2b->pb = ss1b;

                D1Ptr ss2d1 = new D1;
                ss2d1->sb = "D1.sb";
                ss2d1->sd1 = "D1.sd1";
                ss2d1->pb = ss2b;

                D3Ptr ss2d3 = new D3;
                ss2d3->sb = "D3.sb";
                ss2d3->sd3 = "D3.sd3";
                ss2d3->pb = ss2b;

                ss1d1->pd1 = ss2b;
                ss1d3->pd3 = ss2d1;

                ss2d1->pd1 = ss1d3;
                ss2d3->pd3 = ss1d1;

                SS1Ptr ss1 = new SS1;
                ss1->s.push_back(ss1b);
                ss1->s.push_back(ss1d1);
                ss1->s.push_back(ss1d3);

                SS2Ptr ss2 = new SS2;
                ss2->s.push_back(ss2b);
                ss2->s.push_back(ss2d1);
                ss2->s.push_back(ss2d3);
                
                ss1->ice_collectable(true);
                ss2->ice_collectable(true);

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
                BPtr ss1b = new B;
                ss1b->sb = "B.sb";
                ss1b->pb = ss1b;

                D1Ptr ss1d1 = new D1;
                ss1d1->sb = "D1.sb";
                ss1d1->sd1 = "D1.sd1";
                ss1d1->pb = ss1b;

                D3Ptr ss1d3 = new D3;
                ss1d3->sb = "D3.sb";
                ss1d3->sd3 = "D3.sd3";
                ss1d3->pb = ss1b;
                ss1d3->ice_collectable(true);

                BPtr ss2b = new B;
                ss2b->sb = "B.sb";
                ss2b->pb = ss1b;

                D1Ptr ss2d1 = new D1;
                ss2d1->sb = "D1.sb";
                ss2d1->sd1 = "D1.sd1";
                ss2d1->pb = ss2b;

                D3Ptr ss2d3 = new D3;
                ss2d3->sb = "D3.sb";
                ss2d3->sd3 = "D3.sd3";
                ss2d3->pb = ss2b;

                ss1d1->pd1 = ss2b;
                ss1d3->pd3 = ss2d1;

                ss2d1->pd1 = ss1d3;
                ss2d3->pd3 = ss1d1;

                SS1Ptr ss1 = new SS1;
                ss1->s.push_back(ss1b);
                ss1->s.push_back(ss1d1);
                ss1->s.push_back(ss1d3);

                SS2Ptr ss2 = new SS2;
                ss2->s.push_back(ss2b);
                ss2->s.push_back(ss2d1);
                ss2->s.push_back(ss2d3);
                
                ss1->ice_collectable(true);
                ss2->ice_collectable(true);

                CallbackPtr cb = new Callback;
                test->begin_sequenceTest(ss1, ss2,
                    newCallback_TestIntf_sequenceTest(cb, &Callback::response_sequenceTest, &Callback::exception));
                cb->check();
                ss = cb->rss3;
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
                D1Ptr d1 = new D1;
                d1->sb = s.str();
                d1->pb = d1;
                d1->sd1 = s.str();
                bin[i] = d1;
                d1->ice_collectable(true);
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
                D1Ptr d1 = D1Ptr::dynamicCast(b);
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
                D1Ptr d1 = new D1;
                d1->sb = s.str();
                d1->pb = d1;
                d1->sd1 = s.str();
                bin[i] = d1;
                d1->ice_collectable(true);
            }

            CallbackPtr cb = new Callback;
            test->begin_dictionaryTest(bin,
                newCallback_TestIntf_dictionaryTest(cb, &Callback::response_dictionaryTest, &Callback::exception));
            cb->check();
            bout = cb->obdict;
            r = cb->rbdict;

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
                D1Ptr d1 = D1Ptr::dynamicCast(b);
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
            test(e.ice_name() == "Test::BaseException");
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
        CallbackPtr cb = new Callback;
        test->begin_throwBaseAsBase(
            newCallback_TestIntf_throwBaseAsBase(cb, &Callback::response, &Callback::exception_throwBaseAsBase));
        cb->check();
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
            test(e.ice_name() == "Test::DerivedException");
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
        CallbackPtr cb = new Callback;
        test->begin_throwDerivedAsBase(
            newCallback_TestIntf_throwDerivedAsBase(cb, &Callback::response, &Callback::exception_throwDerivedAsBase));
        cb->check();
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
            test(e.ice_name() == "Test::DerivedException");
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
        CallbackPtr cb = new Callback;
        test->begin_throwDerivedAsDerived(
            newCallback_TestIntf_throwDerivedAsDerived(
                cb, &Callback::response, &Callback::exception_throwDerivedAsDerived));
        cb->check();
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
            test(e.ice_name() == "Test::BaseException");
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
        CallbackPtr cb = new Callback;
        test->begin_throwUnknownDerivedAsBase(
            newCallback_TestIntf_throwUnknownDerivedAsBase(
                cb, &Callback::response, &Callback::exception_throwUnknownDerivedAsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "forward-declared class... " << flush;
    {
        try
        {
            ForwardPtr f;
            test->useForward(f);
            test(f);
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "forward-declared class (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_useForward(
            newCallback_TestIntf_useForward(cb, &Callback::response_useForward, &Callback::exception));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "preserved classes... " << flush;
    try
    {
        //
        // Server knows the most-derived class PDerived.
        //
        PDerivedPtr pd = new PDerived;
        pd->pi = 3;
        pd->ps = "preserved";
        pd->pb = pd;
        pd->ice_collectable(true);

        PBasePtr r = test->exchangePBase(pd);
        PDerivedPtr p2 = PDerivedPtr::dynamicCast(r);
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
        PCUnknownPtr pu = new PCUnknown;
        pu->pi = 3;
        pu->pu = "preserved";

        PBasePtr r = test->exchangePBase(pu);
        PCUnknownPtr p2 = PCUnknownPtr::dynamicCast(r);
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
        PCDerivedPtr pcd = new PCDerived;
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
        pcd->ice_collectable(true);

        PBasePtr r = test->exchangePBase(pcd);
        PCDerivedPtr p2 = PCDerivedPtr::dynamicCast(r);
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
        CompactPCDerivedPtr pcd = new CompactPCDerived;
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
        pcd->ice_collectable(true);

        PBasePtr r = test->exchangePBase(pcd);
        CompactPCDerivedPtr p2 = CompactPCDerivedPtr::dynamicCast(r);
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
        PCDerived3Ptr pcd = new PCDerived3;
        pcd->pi = 3;
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        int i;
        for(i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = new PCDerived2;
            p2->pi = i;
            p2->pbs.push_back(0); // Nil reference. This slice should not have an indirection table.
            p2->pcd2 = i;
            pcd->pbs.push_back(p2);
        }
        pcd->pcd2 = pcd->pi;
        pcd->pcd3 = pcd->pbs[10];

        PBasePtr r = test->exchangePBase(pcd);
        PCDerived3Ptr p3 = PCDerived3Ptr::dynamicCast(r);
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!p3);
            test(PreservedPtr::dynamicCast(r));
            test(r->pi == 3);
        }
        else
        {
            test(p3);
            test(p3->pi == 3);
            for(i = 0; i < 300; ++i)
            {
                PCDerived2Ptr p2 = PCDerived2Ptr::dynamicCast(p3->pbs[i]);
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
        PDerivedPtr pd = new PDerived;
        pd->pi = 3;
        pd->ps = "preserved";
        pd->ps = "preserved";
        pd->pb = pd;
        pd->ice_collectable(true);
        
        CallbackPtr cb = new Callback;
        test->begin_exchangePBase(
            pd, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved1, &Callback::exception));
        cb->check();
    }

    {
        //
        // Server only knows the base (non-preserved) type, so the object is sliced.
        //
        PCUnknownPtr pu = new PCUnknown;
        pu->pi = 3;
        pu->pu = "preserved";

        CallbackPtr cb = new Callback;
        test->begin_exchangePBase(
            pu, newCallback_TestIntf_exchangePBase(cb, &Callback::response_preserved2, &Callback::exception));
        cb->check();
    }

    {
        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        PCDerivedPtr pcd = new PCDerived;
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
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
    }

    {
        //
        // Server only knows the intermediate type CompactPDerived. The object will be sliced to
        // CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
        //
        CompactPCDerivedPtr pcd = new CompactPCDerived;
        pcd->pi = 3;
        pcd->pbs.push_back(pcd);
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
    }

    {
        //
        // Send an object that will have multiple preserved slices in the server.
        // The object will be sliced to Preserved for the 1.0 encoding.
        //
        PCDerived3Ptr pcd = new PCDerived3;
        pcd->pi = 3;
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        int i;
        for(i = 0; i < 300; ++i)
        {
            PCDerived2Ptr p2 = new PCDerived2;
            p2->pi = i;
            p2->pbs.push_back(0); // Nil reference. This slice should not have an indirection table.
            p2->pcd2 = i;
            pcd->pbs.push_back(p2);
        }
        pcd->pcd2 = pcd->pi;
        pcd->pcd3 = pcd->pbs[10];

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
    }
    cout << "ok" << endl;

    cout << "garbage collection for preserved classes... " << flush;
    try
    {
        //
        // Register a factory in order to substitute our own subclass of PNode. This provides
        // an easy way to determine how many unmarshaled instances currently exist.
        //
        communicator->addObjectFactory(new NodeFactoryI, PNode::ice_staticId());

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
        communicator->addObjectFactory(new PreservedFactoryI, Preserved::ice_staticId());

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

    return test;
}
