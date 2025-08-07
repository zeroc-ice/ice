// Copyright (c) ZeroC, Inc.

#include "InstrumentationI.h"
#include "Ice/Communicator.h"
#include "Ice/Connection.h"
#include "Ice/Endpoint.h"
#include "Ice/LoggerUtil.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/VersionFunctions.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace Ice::Instrumentation;
using namespace IceMX;

namespace
{
    int ThreadMetrics::* getThreadStateMetric(ThreadState s)
    {
        switch (s)
        {
            case ThreadState::ThreadStateIdle:
                return nullptr;
            case ThreadState::ThreadStateInUseForIO:
                return &ThreadMetrics::inUseForIO;
            case ThreadState::ThreadStateInUseForUser:
                return &ThreadMetrics::inUseForUser;
            case ThreadState::ThreadStateInUseForOther:
                return &ThreadMetrics::inUseForOther;
            default:
                assert(false);
                return nullptr;
        }
    }

    struct ThreadStateChanged
    {
        ThreadStateChanged(ThreadState oldStateP, ThreadState newStateP) : oldState(oldStateP), newState(newStateP) {}

        void operator()(const ThreadMetricsPtr& v)
        {
            if (oldState != ThreadState::ThreadStateIdle)
            {
                --(v.get()->*getThreadStateMetric(oldState));
            }
            if (newState != ThreadState::ThreadStateIdle)
            {
                ++(v.get()->*getThreadStateMetric(newState));
            }
        }

        ThreadState oldState;
        ThreadState newState;
    };

    IPConnectionInfoPtr getIPConnectionInfo(const ConnectionInfoPtr& info)
    {
        for (ConnectionInfoPtr p = info; p; p = p->underlying)
        {
            IPConnectionInfoPtr ipInfo = dynamic_pointer_cast<IPConnectionInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }
        return nullptr;
    }

    class ConnectionHelper : public MetricsHelperT<ConnectionMetrics>
    {
    public:
        class Attributes : public AttributeResolverT<ConnectionHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &ConnectionHelper::getParent);
                add("id", &ConnectionHelper::getId);
                add("state", &ConnectionHelper::getState);
                addConnectionAttributes<ConnectionHelper>(*this);
            }
        };
        static Attributes attributes;

        ConnectionHelper(const ConnectionInfoPtr& con, const EndpointPtr& endpt, ConnectionState state)
            : _connectionInfo(con),
              _endpoint(endpt),
              _state(state)
        {
        }

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        [[nodiscard]] const string& getId() const
        {
            if (_id.empty())
            {
                ostringstream os;
                IPConnectionInfoPtr info = getIPConnectionInfo(_connectionInfo);
                if (info)
                {
                    os << info->localAddress << ':' << info->localPort;
                    os << " -> ";
                    os << info->remoteAddress << ':' << info->remotePort;
                }
                else
                {
                    os << "connection-" << _connectionInfo.get();
                }
                if (!_connectionInfo->connectionId.empty())
                {
                    os << " [" << _connectionInfo->connectionId << "]";
                }
                _id = os.str();
            }
            return _id;
        }

        [[nodiscard]] string getState() const
        {
            switch (_state)
            {
                case ConnectionState::ConnectionStateValidating:
                    return "validating";
                case ConnectionState::ConnectionStateHolding:
                    return "holding";
                case ConnectionState::ConnectionStateActive:
                    return "active";
                case ConnectionState::ConnectionStateClosing:
                    return "closing";
                case ConnectionState::ConnectionStateClosed:
                    return "closed";
                default:
                    assert(false);
                    return "";
            }
        }

        [[nodiscard]] string getParent() const
        {
            if (!_connectionInfo->adapterName.empty())
            {
                return _connectionInfo->adapterName;
            }
            else
            {
                return "Communicator";
            }
        }

        [[nodiscard]] const ConnectionInfoPtr& getConnectionInfo() const { return _connectionInfo; }

        [[nodiscard]] const EndpointPtr& getEndpoint() const { return _endpoint; }

        [[nodiscard]] const EndpointInfoPtr& getEndpointInfo() const
        {
            if (!_endpointInfo)
            {
                _endpointInfo = _endpoint->getInfo();
            }
            return _endpointInfo;
        }

    private:
        const ConnectionInfoPtr& _connectionInfo;
        const EndpointPtr& _endpoint;
        const ConnectionState _state;
        mutable string _id;
        mutable EndpointInfoPtr _endpointInfo;
    };

    ConnectionHelper::Attributes ConnectionHelper::attributes;

    class DispatchHelper : public MetricsHelperT<DispatchMetrics>
    {
    public:
        class Attributes : public AttributeResolverT<DispatchHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &DispatchHelper::getParent);
                add("id", &DispatchHelper::getId);
                add("connection", &DispatchHelper::getConnection);

                addConnectionAttributes<DispatchHelper>(*this);

                add("operation", &DispatchHelper::getCurrent, &Current::operation);
                add("identity", &DispatchHelper::getIdentity);
                add("facet", &DispatchHelper::getCurrent, &Current::facet);
                add("mode", &DispatchHelper::getMode);
                add("requestId", &DispatchHelper::getCurrent, &Current::requestId);

                setDefault(&DispatchHelper::resolve);
            }
        };
        static Attributes attributes;

        DispatchHelper(const Current& current, int size) : _current(current), _size(size) {}

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        void initMetrics(const DispatchMetricsPtr& v) const override { v->size += _size; }

        [[nodiscard]] string resolve(const string& attribute) const
        {
            if (attribute.compare(0, 8, "context.") == 0)
            {
                auto p = _current.ctx.find(attribute.substr(8));
                if (p != _current.ctx.end())
                {
                    return p->second;
                }
            }
            throw invalid_argument(attribute);
        }

        [[nodiscard]] string getMode() const { return _current.requestId == 0 ? "oneway" : "twoway"; }

        [[nodiscard]] const string& getId() const
        {
            if (_id.empty())
            {
                ostringstream os;
                if (!_current.id.category.empty())
                {
                    os << _current.id.category << '/';
                }
                os << _current.id.name << " [" << _current.operation << ']';
                _id = os.str();
            }
            return _id;
        }

        [[nodiscard]] string getParent() const { return _current.adapter->getName(); }

        [[nodiscard]] ConnectionInfoPtr getConnectionInfo() const
        {
            if (_current.con)
            {
                return _current.con->getInfo();
            }
            return nullptr;
        }

        [[nodiscard]] EndpointPtr getEndpoint() const
        {
            if (_current.con)
            {
                return _current.con->getEndpoint();
            }
            return nullptr;
        }

        [[nodiscard]] const ConnectionPtr& getConnection() const { return _current.con; }

        [[nodiscard]] const EndpointInfoPtr& getEndpointInfo() const
        {
            if (_current.con && !_endpointInfo)
            {
                _endpointInfo = _current.con->getEndpoint()->getInfo();
            }
            return _endpointInfo;
        }

        [[nodiscard]] const Current& getCurrent() const { return _current; }

        [[nodiscard]] string getIdentity() const
        {
            return _current.adapter->getCommunicator()->identityToString(_current.id);
        }

    private:
        const Current& _current;
        const int _size;
        mutable string _id;
        mutable EndpointInfoPtr _endpointInfo;
    };

    DispatchHelper::Attributes DispatchHelper::attributes;

    class InvocationHelper : public MetricsHelperT<InvocationMetrics>
    {
    public:
        class Attributes : public AttributeResolverT<InvocationHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &InvocationHelper::getParent);
                add("id", &InvocationHelper::getId);
                add("operation", &InvocationHelper::getOperation);
                add("identity", &InvocationHelper::getIdentity);
                add("facet", &InvocationHelper::getProxy, &Ice::ObjectPrx::ice_getFacet);
                add("encoding", &InvocationHelper::getProxy, &Ice::ObjectPrx::ice_getEncodingVersion);
                add("mode", &InvocationHelper::getMode);
                add("proxy", &InvocationHelper::getProxy);

                setDefault(&InvocationHelper::resolve);
            }
        };
        static Attributes attributes;
        InvocationHelper(const optional<ObjectPrx>& proxy, string_view op, const Context& ctx)
            : _proxy(proxy),
              _operation(op),
              _context(ctx)
        {
        }

        [[nodiscard]] string resolve(const string& attribute) const
        {
            if (attribute.compare(0, 8, "context.") == 0)
            {
                auto p = _context.find(attribute.substr(8));
                if (p != _context.end())
                {
                    return p->second;
                }
            }
            throw invalid_argument(attribute);
        }

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        [[nodiscard]] string getMode() const
        {
            if (!_proxy)
            {
                throw invalid_argument("mode");
            }

            if (_proxy->ice_isTwoway())
            {
                return "twoway";
            }
            else if (_proxy->ice_isOneway())
            {
                return "oneway";
            }
            else if (_proxy->ice_isBatchOneway())
            {
                return "batch-oneway";
            }
            else if (_proxy->ice_isDatagram())
            {
                return "datagram";
            }
            else if (_proxy->ice_isBatchDatagram())
            {
                return "batch-datagram";
            }
            else
            {
                throw invalid_argument("mode");
            }
        }

        [[nodiscard]] const string& getId() const
        {
            if (_id.empty())
            {
                ostringstream os;
                if (_proxy)
                {
                    try
                    {
                        os << _proxy->ice_endpoints(Ice::EndpointSeq())->ice_toString() << " [" << _operation << ']';
                    }
                    catch (const Exception&)
                    {
                        // Either a fixed proxy or the communicator is destroyed.
                        os << _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
                        os << " [" << _operation << ']';
                    }
                }
                else
                {
                    os << _operation;
                }
                _id = os.str();
            }
            return _id;
        }

        [[nodiscard]] string getParent() const { return "Communicator"; }

        [[nodiscard]] const optional<ObjectPrx>& getProxy() const { return _proxy; }

        [[nodiscard]] string getIdentity() const
        {
            if (_proxy)
            {
                return _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
            }
            else
            {
                return "";
            }
        }

        [[nodiscard]] string_view getOperation() const { return _operation; }

    private:
        const optional<ObjectPrx>& _proxy;
        string_view _operation;
        const Context& _context;
        mutable string _id;
    };

    InvocationHelper::Attributes InvocationHelper::attributes;

    class RemoteInvocationHelper : public MetricsHelperT<RemoteMetrics>
    {
    public:
        class Attributes : public AttributeResolverT<RemoteInvocationHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &RemoteInvocationHelper::getParent);
                add("id", &RemoteInvocationHelper::getId);
                add("requestId", &RemoteInvocationHelper::_requestId);
                addConnectionAttributes<RemoteInvocationHelper>(*this);
            }
        };
        static Attributes attributes;

        RemoteInvocationHelper(const ConnectionInfoPtr& con, const EndpointPtr& endpt, int requestId, int size)
            : _connectionInfo(con),
              _endpoint(endpt),
              _requestId(requestId),
              _size(size)
        {
        }

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        void initMetrics(const RemoteMetricsPtr& v) const override { v->size += _size; }

        [[nodiscard]] const string& getId() const
        {
            if (_id.empty())
            {
                _id = _endpoint->toString();
                if (!_connectionInfo->connectionId.empty())
                {
                    _id += " [" + _connectionInfo->connectionId + "]";
                }
            }
            return _id;
        }

        [[nodiscard]] string getParent() const
        {
            if (!_connectionInfo->adapterName.empty())
            {
                return _connectionInfo->adapterName;
            }
            else
            {
                return "Communicator";
            }
        }

        [[nodiscard]] const ConnectionInfoPtr& getConnectionInfo() const { return _connectionInfo; }

        [[nodiscard]] const EndpointPtr& getEndpoint() const { return _endpoint; }

        [[nodiscard]] const EndpointInfoPtr& getEndpointInfo() const
        {
            if (!_endpointInfo)
            {
                _endpointInfo = _endpoint->getInfo();
            }
            return _endpointInfo;
        }

    private:
        const ConnectionInfoPtr& _connectionInfo;
        const EndpointPtr& _endpoint;

    protected:
        // COMPILERFIX: Clang 4.2 reports unused-private-field for the _requestId field that is only used in the nested
        // Attributes class.
        const int _requestId;

    private:
        const int _size;
        mutable string _id;
        mutable EndpointInfoPtr _endpointInfo;
    };

    RemoteInvocationHelper::Attributes RemoteInvocationHelper::attributes;

    class CollocatedInvocationHelper : public MetricsHelperT<CollocatedMetrics>
    {
    public:
        class Attributes : public AttributeResolverT<CollocatedInvocationHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &CollocatedInvocationHelper::getParent);
                add("id", &CollocatedInvocationHelper::getId);
                add("requestId", &CollocatedInvocationHelper::_requestId);
            }
        };
        static Attributes attributes;

        CollocatedInvocationHelper(const Ice::ObjectAdapterPtr& adapter, int requestId, int size)
            : _requestId(requestId),
              _size(size),
              _id(adapter->getName())
        {
        }

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        void initMetrics(const CollocatedMetricsPtr& v) const override { v->size += _size; }

        [[nodiscard]] const string& getId() const { return _id; }

        [[nodiscard]] string getParent() const { return "Communicator"; }

    protected:
        // COMPILERFIX: Clang 4.2 reports unused-private-field for the _requestId field that is only used in the nested
        // Attributes class.
        const int _requestId;

    private:
        const int _size;
        mutable string _id;
    };

    CollocatedInvocationHelper::Attributes CollocatedInvocationHelper::attributes;

    class ThreadHelper : public MetricsHelperT<ThreadMetrics>
    {
    public:
        class Attributes : public AttributeResolverT<ThreadHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &ThreadHelper::_parent);
                add("id", &ThreadHelper::_id);
            }
        };
        static Attributes attributes;

        ThreadHelper(string parent, string id, ThreadState state)
            : _parent(std::move(parent)),
              _id(std::move(id)),
              _state(state)
        {
        }

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        void initMetrics(const ThreadMetricsPtr& v) const override
        {
            if (_state != ThreadState::ThreadStateIdle)
            {
                ++(v.get()->*getThreadStateMetric(_state));
            }
        }

    private:
        const string _parent;
        const string _id;
        const ThreadState _state;
    };

    ThreadHelper::Attributes ThreadHelper::attributes;

    class EndpointHelper : public MetricsHelperT<Metrics>
    {
    public:
        class Attributes : public AttributeResolverT<EndpointHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &EndpointHelper::getParent);
                add("id", &EndpointHelper::getId);
                addEndpointAttributes<EndpointHelper>(*this);
            }
        };
        static Attributes attributes;

        EndpointHelper(EndpointPtr endpt, string id) : _endpoint(std::move(endpt)), _id(std::move(id)) {}

        EndpointHelper(EndpointPtr endpt) : _endpoint(std::move(endpt)) {}

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        [[nodiscard]] const EndpointInfoPtr& getEndpointInfo() const
        {
            if (!_endpointInfo)
            {
                _endpointInfo = _endpoint->getInfo();
            }
            return _endpointInfo;
        }

        [[nodiscard]] string getParent() const { return "Communicator"; }

        [[nodiscard]] const string& getId() const
        {
            if (_id.empty())
            {
                _id = _endpoint->toString();
            }
            return _id;
        }

        [[nodiscard]] string getEndpoint() const { return _endpoint->toString(); }

    private:
        const EndpointPtr _endpoint;
        mutable string _id;
        mutable EndpointInfoPtr _endpointInfo;
    };

    EndpointHelper::Attributes EndpointHelper::attributes;
}

void
ConnectionObserverI::sentBytes(int32_t num)
{
    forEach(add(&ConnectionMetrics::sentBytes, num));
    if (_delegate)
    {
        _delegate->sentBytes(num);
    }
}

void
ConnectionObserverI::receivedBytes(int32_t num)
{
    forEach(add(&ConnectionMetrics::receivedBytes, num));
    if (_delegate)
    {
        _delegate->receivedBytes(num);
    }
}

void
ThreadObserverI::stateChanged(ThreadState oldState, ThreadState newState)
{
    forEach(ThreadStateChanged(oldState, newState));
    if (_delegate)
    {
        _delegate->stateChanged(oldState, newState);
    }
}

void
DispatchObserverI::userException()
{
    forEach(inc(&DispatchMetrics::userException));
    if (_delegate)
    {
        _delegate->userException();
    }
}

void
DispatchObserverI::reply(int32_t size)
{
    forEach(add(&DispatchMetrics::replySize, size));
    if (_delegate)
    {
        _delegate->reply(size);
    }
}

void
RemoteObserverI::reply(int32_t size)
{
    forEach(add(&RemoteMetrics::replySize, size));
    if (_delegate)
    {
        _delegate->reply(size);
    }
}

void
CollocatedObserverI::reply(int32_t size)
{
    forEach(add(&CollocatedMetrics::replySize, size));
    if (_delegate)
    {
        _delegate->reply(size);
    }
}

void
InvocationObserverI::retried()
{
    forEach(inc(&InvocationMetrics::retry));
    if (_delegate)
    {
        _delegate->retried();
    }
}

void
InvocationObserverI::userException()
{
    forEach(inc(&InvocationMetrics::userException));
    if (_delegate)
    {
        _delegate->userException();
    }
}

RemoteObserverPtr
InvocationObserverI::getRemoteObserver(
    const ConnectionInfoPtr& connection,
    const EndpointPtr& endpoint,
    int requestId,
    int size)
{
    try
    {
        RemoteObserverPtr delegate;
        if (_delegate)
        {
            delegate = _delegate->getRemoteObserver(connection, endpoint, requestId, size);
        }
        return getObserverWithDelegate<RemoteObserverI>(
            "Remote",
            RemoteInvocationHelper(connection, endpoint, requestId, size),
            delegate);
    }
    catch (const exception&)
    {
    }
    return nullptr;
}

CollocatedObserverPtr
InvocationObserverI::getCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, int requestId, int size)
{
    try
    {
        CollocatedObserverPtr delegate;
        if (_delegate)
        {
            delegate = _delegate->getCollocatedObserver(adapter, requestId, size);
        }
        return getObserverWithDelegate<CollocatedObserverI>(
            "Collocated",
            CollocatedInvocationHelper(adapter, requestId, size),
            delegate);
    }
    catch (const exception&)
    {
    }
    return nullptr;
}

CommunicatorObserverI::CommunicatorObserverI(const InitializationData& initData)
    : _metrics(new MetricsAdminI(initData.properties, initData.logger)),
      _delegate(initData.observer),
      _connections(_metrics, "Connection"),
      _dispatch(_metrics, "Dispatch"),
      _invocations(_metrics, "Invocation"),
      _threads(_metrics, "Thread"),
      _connects(_metrics, "ConnectionEstablishment"),
      _endpointLookups(_metrics, "EndpointLookup")
{
    _invocations.registerSubMap<RemoteMetrics>("Remote", &InvocationMetrics::remotes);
    _invocations.registerSubMap<CollocatedMetrics>("Collocated", &InvocationMetrics::collocated);
}

void
CommunicatorObserverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{
    _connections.setUpdater(newUpdater(updater, &ObserverUpdater::updateConnectionObservers));
    _threads.setUpdater(newUpdater(updater, &ObserverUpdater::updateThreadObservers));
    if (_delegate)
    {
        _delegate->setObserverUpdater(updater);
    }
}

ObserverPtr
CommunicatorObserverI::getConnectionEstablishmentObserver(const EndpointPtr& endpt, const string& connector)
{
    if (_connects.isEnabled())
    {
        try
        {
            ObserverPtr delegate;
            if (_delegate)
            {
                delegate = _delegate->getConnectionEstablishmentObserver(endpt, connector);
            }
            return _connects.getObserverWithDelegate(EndpointHelper(endpt, connector), delegate);
        }
        catch (const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}

ObserverPtr
CommunicatorObserverI::getEndpointLookupObserver(const EndpointPtr& endpt)
{
    if (_endpointLookups.isEnabled())
    {
        try
        {
            ObserverPtr delegate;
            if (_delegate)
            {
                delegate = _delegate->getEndpointLookupObserver(endpt);
            }
            return _endpointLookups.getObserverWithDelegate(EndpointHelper(endpt), delegate);
        }
        catch (const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}

ConnectionObserverPtr
CommunicatorObserverI::getConnectionObserver(
    const ConnectionInfoPtr& con,
    const EndpointPtr& endpt,
    ConnectionState state,
    const ConnectionObserverPtr& observer)
{
    if (_connections.isEnabled())
    {
        try
        {
            ConnectionObserverPtr delegate;
            auto o = dynamic_pointer_cast<ConnectionObserverI>(observer);
            if (_delegate)
            {
                delegate = _delegate->getConnectionObserver(con, endpt, state, o ? o->getDelegate() : observer);
            }
            return _connections.getObserverWithDelegate(ConnectionHelper(con, endpt, state), delegate, observer);
        }
        catch (const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}

ThreadObserverPtr
CommunicatorObserverI::getThreadObserver(
    const string& parent,
    const string& id,
    ThreadState state,
    const ThreadObserverPtr& observer)
{
    if (_threads.isEnabled())
    {
        try
        {
            ThreadObserverPtr delegate;
            auto o = dynamic_pointer_cast<ThreadObserverI>(observer);
            if (_delegate)
            {
                delegate = _delegate->getThreadObserver(parent, id, state, o ? o->getDelegate() : observer);
            }
            return _threads.getObserverWithDelegate(ThreadHelper(parent, id, state), delegate, observer);
        }
        catch (const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}

InvocationObserverPtr
CommunicatorObserverI::getInvocationObserver(const optional<ObjectPrx>& proxy, string_view op, const Context& ctx)
{
    if (_invocations.isEnabled())
    {
        try
        {
            InvocationObserverPtr delegate;
            if (_delegate)
            {
                delegate = _delegate->getInvocationObserver(proxy, op, ctx);
            }
            return _invocations.getObserverWithDelegate(InvocationHelper(proxy, op, ctx), delegate);
        }
        catch (const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}

DispatchObserverPtr
CommunicatorObserverI::getDispatchObserver(const Current& current, int size)
{
    if (_dispatch.isEnabled())
    {
        try
        {
            DispatchObserverPtr delegate;
            if (_delegate)
            {
                delegate = _delegate->getDispatchObserver(current, size);
            }
            return _dispatch.getObserverWithDelegate(DispatchHelper(current, size), delegate);
        }
        catch (const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}

const IceInternal::MetricsAdminIPtr&
CommunicatorObserverI::getFacet() const
{
    assert(_metrics);
    return _metrics;
}

void
CommunicatorObserverI::destroy()
{
    _connections.destroy();
    _dispatch.destroy();
    _invocations.destroy();
    _threads.destroy();
    _connects.destroy();
    _endpointLookups.destroy();

    _metrics->destroy();
}
