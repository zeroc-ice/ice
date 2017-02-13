// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    response()
    {
        test(false);
    }

    void
    exception_baseAsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const Base& b)
        {
            test(b.b == "Base.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_unknownDerivedAsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownDerived.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_knownDerivedAsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(k.ice_name() =="Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_knownDerivedAsKnownDerived(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(k.ice_name() =="Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_unknownIntermediateAsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownIntermediate.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_knownIntermediateAsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(ki.ice_name() =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_knownMostDerivedAsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_name() =="Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_knownIntermediateAsKnownIntermediate(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(ki.ice_name() =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_knownMostDerivedAsKnownMostDerived(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_name() =="Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_knownMostDerivedAsKnownIntermediate(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_name() =="Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_unknownMostDerived1AsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(string(ki.ice_name()) =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_unknownMostDerived1AsKnownIntermediate(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(ki.ice_name() =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    void
    exception_unknownMostDerived2AsBase(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownMostDerived2.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }
};
typedef IceUtil::Handle<Callback> CallbackPtr;

class RelayI : public Relay
{
    virtual void knownPreservedAsBase(const ::Ice::Current&)
    {
        KnownPreservedDerived ex;
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    virtual void knownPreservedAsKnownPreserved(const ::Ice::Current&)
    {
        KnownPreservedDerived ex;
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    virtual void unknownPreservedAsBase(const ::Ice::Current&)
    {
        Preserved2 ex;
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new PreservedClass("bc", "pc");
        ex.p2 = ex.p1;
        throw ex;
    }

    virtual void unknownPreservedAsKnownPreserved(const ::Ice::Current&)
    {
        Preserved2 ex;
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new PreservedClass("bc", "pc");
        ex.p2 = ex.p1;
        throw ex;
    }
};

TestIntfPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectPrx obj = communicator->stringToProxy("Test:default -p 12010");
    TestIntfPrx test = TestIntfPrx::checkedCast(obj);

    cout << "base... " << flush;
    {
        try
        {
            test->baseAsBase();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "Base.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_baseAsBase(
            newCallback_TestIntf_baseAsBase(cb, &Callback::response, &Callback::exception_baseAsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "slicing of unknown derived... " << flush;
    {
        try
        {
            test->unknownDerivedAsBase();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownDerived.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown derived (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_unknownDerivedAsBase(
            newCallback_TestIntf_unknownDerivedAsBase(cb, &Callback::response,
                                                      &Callback::exception_unknownDerivedAsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as base... " << flush;
    {
        try
        {
            test->knownDerivedAsBase();
            test(false);
        }
        catch(const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(k.ice_name() =="Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_knownDerivedAsBase(
            newCallback_TestIntf_knownDerivedAsBase(cb, &Callback::response, &Callback::exception_knownDerivedAsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as derived... " << flush;
    {
        try
        {
            test->knownDerivedAsKnownDerived();
            test(false);
        }
        catch(const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(k.ice_name() =="Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as derived (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_knownDerivedAsKnownDerived(
            newCallback_TestIntf_knownDerivedAsKnownDerived(cb, &Callback::response, 
                                                            &Callback::exception_knownDerivedAsKnownDerived));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "slicing of unknown intermediate as base... " << flush;
    {
        try
        {
            test->unknownIntermediateAsBase();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownIntermediate.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown intermediate as base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_unknownIntermediateAsBase(
            newCallback_TestIntf_unknownIntermediateAsBase(cb, &Callback::response, 
                                                           &Callback::exception_unknownIntermediateAsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "slicing of known intermediate as base... " << flush;
    {
        try
        {
            test->knownIntermediateAsBase();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(ki.ice_name() =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known intermediate as base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_knownIntermediateAsBase(
            newCallback_TestIntf_knownIntermediateAsBase(cb, &Callback::response, 
                                                         &Callback::exception_knownIntermediateAsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "slicing of known most derived as base... " << flush;
    {
        try
        {
            test->knownMostDerivedAsBase();
            test(false);
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_name() =="Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known most derived as base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_knownMostDerivedAsBase(
            newCallback_TestIntf_knownMostDerivedAsBase(cb, &Callback::response, 
                                                        &Callback::exception_knownMostDerivedAsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "non-slicing of known intermediate as intermediate... " << flush;
    {
        try
        {
            test->knownIntermediateAsKnownIntermediate();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(ki.ice_name() =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known intermediate as intermediate (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_knownIntermediateAsKnownIntermediate(
            newCallback_TestIntf_knownIntermediateAsKnownIntermediate(cb, &Callback::response, 
                                                        &Callback::exception_knownIntermediateAsKnownIntermediate));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived exception as intermediate... " << flush;
    {
        try
        {
            test->knownMostDerivedAsKnownIntermediate();
            test(false);
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_name() =="Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as intermediate (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_knownMostDerivedAsKnownIntermediate(
            newCallback_TestIntf_knownMostDerivedAsKnownIntermediate(cb, &Callback::response, 
                                                        &Callback::exception_knownMostDerivedAsKnownIntermediate));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as most derived... " << flush;
    {
        try
        {
            test->knownMostDerivedAsKnownMostDerived();
            test(false);
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_name() =="Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as most derived (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_knownMostDerivedAsKnownMostDerived(
            newCallback_TestIntf_knownMostDerivedAsKnownMostDerived(cb, &Callback::response, 
                                                        &Callback::exception_knownMostDerivedAsKnownMostDerived));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as base... " << flush;
    {
        try
        {
            test->unknownMostDerived1AsBase();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(ki.ice_name() =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_unknownMostDerived1AsBase(
            newCallback_TestIntf_unknownMostDerived1AsBase(cb, &Callback::response, 
                                                           &Callback::exception_unknownMostDerived1AsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as intermediate... " << flush;
    {
        try
        {
            test->unknownMostDerived1AsKnownIntermediate();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(ki.ice_name() =="Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as intermediate (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_unknownMostDerived1AsKnownIntermediate(
            newCallback_TestIntf_unknownMostDerived1AsKnownIntermediate(cb, &Callback::response, 
                                                         &Callback::exception_unknownMostDerived1AsKnownIntermediate));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, unknown intermediate as base... " << flush;
    {
        try
        {
            test->unknownMostDerived2AsBase();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownMostDerived2.b");
            test(b.ice_name() =="Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, unknown intermediate as base (AMI)... " << flush;
    {
        CallbackPtr cb = new Callback;
        test->begin_unknownMostDerived2AsBase(
            newCallback_TestIntf_unknownMostDerived2AsBase(cb, &Callback::response, 
                                                         &Callback::exception_unknownMostDerived2AsBase));
        cb->check();
    }
    cout << "ok" << endl;

    cout << "unknown most derived in compact format... " << flush;
    {
        try
        {
            test->unknownMostDerived2AsBaseCompact();
            test(false);
        }
        catch(const Base&)
        {
            //
            // For the 1.0 encoding, the unknown exception is sliced to Base.
            //
            test(test->ice_getEncodingVersion() == Ice::Encoding_1_0);
        }
        catch(const Ice::UnknownUserException&)
        {
            //
            // An UnknownUserException is raised for the compact format because the
            // most-derived type is unknown and the exception cannot be sliced.
            //
            test(test->ice_getEncodingVersion() != Ice::Encoding_1_0);
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "preserved exceptions... " << flush;
    {
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("Relay", "default");
        RelayPrx relay = RelayPrx::uncheckedCast(adapter->addWithUUID(new RelayI));
        adapter->activate();

        try
        {
            test->relayKnownPreservedAsBase(relay);
            test(false);
        }
        catch(const KnownPreservedDerived& ex)
        {
            test(ex.b == "base");
            test(ex.kp == "preserved");
            test(ex.kpd == "derived");
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(...)
        {
            test(false);
        }

        try
        {
            test->relayKnownPreservedAsKnownPreserved(relay);
            test(false);
        }
        catch(const KnownPreservedDerived& ex)
        {
            test(ex.b == "base");
            test(ex.kp == "preserved");
            test(ex.kpd == "derived");
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(...)
        {
            test(false);
        }

        try
        {
            test->relayUnknownPreservedAsBase(relay);
            test(false);
        }
        catch(const Preserved2& ex)
        {
            test(ex.b == "base");
            test(ex.kp == "preserved");
            test(ex.kpd == "derived");
            test(ex.p1->ice_id() == PreservedClass::ice_staticId());
            PreservedClassPtr pc = PreservedClassPtr::dynamicCast(ex.p1);
            test(pc->bc == "bc");
            test(pc->pc == "pc");
            test(ex.p2 == ex.p1);
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(const KnownPreservedDerived& ex)
        {
            //
            // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
            //
            test(test->ice_getEncodingVersion() == Ice::Encoding_1_0);
            test(ex.b == "base");
            test(ex.kp == "preserved");
            test(ex.kpd == "derived");
        }
        catch(...)
        {
            test(false);
        }

        try
        {
            test->relayUnknownPreservedAsKnownPreserved(relay);
            test(false);
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(const Preserved2& ex)
        {
            test(ex.b == "base");
            test(ex.kp == "preserved");
            test(ex.kpd == "derived");
            test(ex.p1->ice_id() == PreservedClass::ice_staticId());
            PreservedClassPtr pc = PreservedClassPtr::dynamicCast(ex.p1);
            test(pc->bc == "bc");
            test(pc->pc == "pc");
            test(ex.p2 == ex.p1);
        }
        catch(const KnownPreservedDerived& ex)
        {
            //
            // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
            //
            test(test->ice_getEncodingVersion() == Ice::Encoding_1_0);
            test(ex.b == "base");
            test(ex.kp == "preserved");
            test(ex.kpd == "derived");
        }
        catch(...)
        {
            test(false);
        }

        adapter->destroy();
    }
    cout << "ok" << endl;

    return test;
}
