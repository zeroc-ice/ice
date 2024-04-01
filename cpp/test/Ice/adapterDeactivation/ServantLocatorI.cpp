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
        RouterI() : _nextPort(23456) {}

        optional<ObjectPrx> getClientProxy(optional<bool>&, const Current&) const final { return nullopt; }

        optional<ObjectPrx> getServerProxy(const Current& c) const final
        {
            ostringstream os;
            os << "dummy:tcp -h localhost -p " << _nextPort++ << " -t 30000";
            return c.adapter->getCommunicator()->stringToProxy(os.str());
        }

        ObjectProxySeq addProxies(ObjectProxySeq, const Current&) final { return ObjectProxySeq(); }

    private:
        mutable int _nextPort;
    };
}

ServantLocatorI::ServantLocatorI() : _deactivated(false), _router(make_shared<RouterI>()) {}

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
