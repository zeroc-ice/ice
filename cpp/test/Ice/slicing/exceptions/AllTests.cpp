//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <ClientPrivate.h>

using namespace std;
using namespace Test;

#ifndef ICE_CPP11_MAPPING
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
            test(b.ice_id() =="::Test::Base");
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
            test(b.ice_id() == "::Test::Base");
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
            test(k.ice_id() == "::Test::KnownDerived");
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
            test(k.ice_id() == "::Test::KnownDerived");
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
            test(b.ice_id() == "::Test::Base");
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
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
            test(kmd.ice_id() == "::Test::KnownMostDerived");
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
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
            test(kmd.ice_id() == "::Test::KnownMostDerived");
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
            test(kmd.ice_id() == "::Test::KnownMostDerived");
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
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
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }
};
typedef IceUtil::Handle<Callback> CallbackPtr;
#endif

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
        ex.p1 = ICE_MAKE_SHARED(PreservedClass, "bc", "pc");
        ex.p2 = ex.p1;
        throw ex;
    }

    virtual void unknownPreservedAsKnownPreserved(const ::Ice::Current&)
    {
        Preserved2 ex;
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = ICE_MAKE_SHARED(PreservedClass, "bc", "pc");
        ex.p2 = ex.p1;
        throw ex;
    }
};

TestIntfPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    Ice::ObjectPrxPtr obj = communicator->stringToProxy("Test:" + helper->getTestEndpoint());
    TestIntfPrxPtr test = ICE_CHECKED_CAST(TestIntfPrx, obj);

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
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->baseAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "Base.b");
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }

#else
        CallbackPtr cb = new Callback;
        test->begin_baseAsBase(
            newCallback_TestIntf_baseAsBase(cb, &Callback::response, &Callback::exception_baseAsBase));
        cb->check();
#endif
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
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown derived (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->unknownDerivedAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownDerived.b");
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_unknownDerivedAsBase(
            newCallback_TestIntf_unknownDerivedAsBase(cb, &Callback::response,
                                                      &Callback::exception_unknownDerivedAsBase));
        cb->check();
#endif
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
            test(k.ice_id() == "::Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->knownDerivedAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(k.ice_id() == "::Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_knownDerivedAsBase(
            newCallback_TestIntf_knownDerivedAsBase(cb, &Callback::response, &Callback::exception_knownDerivedAsBase));
        cb->check();
#endif
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
            test(k.ice_id() == "::Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known derived as derived (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->knownDerivedAsKnownDerivedAsync();
        try
        {
            result.get();
        }
        catch(const KnownDerived& k)
        {
            test(k.b == "KnownDerived.b");
            test(k.kd == "KnownDerived.kd");
            test(k.ice_id() == "::Test::KnownDerived");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_knownDerivedAsKnownDerived(
            newCallback_TestIntf_knownDerivedAsKnownDerived(cb, &Callback::response,
                                                            &Callback::exception_knownDerivedAsKnownDerived));
        cb->check();
#endif
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
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown intermediate as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->unknownIntermediateAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownIntermediate.b");
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_unknownIntermediateAsBase(
            newCallback_TestIntf_unknownIntermediateAsBase(cb, &Callback::response,
                                                           &Callback::exception_unknownIntermediateAsBase));
        cb->check();
#endif
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known intermediate as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->knownIntermediateAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_knownIntermediateAsBase(
            newCallback_TestIntf_knownIntermediateAsBase(cb, &Callback::response,
                                                         &Callback::exception_knownIntermediateAsBase));
        cb->check();
#endif
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
            test(kmd.ice_id() == "::Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of known most derived as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->knownMostDerivedAsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_id() == "::Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_knownMostDerivedAsBase(
            newCallback_TestIntf_knownMostDerivedAsBase(cb, &Callback::response,
                                                        &Callback::exception_knownMostDerivedAsBase));
        cb->check();
#endif
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known intermediate as intermediate (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->knownIntermediateAsKnownIntermediateAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "KnownIntermediate.b");
            test(ki.ki == "KnownIntermediate.ki");
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_knownIntermediateAsKnownIntermediate(
            newCallback_TestIntf_knownIntermediateAsKnownIntermediate(cb, &Callback::response,
                                                        &Callback::exception_knownIntermediateAsKnownIntermediate));
        cb->check();
#endif
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
            test(kmd.ice_id() == "::Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as intermediate (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->knownMostDerivedAsKnownIntermediateAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_id() == "::Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_knownMostDerivedAsKnownIntermediate(
            newCallback_TestIntf_knownMostDerivedAsKnownIntermediate(cb, &Callback::response,
                                                        &Callback::exception_knownMostDerivedAsKnownIntermediate));
        cb->check();
#endif
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
            test(kmd.ice_id() == "::Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "non-slicing of known most derived as most derived (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->knownMostDerivedAsKnownMostDerivedAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownMostDerived& kmd)
        {
            test(kmd.b == "KnownMostDerived.b");
            test(kmd.ki == "KnownMostDerived.ki");
            test(kmd.kmd == "KnownMostDerived.kmd");
            test(kmd.ice_id() == "::Test::KnownMostDerived");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_knownMostDerivedAsKnownMostDerived(
            newCallback_TestIntf_knownMostDerivedAsKnownMostDerived(cb, &Callback::response,
                                                        &Callback::exception_knownMostDerivedAsKnownMostDerived));
        cb->check();
#endif
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->unknownMostDerived1AsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_unknownMostDerived1AsBase(
            newCallback_TestIntf_unknownMostDerived1AsBase(cb, &Callback::response,
                                                           &Callback::exception_unknownMostDerived1AsBase));
        cb->check();
#endif
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
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, known intermediate as intermediate (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->unknownMostDerived1AsKnownIntermediateAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const KnownIntermediate& ki)
        {
            test(ki.b == "UnknownMostDerived1.b");
            test(ki.ki == "UnknownMostDerived1.ki");
            test(ki.ice_id() == "::Test::KnownIntermediate");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_unknownMostDerived1AsKnownIntermediate(
            newCallback_TestIntf_unknownMostDerived1AsKnownIntermediate(cb, &Callback::response,
                                                         &Callback::exception_unknownMostDerived1AsKnownIntermediate));
        cb->check();
#endif
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
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "slicing of unknown most derived, unknown intermediate as base (AMI)... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto result = test->unknownMostDerived2AsBaseAsync();
        try
        {
            result.get();
            test(false);
        }
        catch(const Base& b)
        {
            test(b.b == "UnknownMostDerived2.b");
            test(b.ice_id() == "::Test::Base");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        test->begin_unknownMostDerived2AsBase(
            newCallback_TestIntf_unknownMostDerived2AsBase(cb, &Callback::response,
                                                         &Callback::exception_unknownMostDerived2AsBase));
        cb->check();
#endif
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
    try
    {
        test->unknownPreservedAsBase();
        test(false);
    }
    catch(const Base& ex)
    {
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!ex.ice_getSlicedData());
        }
        else
        {
            Ice::SlicedDataPtr slicedData = ex.ice_getSlicedData();
            test(slicedData);
            test(slicedData->slices.size() == 2);
            test(slicedData->slices[1]->typeId == "::Test::SPreserved1");
            test(slicedData->slices[0]->typeId == "::Test::SPreserved2");
        }
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        test->unknownPreservedAsKnownPreserved();
        test(false);
    }
    catch(const KnownPreserved& ex)
    {
        test(ex.kp == "preserved");
        if(test->ice_getEncodingVersion() == Ice::Encoding_1_0)
        {
            test(!ex.ice_getSlicedData());
        }
        else
        {
            Ice::SlicedDataPtr slicedData = ex.ice_getSlicedData();
            test(slicedData);
            test(slicedData->slices.size() == 2);
            test(slicedData->slices[1]->typeId == "::Test::SPreserved1");
            test(slicedData->slices[0]->typeId == "::Test::SPreserved2");
        }
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    catch(...)
    {
        test(false);
    }

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
    RelayPrxPtr relay = ICE_UNCHECKED_CAST(RelayPrx, adapter->addWithUUID(ICE_MAKE_SHARED(RelayI)));
    adapter->activate();
    test->ice_getConnection()->setAdapter(adapter);
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
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
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
        PreservedClassPtr pc = ICE_DYNAMIC_CAST(PreservedClass, ex.p1);
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
        PreservedClassPtr pc = ICE_DYNAMIC_CAST(PreservedClass, ex.p1);
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
    cout << "ok" << endl;

    return test;
}
