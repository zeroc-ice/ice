// Copyright (c) ZeroC, Inc.

#include "Instance.h"
#include "../Ice/InstrumentationI.h"
#include "InstrumentationI.h"
#include "SessionRouterI.h"

using namespace std;
using namespace Glacier2;

Glacier2::Instance::Instance(
    shared_ptr<Ice::Communicator> communicator,
    Ice::ObjectAdapterPtr clientAdapter,
    Ice::ObjectAdapterPtr serverAdapter)
    : _communicator(std::move(communicator)),
      _properties(_communicator->getProperties()),
      _logger(_communicator->getLogger()),
      _clientAdapter(std::move(clientAdapter)),
      _serverAdapter(std::move(serverAdapter)),
      _proxyVerifier(make_shared<ProxyVerifier>(_communicator))
{
    //
    // If an Ice metrics observer is setup on the communicator, also enable metrics for Glacier2.
    //
    auto o = dynamic_pointer_cast<IceInternal::CommunicatorObserverI>(_communicator->getObserver());
    if (o)
    {
        const_cast<shared_ptr<Instrumentation::RouterObserver>&>(_observer) =
            make_shared<RouterObserverI>(o->getFacet(), _properties->getIceProperty("Glacier2.InstanceName"));
    }
}

void
Glacier2::Instance::destroy()
{
    _sessionRouter = nullptr;
}

void
Glacier2::Instance::setSessionRouter(shared_ptr<SessionRouterI> sessionRouter)
{
    assert(_sessionRouter == nullptr);
    _sessionRouter = std::move(sessionRouter);
}
