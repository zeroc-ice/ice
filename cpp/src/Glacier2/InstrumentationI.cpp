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

    SessionHelper(const string& instanceName, const string& id, const ::Ice::ConnectionPtr& connection, int rtSize) :
        _instanceName(instanceName), _id(id), _connection(connection), _routingTableSize(rtSize)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    virtual void initMetrics(const SessionMetricsPtr& v) const
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

    ::Ice::ConnectionInfoPtr
    getConnectionInfo() const
    {
        return _connection->getInfo();
    }

    ::Ice::EndpointPtr
    getEndpoint() const
    {
        return _connection->getEndpoint();
    }

    const ::Ice::ConnectionPtr&
    getConnection() const
    {
        return _connection;
    }

    const ::Ice::EndpointInfoPtr&
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
    const ::Ice::ConnectionPtr& _connection;
    const int _routingTableSize;
    mutable ::Ice::EndpointInfoPtr _endpointInfo;
};

SessionHelper::Attributes SessionHelper::attributes;

namespace
{

struct ForwardedUpdate
{
    ForwardedUpdate(bool clientP) : client(clientP)
    {
    }

    void operator()(const SessionMetricsPtr& v)
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

RouterObserverI::RouterObserverI(const IceInternal::MetricsAdminIPtr& metrics, const string& instanceName) :
    _metrics(metrics), _instanceName(instanceName), _sessions(metrics, "Session")
{
}

void
RouterObserverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{
    _sessions.setUpdater(newUpdater(updater, &ObserverUpdater::updateSessionObservers));
}

SessionObserverPtr
RouterObserverI::getSessionObserver(const string& id,
                                    const ::Ice::ConnectionPtr& connection,
                                    int routingTableSize,
                                    const SessionObserverPtr& old)
{
    if(_sessions.isEnabled())
    {
        try
        {
            return _sessions.getObserver(SessionHelper(_instanceName, id, connection, routingTableSize), old);
        }
        catch(const exception& ex)
        {
            ::Ice::Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}
