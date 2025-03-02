// Copyright (c) ZeroC, Inc.

#include "IPEndpointI.h"
#include "HashUtil.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Instance.h"
#include "NetworkProxy.h"
#include "ProtocolInstance.h"

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

int16_t
IceInternal::IPEndpointI::type() const
{
    return _instance->type();
}

const string&
IceInternal::IPEndpointI::protocol() const
{
    return _instance->protocol();
}

bool
IceInternal::IPEndpointI::secure() const
{
    return _instance->secure();
}

void
IceInternal::IPEndpointI::streamWriteImpl(OutputStream* s) const
{
    s->write(_host, false);
    s->write(_port);
}

const string&
IceInternal::IPEndpointI::connectionId() const
{
    return _connectionId;
}

EndpointIPtr
IceInternal::IPEndpointI::connectionId(const string& connectionId) const
{
    if (connectionId == _connectionId)
    {
        return const_cast<IPEndpointI*>(this)->shared_from_this();
    }
    else
    {
        return createEndpoint(_host, _port, connectionId);
    }
}

void
IceInternal::IPEndpointI::connectorsAsync(
    std::function<void(std::vector<ConnectorPtr>)> response,
    std::function<void(exception_ptr)> exception) const
{
    _instance->resolve(
        _host,
        _port,
        const_cast<IPEndpointI*>(this)->shared_from_this(),
        std::move(response),
        std::move(exception));
}

vector<EndpointIPtr>
IceInternal::IPEndpointI::expandHost() const
{
    // If this endpoint has an empty host (wildcard address), don't expand, just return this endpoint.
    if (_host.empty())
    {
        vector<EndpointIPtr> endpoints;
        endpoints.push_back(const_cast<IPEndpointI*>(this)->shared_from_this());
        return endpoints;
    }

    vector<Address> addrs = getAddresses(_host, _port, _instance->protocolSupport(), _instance->preferIPv6(), true);

    vector<EndpointIPtr> endpoints;
    for (const auto& address : addrs)
    {
        string host;
        int port;
        addrToAddressAndPort(address, host, port);
        endpoints.push_back(createEndpoint(host, port, _connectionId));
    }
    return endpoints;
}

bool
IceInternal::IPEndpointI::isLoopbackOrMulticast() const
{
    return _host.empty() ? false : isLoopbackOrMulticastAddress(_host);
}

bool
IceInternal::IPEndpointI::equivalent(const EndpointIPtr& endpoint) const
{
    auto ipEndpointI = dynamic_pointer_cast<IPEndpointI>(endpoint);
    if (!ipEndpointI)
    {
        return false;
    }
    return ipEndpointI->type() == type() && ipEndpointI->_normalizedHost == _normalizedHost &&
           ipEndpointI->_port == _port;
}

size_t
IceInternal::IPEndpointI::hash() const noexcept
{
    size_t h = 5381;
    hashAdd(h, type());
    hashAdd(h, _host);
    hashAdd(h, _port);
    hashAdd(h, _connectionId);
    if (isAddressValid(_sourceAddr))
    {
        hashAdd(h, inetAddrToString(_sourceAddr));
    }
    return h;
}

string
IceInternal::IPEndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    if (!_host.empty())
    {
        s << " -h ";
        bool addQuote = _host.find(':') != string::npos;
        if (addQuote)
        {
            s << "\"";
        }
        s << _host;
        if (addQuote)
        {
            s << "\"";
        }
    }

    s << " -p " << to_string(_port);

    if (isAddressValid(_sourceAddr))
    {
        const string sourceAddr = inetAddrToString(_sourceAddr);
        bool addQuote = sourceAddr.find(':') != string::npos;
        s << " --sourceAddress ";
        if (addQuote)
        {
            s << "\"";
        }
        s << sourceAddr;
        if (addQuote)
        {
            s << "\"";
        }
    }

    return s.str();
}

bool
IceInternal::IPEndpointI::operator==(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const IPEndpointI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    if (_host != p->_host)
    {
        return false;
    }

    if (_port != p->_port)
    {
        return false;
    }

    if (_connectionId != p->_connectionId)
    {
        return false;
    }

    if (compareAddress(_sourceAddr, p->_sourceAddr) != 0)
    {
        return false;
    }
    return true;
}

bool
IceInternal::IPEndpointI::operator<(const Endpoint& r) const
{
    const auto* p = dynamic_cast<const IPEndpointI*>(&r);
    if (!p)
    {
        const auto* e = dynamic_cast<const EndpointI*>(&r);
        if (!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if (this == p)
    {
        return false;
    }

    if (type() < p->type())
    {
        return true;
    }
    else if (p->type() < type())
    {
        return false;
    }

    if (_host < p->_host)
    {
        return true;
    }
    else if (p->_host < _host)
    {
        return false;
    }

    if (_port < p->_port)
    {
        return true;
    }
    else if (p->_port < _port)
    {
        return false;
    }

    if (_connectionId < p->_connectionId)
    {
        return true;
    }
    else if (p->_connectionId < _connectionId)
    {
        return false;
    }

    int rc = compareAddress(_sourceAddr, p->_sourceAddr);
    if (rc < 0)
    {
        return true;
    }
    else if (rc > 0)
    {
        return false;
    }

    return false;
}

vector<ConnectorPtr>
IceInternal::IPEndpointI::connectors(const vector<Address>& addresses, const NetworkProxyPtr& proxy) const
{
    vector<ConnectorPtr> connectors;
    connectors.reserve(addresses.size());
    for (const auto& address : addresses)
    {
        connectors.push_back(createConnector(address, proxy));
    }
    return connectors;
}

void
IceInternal::IPEndpointI::initWithOptions(vector<string>& args, bool oaEndpoint)
{
    EndpointI::initWithOptions(args);

    if (_host.empty())
    {
        const_cast<string&>(_host) = _instance->defaultHost();
        const_cast<string&>(_normalizedHost) = normalizeIPv6Address(_host);
    }
    else if (_host == "*")
    {
        if (oaEndpoint)
        {
            const_cast<string&>(_host) = string{};
            const_cast<string&>(_normalizedHost) = string{};
        }
        else
        {
            throw Ice::ParseException(__FILE__, __LINE__, "'-h *' not valid for proxy endpoint '" + toString() + "'");
        }
    }

    if (isAddressValid(_sourceAddr))
    {
        if (oaEndpoint)
        {
            throw Ice::ParseException(
                __FILE__,
                __LINE__,
                "'--sourceAddress' not valid for object adapter endpoint '" + toString() + "'");
        }
    }
    else if (!oaEndpoint)
    {
        const_cast<Address&>(_sourceAddr) = _instance->defaultSourceAddress();
    }
}

bool
IceInternal::IPEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    if (option == "-h")
    {
        if (argument.empty())
        {
            throw Ice::ParseException(
                __FILE__,
                __LINE__,
                "no argument provided for -h option in endpoint '" + endpoint + "'");
        }
        const_cast<string&>(_host) = argument;
        const_cast<string&>(_normalizedHost) = normalizeIPv6Address(_host);
    }
    else if (option == "-p")
    {
        if (argument.empty())
        {
            throw Ice::ParseException(
                __FILE__,
                __LINE__,
                "no argument provided for -p option in endpoint '" + endpoint + "'");
        }
        istringstream p(argument);
        if (!(p >> const_cast<int32_t&>(_port)) || !p.eof())
        {
            throw Ice::ParseException(
                __FILE__,
                __LINE__,
                "invalid port value '" + argument + "' in endpoint '" + endpoint + "'");
        }
        else if (_port < 0 || _port > 65535)
        {
            throw Ice::ParseException(
                __FILE__,
                __LINE__,
                "port value '" + argument + "' out of range in endpoint '" + endpoint + "'");
        }
    }
    else if (option == "--sourceAddress")
    {
        if (argument.empty())
        {
            throw Ice::ParseException(
                __FILE__,
                __LINE__,
                "no argument provided for --sourceAddress option in endpoint '" + endpoint + "'");
        }
        const_cast<Address&>(_sourceAddr) = getNumericAddress(argument);
        if (!isAddressValid(_sourceAddr))
        {
            throw Ice::ParseException(
                __FILE__,
                __LINE__,
                "invalid IP address provided for --sourceAddress option in endpoint '" + endpoint + "'");
        }
    }
    else
    {
        return false;
    }
    return true;
}

IceInternal::IPEndpointI::IPEndpointI(
    ProtocolInstancePtr instance,
    string host,
    int port,
    const Address& sourceAddr,
    string connectionId)
    : _instance(std::move(instance)),
      _host(std::move(host)),
      _normalizedHost(normalizeIPv6Address(_host)),
      _port(port),
      _sourceAddr(sourceAddr),
      _connectionId(std::move(connectionId))
{
}

IceInternal::IPEndpointI::IPEndpointI(ProtocolInstancePtr instance) : _instance(std::move(instance)), _port(0) {}

IceInternal::IPEndpointI::IPEndpointI(ProtocolInstancePtr instance, InputStream* s)
    : _instance(std::move(instance)),
      _port(0)
{
    s->read(const_cast<string&>(_host), false);
    const_cast<string&>(_normalizedHost) = normalizeIPv6Address(_host);
    s->read(const_cast<int32_t&>(_port));
}

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance)
    : _instance(instance),
      _protocol(instance->protocolSupport()),
      _preferIPv6(instance->preferIPv6())
{
    updateObserver();
}

void
IceInternal::EndpointHostResolver::resolve(
    const string& host,
    int port,
    const IPEndpointIPtr& endpoint,
    function<void(vector<ConnectorPtr>)> response,
    function<void(exception_ptr)> exception)
{
    //
    // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
    // entry and the thread will take care of getting the endpoint addresses.
    //
    NetworkProxyPtr networkProxy = _instance->networkProxy();
    if (!networkProxy)
    {
        try
        {
            vector<Address> addrs = getAddresses(host, port, _protocol, _preferIPv6, false);
            if (!addrs.empty())
            {
                response(endpoint->connectors(addrs, nullptr));
                return;
            }
        }
        catch (const Ice::LocalException&)
        {
            exception(current_exception());
            return;
        }
    }

    lock_guard lock(_mutex);
    assert(!_destroyed);

    ResolveEntry entry;
    entry.host = host;
    entry.port = port;
    entry.endpoint = endpoint;
    entry.response = std::move(response);
    entry.exception = std::move(exception);

    const CommunicatorObserverPtr& observer = _instance->initializationData().observer;
    if (observer)
    {
        entry.observer = observer->getEndpointLookupObserver(endpoint);
        if (entry.observer)
        {
            entry.observer->attach();
        }
    }

    _queue.push_back(entry);
    _conditionVariable.notify_one();
}

void
IceInternal::EndpointHostResolver::destroy()
{
    lock_guard lock(_mutex);
    assert(!_destroyed);
    _destroyed = true;
    _conditionVariable.notify_one();
}

void
IceInternal::EndpointHostResolver::run()
{
    while (true)
    {
        ResolveEntry r;
        ThreadObserverPtr threadObserver;
        {
            unique_lock lock(_mutex);
            _conditionVariable.wait(lock, [this] { return _destroyed || !_queue.empty(); });

            if (_destroyed)
            {
                break;
            }

            r = _queue.front();
            _queue.pop_front();
            threadObserver = _observer.get();
        }

        if (threadObserver)
        {
            threadObserver->stateChanged(ThreadState::ThreadStateIdle, ThreadState::ThreadStateInUseForOther);
        }

        try
        {
            NetworkProxyPtr networkProxy = _instance->networkProxy();
            ProtocolSupport protocol = _protocol;
            if (networkProxy)
            {
                networkProxy = networkProxy->resolveHost(_protocol);
                if (networkProxy)
                {
                    protocol = networkProxy->getProtocolSupport();
                }
            }

            vector<Address> addresses = getAddresses(r.host, r.port, protocol, _preferIPv6, true);
            if (r.observer)
            {
                r.observer->detach();
                r.observer = nullptr;
            }

            r.response(r.endpoint->connectors(addresses, networkProxy));

            if (threadObserver)
            {
                threadObserver->stateChanged(ThreadState::ThreadStateInUseForOther, ThreadState::ThreadStateIdle);
            }
        }
        catch (const Ice::LocalException& ex)
        {
            if (threadObserver)
            {
                threadObserver->stateChanged(ThreadState::ThreadStateInUseForOther, ThreadState::ThreadStateIdle);
            }
            if (r.observer)
            {
                r.observer->failed(ex.ice_id());
                r.observer->detach();
            }
            r.exception(current_exception());
        }
    }

    for (const auto& p : _queue)
    {
        Ice::CommunicatorDestroyedException ex(__FILE__, __LINE__);
        if (p.observer)
        {
            p.observer->failed(ex.ice_id());
            p.observer->detach();
        }
        p.exception(make_exception_ptr(ex));
    }
    _queue.clear();

    if (_observer)
    {
        _observer.detach();
    }
}

void
IceInternal::EndpointHostResolver::updateObserver()
{
    lock_guard lock(_mutex);
    const CommunicatorObserverPtr& observer = _instance->initializationData().observer;
    if (observer)
    {
        _observer.attach(
            observer
                ->getThreadObserver("Communicator", "Ice.HostResolver", ThreadState::ThreadStateIdle, _observer.get()));
    }
}
