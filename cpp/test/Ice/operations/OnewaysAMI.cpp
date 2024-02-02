//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

namespace
{

class CallbackBase
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

void
onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr& proxy)
{
    Test::MyClassPrxPtr p = ICE_UNCHECKED_CAST(Test::MyClassPrx, proxy->ice_oneway());

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
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
        cb->check();
    }

    {
        try
        {
            p->ice_isAAsync(Test::MyClass::ice_staticId(),
                [&](bool)
                {
                    test(false);
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idAsync(
                [&](string)
                {
                    test(false);
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idsAsync(
                [&](vector<string>)
                {
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
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
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
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
        cb->check();
    }

    {
        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
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
        cb->check();
    }

    {
        try
        {
            p->opByteAsync(Ice::Byte(0xff), Ice::Byte(0x0f),
                [](Ice::Byte, Ice::Byte)
                {
                    test(false);
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

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
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idAsync();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idsAsync();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }
}
