// Copyright (c) ZeroC, Inc.

#include "InstrumentationI.h"
#include "../Ice/InstrumentationI.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"

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
            Attributes() noexcept
            {
                add("parent", &SessionHelper::getInstanceName);
                add("id", &SessionHelper::getId);
                add("connection", &SessionHelper::getConnection);
                IceInternal::addConnectionAttributes<SessionHelper>(*this);
            }
        };
        static Attributes attributes;

        SessionHelper(const string& instanceName, const string& id, shared_ptr<Ice::Connection> connection, int rtSize)
            : _instanceName(instanceName),
              _id(id),
              _connection(std::move(connection)),
              _routingTableSize(rtSize)
        {
        }

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        void initMetrics(const shared_ptr<SessionMetrics>& v) const override
        {
            v->routingTableSize += _routingTableSize;
        }

        [[nodiscard]] const string& getInstanceName() const { return _instanceName; }

        [[nodiscard]] const string& getId() const { return _id; }

        [[nodiscard]] Ice::ConnectionInfoPtr getConnectionInfo() const { return _connection->getInfo(); }

        [[nodiscard]] Ice::EndpointPtr getEndpoint() const { return _connection->getEndpoint(); }

        [[nodiscard]] const shared_ptr<Ice::Connection>& getConnection() const { return _connection; }

        [[nodiscard]] Ice::EndpointInfoPtr getEndpointInfo() const
        {
            if (!_endpointInfo)
            {
                _endpointInfo = _connection->getEndpoint()->getInfo();
            }
            return _endpointInfo;
        }

    private:
        const string& _instanceName;
        const string& _id;
        const shared_ptr<Ice::Connection> _connection;
        const int _routingTableSize;
        mutable Ice::EndpointInfoPtr _endpointInfo;
    };

    SessionHelper::Attributes SessionHelper::attributes;

    struct ForwardedUpdate
    {
        ForwardedUpdate(bool clientP) : client(clientP) {}

        void operator()(const shared_ptr<SessionMetrics>& v)
        {
            if (client)
            {
                ++v->forwardedClient;
            }
            else
            {
                ++v->forwardedServer;
            }
        }

        int client;
    };
}

void
SessionObserverI::forwarded(bool client)
{
    forEach(ForwardedUpdate(client));
}

void
SessionObserverI::routingTableSize(int delta)
{
    forEach(add(&SessionMetrics::routingTableSize, delta));
}

RouterObserverI::RouterObserverI(shared_ptr<IceInternal::MetricsAdminI> metrics, string instanceName)
    : _metrics(std::move(metrics)),
      _instanceName(std::move(instanceName)),
      _sessions(_metrics, "Session")
{
}

void
RouterObserverI::setObserverUpdater(const shared_ptr<ObserverUpdater>& updater)
{
    _sessions.setUpdater(newUpdater(updater, &ObserverUpdater::updateSessionObservers));
}

shared_ptr<SessionObserver>
RouterObserverI::getSessionObserver(
    const string& id,
    const shared_ptr<Ice::Connection>& connection,
    int routingTableSize,
    const shared_ptr<SessionObserver>& old)
{
    if (_sessions.isEnabled())
    {
        try
        {
            return _sessions.getObserver(SessionHelper(_instanceName, id, connection, routingTableSize), old);
        }
        catch (const exception& ex)
        {
            Ice::Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}
