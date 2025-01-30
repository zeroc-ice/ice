// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

namespace
{
    class CallbackBase
    {
    public:
        CallbackBase() = default;

        virtual ~CallbackBase() = default;

        void check()
        {
            unique_lock lock(_mutex);
            _condition.wait(lock, [this] { return _called; });
            _called = false;
        }

    protected:
        void called()
        {
            lock_guard lock(_mutex);
            assert(!_called);
            _called = true;
            _condition.notify_one();
        }

    private:
        mutex _mutex;
        condition_variable _condition;
        bool _called{false};
    };

    class Callback : public CallbackBase
    {
    public:
        Callback() = default;

        void sent(bool) { called(); }

        void noException(const Ice::Exception&) { test(false); }
    };
    using CallbackPtr = std::shared_ptr<Callback>;
}

void
onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrx& proxy)
{
    Test::MyClassPrx p = proxy->ice_oneway();

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->ice_pingAsync(nullptr, [](exception_ptr) { test(false); }, [&](bool sent) { cb->sent(sent); });
        cb->check();
    }

    {
        try
        {
            p->ice_isAAsync(Test::MyClass::ice_staticId(), [&](bool) { test(false); });
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idAsync([&](const string&) { test(false); });
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idsAsync([&](const vector<string>&) {});
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->opVoidAsync(nullptr, [](exception_ptr) { test(false); }, [&](bool sent) { cb->sent(sent); });
        cb->check();
    }

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->opIdempotentAsync(nullptr, [](exception_ptr) { test(false); }, [&](bool sent) { cb->sent(sent); });
        cb->check();
    }

    {
        try
        {
            p->opByteAsync(uint8_t(0xff), uint8_t(0x0f), [](uint8_t, uint8_t) { test(false); });
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->ice_pingAsync(
            nullptr,
            [=](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Ice::Exception& ex)
                {
                    cb->noException(ex);
                }
            },
            [=](bool sent) { cb->sent(sent); });
        cb->check();
    }
    {
        try
        {
            p->ice_isAAsync(Test::MyClass::ice_staticId(), nullptr);
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idAsync(nullptr);
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idsAsync(nullptr);
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }
}
