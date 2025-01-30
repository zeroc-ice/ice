// Copyright (c) ZeroC, Inc.

#include "ServantLocatorI.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{
    class RouterI final : public Router
    {
    public:
        optional<ObjectPrx> getClientProxy(optional<bool>&, const Current&) const final { return nullopt; }

        [[nodiscard]] optional<ObjectPrx> getServerProxy(const Current& c) const final
        {
            return ObjectPrx(c.adapter->getCommunicator(), "dummy:tcp -h localhost -p 23456 -t 30000");
        }

        ObjectProxySeq addProxies(ObjectProxySeq, const Current&) final { return {}; }
    };
}

ServantLocatorI::ServantLocatorI() : _router(make_shared<RouterI>()) {}

ServantLocatorI::~ServantLocatorI() { test(_deactivated); }

ObjectPtr
ServantLocatorI::locate(const Current& current, shared_ptr<void>& cookie)
{
    test(!_deactivated);

    if (current.id.name == "router")
    {
        return _router;
    }

    test(current.id.category == "");
    test(current.id.name == "test");

    cookie = make_shared<Cookie>();

    return make_shared<TestI>();
}

void
ServantLocatorI::finished(const Current& current, const ObjectPtr&, const shared_ptr<void>& cookie)
{
    test(!_deactivated);
    if (current.id.name == "router")
    {
        return;
    }

    shared_ptr<Cookie> co = static_pointer_cast<Cookie>(cookie);
    test(co);
    test(co->message() == "blahblah");
}

void
ServantLocatorI::deactivate(string_view)
{
    test(!_deactivated);

    _deactivated = true;
}
