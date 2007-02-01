// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

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

    bool check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            if(!timedWait(IceUtil::Time::seconds(5)))
            {
                return false;
            }
        }
        _called = false;
        return true;
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

class AMI_Test_baseAsBaseI : public AMI_TestIntf_baseAsBase, public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_baseAsBaseI> AMI_Test_baseAsBaseIPtr;

class AMI_Test_unknownDerivedAsBaseI : public AMI_TestIntf_unknownDerivedAsBase, public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_unknownDerivedAsBaseI> AMI_Test_unknownDerivedAsBaseIPtr;

class AMI_Test_knownDerivedAsBaseI : public AMI_TestIntf_knownDerivedAsBase, public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_knownDerivedAsBaseI> AMI_Test_knownDerivedAsBaseIPtr;

class AMI_Test_knownDerivedAsKnownDerivedI : public AMI_TestIntf_knownDerivedAsKnownDerived, public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_knownDerivedAsKnownDerivedI> AMI_Test_knownDerivedAsKnownDerivedIPtr;

class AMI_Test_unknownIntermediateAsBaseI : public AMI_TestIntf_unknownIntermediateAsBase, public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_unknownIntermediateAsBaseI> AMI_Test_unknownIntermediateAsBaseIPtr;

class AMI_Test_knownIntermediateAsBaseI : public AMI_TestIntf_knownIntermediateAsBase, public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_knownIntermediateAsBaseI> AMI_Test_knownIntermediateAsBaseIPtr;

class AMI_Test_knownMostDerivedAsBaseI : public AMI_TestIntf_knownMostDerivedAsBase,
                                         public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_knownMostDerivedAsBaseI> AMI_Test_knownMostDerivedAsBaseIPtr;

class AMI_Test_knownIntermediateAsKnownIntermediateI : public AMI_TestIntf_knownIntermediateAsKnownIntermediate,
                                                       public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_knownIntermediateAsKnownIntermediateI> AMI_Test_knownIntermediateAsKnownIntermediateIPtr;

class AMI_Test_knownMostDerivedAsKnownMostDerivedI : public AMI_TestIntf_knownMostDerivedAsKnownMostDerived,
                                                      public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_knownMostDerivedAsKnownMostDerivedI> AMI_Test_knownMostDerivedAsKnownMostDerivedIPtr;

class AMI_Test_knownMostDerivedAsKnownIntermediateI : public AMI_TestIntf_knownMostDerivedAsKnownIntermediate,
                                                      public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_knownMostDerivedAsKnownIntermediateI> AMI_Test_knownMostDerivedAsKnownIntermediateIPtr;

class AMI_Test_unknownMostDerived1AsBaseI : public AMI_TestIntf_unknownMostDerived1AsBase,
                                            public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_unknownMostDerived1AsBaseI> AMI_Test_unknownMostDerived1AsBaseIPtr;

class AMI_Test_unknownMostDerived1AsKnownIntermediateI : public AMI_TestIntf_unknownMostDerived1AsKnownIntermediate,
                                                         public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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
};

typedef IceUtil::Handle<AMI_Test_unknownMostDerived1AsKnownIntermediateI>
            AMI_Test_unknownMostDerived1AsKnownIntermediateIPtr;

class AMI_Test_unknownMostDerived2AsBaseI : public AMI_TestIntf_unknownMostDerived2AsBase,
                                            public CallbackBase
{
    virtual void
    ice_response()
    {
        test(false);
    }

    virtual void
    ice_exception(const Ice::Exception& exc)
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

typedef IceUtil::Handle<AMI_Test_unknownMostDerived2AsBaseI> AMI_Test_unknownMostDerived2AsBaseIPtr;

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
        AMI_Test_baseAsBaseIPtr cb = new AMI_Test_baseAsBaseI;
        test->baseAsBase_async(cb);
        test(cb->check());
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
        AMI_Test_unknownDerivedAsBaseIPtr cb = new AMI_Test_unknownDerivedAsBaseI;
        test->unknownDerivedAsBase_async(cb);
        test(cb->check());
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
        AMI_Test_knownDerivedAsBaseIPtr cb = new AMI_Test_knownDerivedAsBaseI;
        test->knownDerivedAsBase_async(cb);
        test(cb->check());
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
        AMI_Test_knownDerivedAsKnownDerivedIPtr cb = new AMI_Test_knownDerivedAsKnownDerivedI;
        test->knownDerivedAsKnownDerived_async(cb);
        test(cb->check());
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
        AMI_Test_unknownIntermediateAsBaseIPtr cb = new AMI_Test_unknownIntermediateAsBaseI;
        test->unknownIntermediateAsBase_async(cb);
        test(cb->check());
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
        AMI_Test_knownIntermediateAsBaseIPtr cb = new AMI_Test_knownIntermediateAsBaseI;
        test->knownIntermediateAsBase_async(cb);
        test(cb->check());
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
        AMI_Test_knownMostDerivedAsBaseIPtr cb = new AMI_Test_knownMostDerivedAsBaseI;
        test->knownMostDerivedAsBase_async(cb);
        test(cb->check());
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
        AMI_Test_knownIntermediateAsKnownIntermediateIPtr cb = new AMI_Test_knownIntermediateAsKnownIntermediateI;
        test->knownIntermediateAsKnownIntermediate_async(cb);
        test(cb->check());
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
        AMI_Test_knownMostDerivedAsKnownIntermediateIPtr cb = new AMI_Test_knownMostDerivedAsKnownIntermediateI;
        test->knownMostDerivedAsKnownIntermediate_async(cb);
        test(cb->check());
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
        AMI_Test_knownMostDerivedAsKnownMostDerivedIPtr cb = new AMI_Test_knownMostDerivedAsKnownMostDerivedI;
        test->knownMostDerivedAsKnownMostDerived_async(cb);
        test(cb->check());
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
        AMI_Test_unknownMostDerived1AsBaseIPtr cb = new AMI_Test_unknownMostDerived1AsBaseI;
        test->unknownMostDerived1AsBase_async(cb);
        test(cb->check());
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
        AMI_Test_unknownMostDerived1AsKnownIntermediateIPtr cb = new AMI_Test_unknownMostDerived1AsKnownIntermediateI;
        test->unknownMostDerived1AsKnownIntermediate_async(cb);
        test(cb->check());
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
        AMI_Test_unknownMostDerived2AsBaseIPtr cb = new AMI_Test_unknownMostDerived2AsBaseI;
        test->unknownMostDerived2AsBase_async(cb);
        test(cb->check());
    }
    cout << "ok" << endl;

    return test;
}
