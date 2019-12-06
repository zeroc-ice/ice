//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/SessionRouterI.h>
#include <Glacier2/Instance.h>
#include <Glacier2/InstrumentationI.h>
#include <Ice/InstrumentationI.h>

using namespace std;
using namespace Glacier2;

namespace
{

const string serverSleepTime = "Glacier2.Server.SleepTime";
const string clientSleepTime = "Glacier2.Client.SleepTime";
const string serverBuffered = "Glacier2.Server.Buffered";
const string clientBuffered = "Glacier2.Client.Buffered";

}

Glacier2::Instance::Instance(shared_ptr<Ice::Communicator> communicator, shared_ptr<Ice::ObjectAdapter> clientAdapter,
                   shared_ptr<Ice::ObjectAdapter> serverAdapter) :
    _communicator(move(communicator)),
    _properties(_communicator->getProperties()),
    _logger(_communicator->getLogger()),
    _clientAdapter(move(clientAdapter)),
    _serverAdapter(move(serverAdapter)),
    _proxyVerifier(make_shared<ProxyVerifier>(_communicator))
{
    if(_properties->getPropertyAsIntWithDefault(serverBuffered, 0) > 0)
    {
        auto sleepTime = chrono::milliseconds(_properties->getPropertyAsInt(serverSleepTime));
        const_cast<shared_ptr<RequestQueueThread>&>(_serverRequestQueueThread) = make_shared<RequestQueueThread>(sleepTime);
    }

    if(_properties->getPropertyAsIntWithDefault(clientBuffered, 0) > 0)
    {
        auto sleepTime = chrono::milliseconds(_properties->getPropertyAsInt(clientSleepTime));
        const_cast<shared_ptr<RequestQueueThread>&>(_clientRequestQueueThread) = make_shared<RequestQueueThread>(sleepTime);
    }

    //
    // If an Ice metrics observer is setup on the communicator, also enable metrics for Glacier2.
    //
    auto o = dynamic_pointer_cast<IceInternal::CommunicatorObserverI>(_communicator->getObserver());
    if(o)
    {
        const_cast<shared_ptr<Instrumentation::RouterObserver>&>(_observer) =
            make_shared<RouterObserverI>(o->getFacet(),
                                         _properties->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2"));
    }
}

void
Glacier2::Instance::destroy()
{
    if(_clientRequestQueueThread)
    {
        _clientRequestQueueThread->destroy();
    }

    if(_serverRequestQueueThread)
    {
        _serverRequestQueueThread->destroy();
    }

    _sessionRouter = nullptr;
}

void
Glacier2::Instance::setSessionRouter(shared_ptr<SessionRouterI> sessionRouter)
{
    assert(_sessionRouter == nullptr);
    _sessionRouter = move(sessionRouter);
}
