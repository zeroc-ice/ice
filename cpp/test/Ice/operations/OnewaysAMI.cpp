// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

namespace
{

class CallbackBase : public Ice::LocalObject
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

typedef IceUtil::Handle<CallbackBase> CallbackBasePtr;

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
typedef IceUtil::Handle<Callback> CallbackPtr;

}

void
onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr& proxy)
{
    Test::MyClassPrxPtr p = ICE_UNCHECKED_CAST(Test::MyClassPrx, proxy->ice_oneway());

    {
        CallbackPtr cb = new Callback;
#ifdef ICE_CPP11_MAPPING
        p->ice_ping_async(
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
            p->ice_isA_async(Test::MyClass::ice_staticId(), 
                [&](bool)
                {
                    test(false);
                });
#else
            p->begin_ice_isA(Test::MyClass::ice_staticId());
#endif
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
    
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            p->ice_id_async( 
                [&](string)
                {
                    test(false);
                });
#else
            p->begin_ice_id();
#endif
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
    
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            p->ice_ids_async( 
                [&](vector<string>)
                {
                });
#else
            p->begin_ice_ids();
#endif
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }

    {
        CallbackPtr cb = new Callback;
#ifdef ICE_CPP11_MAPPING
        p->opVoid_async(
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
        CallbackPtr cb = new Callback;
#ifdef ICE_CPP11_MAPPING
        p->opIdempotent_async(
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
        CallbackPtr cb = new Callback;
#ifdef ICE_CPP11_MAPPING
        p->opNonmutating_async(
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
            p->opByte_async(Ice::Byte(0xff), Ice::Byte(0x0f),
                [](Ice::Byte, Ice::Byte)
                {
                    test(false);
                });
#else
            p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f));
#endif
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
#ifdef ICE_CPP11_COMPILER
#   ifndef ICE_CPP11_MAPPING
    {
        CallbackPtr cb = new Callback;
        p->begin_ice_ping(nullptr, 
                          [=](const Ice::Exception& ex){ cb->noException(ex); },
                          [=](bool sent){ cb->sent(sent); });
        cb->check();

    }
#   endif
    {
        try
        {
#   ifdef ICE_CPP11_MAPPING
            p->ice_isA_async(Test::MyClass::ice_staticId());
#   else
            p->begin_ice_isA(Test::MyClass::ice_staticId(), [=](bool){ test(false); });
#   endif
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
    
    {
        try
        {
#   ifdef ICE_CPP11_MAPPING
            p->ice_id_async();
#   else
            p->begin_ice_id([=](const string&){ test(false); });
#   endif
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
    
    {
        try
        {
#   ifdef ICE_CPP11_MAPPING
            p->ice_ids_async();
#   else
            p->begin_ice_ids([=](const Ice::StringSeq&){ test(false); });
#   endif
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }

#   ifndef ICE_CPP11_MAPPING
    {
        CallbackPtr cb = new Callback;
        p->begin_opVoid(nullptr, 
                        [=](const Ice::Exception& ex){ cb->noException(ex); },
                        [=](bool sent){ cb->sent(sent); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opIdempotent(nullptr, 
                              [=](const Ice::Exception& ex){ cb->noException(ex); },
                              [=](bool sent){ cb->sent(sent); });
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        p->begin_opNonmutating(nullptr, 
                               [=](const Ice::Exception& ex){ cb->noException(ex); },
                               [=](bool sent){ cb->sent(sent); });
        cb->check();
    }

    {
        try
        {
            p->begin_opByte(Ice::Byte(0xff), Ice::Byte(0x0f), [=](const Ice::Byte&, const Ice::Byte&){ test(false); });
            test(false);
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
#   endif
#endif
}
