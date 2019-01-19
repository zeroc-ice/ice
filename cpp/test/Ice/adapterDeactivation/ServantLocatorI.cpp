//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <ServantLocatorI.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{

class RouterI : public Ice::Router
{
public:

    RouterI() : _nextPort(23456)
    {
    }

    virtual Ice::ObjectPrxPtr
    getClientProxy(IceUtil::Optional<bool>&, const Ice::Current&) const
    {
        return ICE_NULLPTR;
    }

    virtual Ice::ObjectPrxPtr
    getServerProxy(const Ice::Current& c) const
    {
        ostringstream os;
        os << "dummy:tcp -h localhost -p " << _nextPort++ << " -t 30000";
        return c.adapter->getCommunicator()->stringToProxy(os.str());
    }

    virtual Ice::ObjectProxySeq
#ifdef ICE_CPP11_MAPPING
    addProxies(Ice::ObjectProxySeq, const Ice::Current&)
#else
    addProxies(const Ice::ObjectProxySeq&, const Ice::Current&)
#endif
    {
        return Ice::ObjectProxySeq();
    }

private:

    mutable int _nextPort;
};

}

ServantLocatorI::ServantLocatorI() : _deactivated(false), _router(ICE_MAKE_SHARED(RouterI))
{
}

ServantLocatorI::~ServantLocatorI()
{
    test(_deactivated);
}

Ice::ObjectPtr
#ifdef ICE_CPP11_MAPPING
ServantLocatorI::locate(const Ice::Current& current, std::shared_ptr<void>& cookie)
#else
ServantLocatorI::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
#endif
{
    test(!_deactivated);

    if(current.id.name == "router")
    {
        return _router;
    }

    test(current.id.category == "");
    test(current.id.name == "test");

    cookie = ICE_MAKE_SHARED(CookieI);

    return ICE_MAKE_SHARED(TestI);
}

void
#ifdef ICE_CPP11_MAPPING
ServantLocatorI::finished(const Ice::Current& current, const Ice::ObjectPtr&, const std::shared_ptr<void>& cookie)
#else
ServantLocatorI::finished(const Ice::Current& current, const Ice::ObjectPtr&, const Ice::LocalObjectPtr& cookie)
#endif
{
    test(!_deactivated);
    if(current.id.name == "router")
    {
        return;
    }

#ifdef ICE_CPP11_MAPPING
    shared_ptr<CookieI> co = static_pointer_cast<CookieI>(cookie);
#else
    CookiePtr co = CookiePtr::dynamicCast(cookie);
#endif
    test(co);
    test(co->message() == "blahblah");
}

void
ServantLocatorI::deactivate(const string&)
{
    test(!_deactivated);

    _deactivated = true;
}
