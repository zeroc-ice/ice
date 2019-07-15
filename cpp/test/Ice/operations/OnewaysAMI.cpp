//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

// Work-around for anonymous namspace bug in xlclang++
#ifdef __ibmxl__
namespace OnewaysAMINamespace
#else
namespace
#endif
{

class CallbackBase
#ifndef ICE_CPP11_MAPPING
: public Ice::LocalObject
#endif
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
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
};

class Callback : public CallbackBase
{
public:

    Callback()
    {
    }

    void sent(bool)
    {
        called();
    }

    void noException(const Ice::Exception&)
    {
        test(false);
    }
};
ICE_DEFINE_PTR(CallbackPtr, Callback);

}

#ifdef __ibmxl__
using namespace OnewaysAMINamespace;
#endif

void
onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr& proxy)
{
    Test::MyClassPrxPtr p = ICE_UNCHECKED_CAST(Test::MyClassPrx, proxy->ice_oneway());

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->ice_pingAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
#else
        Ice::Callback_Object_ice_pingPtr callback =
            Ice::newCallback_Object_ice_ping(cb, &Callback::noException, &Callback::sent);
        p->begin_ice_ping(callback);
#endif
        cb->check();
    }

    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            p->ice_isAAsync(Test::MyClass::ice_staticId(),
                [&](bool)
                {
                    test(false);
                });
#else
            p->begin_ice_isA(Test::MyClass::ice_staticId());
#endif
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const Ice::TwowayOnlyException&)
#else
        catch(const IceUtil::IllegalArgumentException&)
#endif
        {
        }
    }

    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            p->ice_idAsync(
                [&](string)
                {
                    test(false);
                });
#else
            p->begin_ice_id();
#endif
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const Ice::TwowayOnlyException&)
#else
        catch(const IceUtil::IllegalArgumentException&)
#endif
        {
        }
    }

    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            p->ice_idsAsync(
                [&](vector<string>)
                {
                });
#else
            p->begin_ice_ids();
#endif
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const Ice::TwowayOnlyException&)
#else
        catch(const IceUtil::IllegalArgumentException&)
#endif
        {
        }
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opVoidAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
#else
        Test::Callback_MyClass_opVoidPtr callback =
            Test::newCallback_MyClass_opVoid(cb, &Callback::noException, &Callback::sent);
        p->begin_opVoid(callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opIdempotentAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
#else
        Test::Callback_MyClass_opIdempotentPtr callback =
            Test::newCallback_MyClass_opIdempotent(cb, &Callback::noException, &Callback::sent);
        p->begin_opIdempotent(callback);
#endif
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opNonmutatingAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
#else
        Test::Callback_MyClass_opNonmutatingPtr callback =
            Test::newCallback_MyClass_opNonmutating(cb, &Callback::noException, &Callback::sent);
        p->begin_opNonmutating(callback);
#endif
        cb->check();
    }

    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            p->opByteAsync(Ice::Byte(0xff), Ice::Byte(0x0f),
                [](Ice::Byte, Ice::Byte)
                {
                    test(false);
                });
#else
            p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f));
#endif
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const Ice::TwowayOnlyException&)
#else
        catch(const IceUtil::IllegalArgumentException&)
#endif
        {
        }
    }
#ifdef ICE_CPP11_MAPPING
    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
        p->ice_pingAsync(nullptr,
                        [=](exception_ptr e)
                        {
                            try
                            {
                                rethrow_exception(e);
                            }
                            catch(const Ice::Exception& ex)
                            {
                                cb->noException(ex);
                            }
                        },
                        [=](bool sent)
                        {
                            cb->sent(sent);
                        });
        cb->check();

    }
    {
        try
        {
            p->ice_isAAsync(Test::MyClass::ice_staticId());
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const Ice::TwowayOnlyException&)
#else
        catch(const IceUtil::IllegalArgumentException&)
#endif
        {
        }
    }

    {
        try
        {
            p->ice_idAsync();
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const Ice::TwowayOnlyException&)
#else
        catch(const IceUtil::IllegalArgumentException&)
#endif
        {
        }
    }

    {
        try
        {
            p->ice_idsAsync();
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const Ice::TwowayOnlyException&)
#else
        catch(const IceUtil::IllegalArgumentException&)
#endif
        {
        }
    }
#endif
}
