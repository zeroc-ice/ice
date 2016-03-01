// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ACM.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ACMMonitor* p) { return p; }
IceUtil::Shared* IceInternal::upCast(FactoryACMMonitor* p) { return p; }

IceInternal::ACMConfig::ACMConfig(bool server) :
    timeout(IceUtil::Time::seconds(60)), 
    heartbeat(Ice::HeartbeatOnInvocation), 
    close(server ? Ice::CloseOnInvocation : Ice::CloseOnInvocationAndIdle)
{
}

IceInternal::ACMConfig::ACMConfig(const Ice::PropertiesPtr& p, 
                                  const Ice::LoggerPtr& l, 
                                  const string& prefix,
                                  const ACMConfig& dflt)
{
    string timeoutProperty;
    if((prefix == "Ice.ACM.Client" || prefix == "Ice.ACM.Server") && p->getProperty(prefix + ".Timeout").empty())
    {
        timeoutProperty = prefix; // Deprecated property.
    }
    else
    {
        timeoutProperty = prefix + ".Timeout";
    };

    this->timeout = IceUtil::Time::seconds(p->getPropertyAsIntWithDefault(timeoutProperty, 
                                                                          static_cast<int>(dflt.timeout.toSeconds())));
    int hb = p->getPropertyAsIntWithDefault(prefix + ".Heartbeat", dflt.heartbeat);
    if(hb >= Ice::HeartbeatOff && hb <= Ice::HeartbeatAlways)
    {
        this->heartbeat = static_cast<Ice::ACMHeartbeat>(hb);
    }
    else
    {
        l->warning("invalid value for property `" + prefix + ".Heartbeat" + "', default value will be used instead");
        this->heartbeat = dflt.heartbeat;
    }

    int cl = p->getPropertyAsIntWithDefault(prefix + ".Close", dflt.close);
    if(cl >= Ice::CloseOff && cl <= Ice::CloseOnIdleForceful)
    {
        this->close = static_cast<Ice::ACMClose>(cl);
    }
    else
    {
        l->warning("invalid value for property `" + prefix + ".Close" + "', default value will be used instead");
        this->close = dflt.close;
    }
}

IceInternal::FactoryACMMonitor::FactoryACMMonitor(const InstancePtr& instance, const ACMConfig& config) :
    _instance(instance), _config(config)
{
}

IceInternal::FactoryACMMonitor::~FactoryACMMonitor()
{
    assert(!_instance);
    assert(_connections.empty());
    assert(_changes.empty());
    assert(_reapedConnections.empty());
}

void
IceInternal::FactoryACMMonitor::destroy()
{
    Lock sync(*this);
    if(!_instance)
    {
        return;
    }

    _instance = 0;
    _connections.clear();
    _changes.clear();
}

void
IceInternal::FactoryACMMonitor::add(const ConnectionIPtr& connection)
{
    if(_config.timeout == IceUtil::Time())
    {
        return;
    }

    Lock sync(*this);
    if(_connections.empty())
    {
        _connections.insert(connection);
        _instance->timer()->scheduleRepeated(this, _config.timeout / 2);
    }
    else
    {
        _changes.push_back(make_pair(connection, true));
    }
}

void
IceInternal::FactoryACMMonitor::remove(const ConnectionIPtr& connection)
{
    if(_config.timeout == IceUtil::Time())
    {
        return;
    }

    Lock sync(*this);
    assert(_instance);
    _changes.push_back(make_pair(connection, false));
}

void
IceInternal::FactoryACMMonitor::reap(const ConnectionIPtr& connection)
{
    Lock sync(*this);
    _reapedConnections.push_back(connection);
}

ACMMonitorPtr
IceInternal::FactoryACMMonitor::acm(const IceUtil::Optional<int>& timeout, 
                                    const IceUtil::Optional<Ice::ACMClose>& close, 
                                    const IceUtil::Optional<Ice::ACMHeartbeat>& heartbeat)
{
    Lock sync(*this);
    assert(_instance);

    ACMConfig config(_config);
    if(timeout)
    {
        config.timeout = IceUtil::Time::seconds(*timeout);
    }
    if(close)
    {
        config.close = *close;
    }
    if(heartbeat)
    {
        config.heartbeat = *heartbeat;
    }
    return new ConnectionACMMonitor(this, _instance->timer(), config);
}

Ice::ACM
IceInternal::FactoryACMMonitor::getACM()
{
    Ice::ACM acm;
    acm.timeout = static_cast<int>(_config.timeout.toSeconds());
    acm.close = _config.close;
    acm.heartbeat = _config.heartbeat;
    return acm;
}

void
IceInternal::FactoryACMMonitor::swapReapedConnections(vector<ConnectionIPtr>& connections)
{
    Lock sync(*this);
    _reapedConnections.swap(connections);
}

void
IceInternal::FactoryACMMonitor::runTimerTask()
{
    {
        Lock sync(*this);
        if(!_instance)
        {
            return;
        }

        for(vector<pair<ConnectionIPtr, bool> >::const_iterator p = _changes.begin(); p != _changes.end(); ++p)
        {
            if(p->second)
            {
                _connections.insert(p->first);
            }
            else
            {
                _connections.erase(p->first);
            }
        }
        _changes.clear();

        if(_connections.empty())
        {
            _instance->timer()->cancel(this);
            return;
        }
    }

        
    //
    // Monitor connections outside the thread synchronization, so
    // that connections can be added or removed during monitoring.
    //
    IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
    for(set<ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
        try
        {          
            (*p)->monitor(now, _config);
        }
        catch(const exception& ex)
        {   
            handleException(ex);
        }
        catch(...)
        {
            handleException();
        }
    }
}

void
FactoryACMMonitor::handleException(const exception& ex)
{
    Lock sync(*this);
    if(!_instance)
    {
        return;
    }
    
    Error out(_instance->initializationData().logger);
    out << "exception in connection monitor:\n" << ex.what();
}

void
FactoryACMMonitor::handleException()
{
    Lock sync(*this);
    if(!_instance)
    {
        return;
    }
    
    Error out(_instance->initializationData().logger);
    out << "unknown exception in connection monitor";
}

IceInternal::ConnectionACMMonitor::ConnectionACMMonitor(const FactoryACMMonitorPtr& parent, 
                                                        const IceUtil::TimerPtr& timer,
                                                        const ACMConfig& config) :
    _parent(parent), _timer(timer), _config(config)
{
}

IceInternal::ConnectionACMMonitor::~ConnectionACMMonitor()
{
    assert(!_connection);
}

void
IceInternal::ConnectionACMMonitor::add(const ConnectionIPtr& connection)
{
    Lock sync(*this);
    assert(!_connection && connection);
    _connection = connection;
    if(_config.timeout != IceUtil::Time())
    {
        _timer->scheduleRepeated(this, _config.timeout / 2);
    }
}

void
IceInternal::ConnectionACMMonitor::remove(const ConnectionIPtr& connection)
{
    Lock sync(*this);
    assert(_connection == connection);
    if(_config.timeout != IceUtil::Time())
    {
        _timer->cancel(this);
    }
    _connection = 0;
}

void
IceInternal::ConnectionACMMonitor::reap(const ConnectionIPtr& connection)
{
    _parent->reap(connection);
}

ACMMonitorPtr
IceInternal::ConnectionACMMonitor::acm(const IceUtil::Optional<int>& timeout, 
                                       const IceUtil::Optional<Ice::ACMClose>& close, 
                                       const IceUtil::Optional<Ice::ACMHeartbeat>& heartbeat)
{
    return _parent->acm(timeout, close, heartbeat);
}

Ice::ACM
IceInternal::ConnectionACMMonitor::getACM()
{
    Ice::ACM acm;
    acm.timeout = static_cast<int>(_config.timeout.toSeconds());
    acm.close = _config.close;
    acm.heartbeat = _config.heartbeat;
    return acm;
}

void
IceInternal::ConnectionACMMonitor::runTimerTask()
{
    Ice::ConnectionIPtr connection;
    {
        Lock sync(*this);
        if(!_connection)
        {
            return;
        }
        connection = _connection;
    }
    
    try
    {          
        connection->monitor(IceUtil::Time::now(IceUtil::Time::Monotonic), _config);
    }
    catch(const exception& ex)
    {   
        _parent->handleException(ex);
    }
    catch(...)
    {
        _parent->handleException();
    }
}
