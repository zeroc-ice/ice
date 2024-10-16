//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        RouterI(Test::TestHelper* helper) : _routerPort(helper->getTestPort(5)) {}
        optional<ObjectPrx> getClientProxy(optional<bool>&, const Current&) const final { return nullopt; }

        optional<ObjectPrx> getServerProxy(const Current& c) const final
        {
            return ObjectPrx(c.adapter->getCommunicator(), "dummy:tcp -h localhost -p " + to_string(_routerPort) + " -t 30000");
        }

        ObjectProxySeq addProxies(ObjectProxySeq, const Current&) final { return ObjectProxySeq(); }

    private:
        int _routerPort;
    };
}

ServantLocatorI::ServantLocatorI(Test::TestHelper* helper) :
    _deactivated(false),
    _router(make_shared<RouterI>(helper)) {}

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
ServantLocatorI::deactivate(const string&)
{
    test(!_deactivated);

    _deactivated = true;
}
