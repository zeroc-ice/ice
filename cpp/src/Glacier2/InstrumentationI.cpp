//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/InstrumentationI.h>
#include <Ice/InstrumentationI.h>

#include <Ice/LocalException.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Glacier2;
using namespace Glacier2::Instrumentation;
using namespace IceMX;

namespace
{

class SessionHelper : public MetricsHelperT<SessionMetrics>
{
public:

    class Attributes : public AttributeResolverT<SessionHelper>
    {
    public:

        Attributes()
        {
            add("parent", &SessionHelper::getInstanceName);
            add("id", &SessionHelper::getId);
            add("connection", &SessionHelper::getConnection);
            IceInternal::addConnectionAttributes<SessionHelper>(*this);
        }
    };
    static Attributes attributes;

    SessionHelper(const string& instanceName, const string& id, shared_ptr<Ice::Connection> connection, int rtSize) :
        _instanceName(instanceName), _id(id), _connection(move(connection)), _routingTableSize(rtSize)
    {
    }

    string operator()(const string& attribute) const override
    {
        return attributes(this, attribute);
    }

    void initMetrics(const shared_ptr<SessionMetrics>& v) const override
    {
        v->routingTableSize += _routingTableSize;
    }

    const string& getInstanceName() const
    {
        return _instanceName;
    }

    const string& getId() const
    {
        return _id;
    }

    shared_ptr<Ice::ConnectionInfo>
    getConnectionInfo() const
    {
        return _connection->getInfo();
    }

    shared_ptr<Ice::Endpoint>
    getEndpoint() const
    {
        return _connection->getEndpoint();
    }

    const shared_ptr<Ice::Connection>&
    getConnection() const
    {
        return _connection;
    }

    shared_ptr<Ice::EndpointInfo>
    getEndpointInfo() const
    {
        if(!_endpointInfo)
        {
            _endpointInfo = _connection->getEndpoint()->getInfo();
        }
        return _endpointInfo;
    }

private:

    const string& _instanceName;
    const string& _id;
    const shared_ptr<Ice::Connection>& _connection;
    const int _routingTableSize;
    mutable shared_ptr<Ice::EndpointInfo> _endpointInfo;
};

SessionHelper::Attributes SessionHelper::attributes;

namespace
{

struct ForwardedUpdate
{
    ForwardedUpdate(bool clientP) : client(clientP)
    {
    }

    void operator()(const shared_ptr<SessionMetrics>& v)
    {
        if(client)
        {
            ++v->forwardedClient;
            if(v->queuedClient > 0)
            {
                --v->queuedClient;
            }
        }
        else
        {
            ++v->forwardedServer;
            if(v->queuedServer > 0)
            {
                --v->queuedServer;
            }
        }
    }

    int client;
};

}

}

void
SessionObserverI::forwarded(bool client)
{
    forEach(ForwardedUpdate(client));
}

void
SessionObserverI::queued(bool client)
{
    if(client)
    {
        forEach(inc(&SessionMetrics::queuedClient));
    }
    else
    {
        forEach(inc(&SessionMetrics::queuedServer));
    }
}

void
SessionObserverI::overridden(bool client)
{
    if(client)
    {
        forEach(inc(&SessionMetrics::overriddenClient));
    }
    else
    {
        forEach(inc(&SessionMetrics::overriddenServer));
    }
}

void
SessionObserverI::routingTableSize(int delta)
{
    forEach(add(&SessionMetrics::routingTableSize, delta));
}

RouterObserverI::RouterObserverI(shared_ptr<IceInternal::MetricsAdminI> metrics, const string& instanceName) :
    _metrics(move(metrics)), _instanceName(instanceName), _sessions(_metrics, "Session")
{
}

void
RouterObserverI::setObserverUpdater(const shared_ptr<ObserverUpdater>& updater)
{
    _sessions.setUpdater(newUpdater(updater, &ObserverUpdater::updateSessionObservers));
}

shared_ptr<SessionObserver>
RouterObserverI::getSessionObserver(const string& id,
                                    const shared_ptr<Ice::Connection>& connection,
                                    int routingTableSize,
                                    const shared_ptr<SessionObserver>& old)
{
    if(_sessions.isEnabled())
    {
        try
        {
            return _sessions.getObserver(SessionHelper(_instanceName, id, connection, routingTableSize), old);
        }
        catch(const exception& ex)
        {
            Ice::Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}
