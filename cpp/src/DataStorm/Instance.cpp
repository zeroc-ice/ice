//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Instance.h"
#include "CallbackExecutor.h"
#include "ConnectionManager.h"
#include "DataStorm/Node.h"
#include "LookupI.h"
#include "NodeI.h"
#include "NodeSessionManager.h"
#include "TopicFactoryI.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;

Instance::Instance(const Ice::CommunicatorPtr& communicator) : _communicator(communicator), _shutdown(false)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    if (properties->getPropertyAsIntWithDefault("DataStorm.Node.Server.Enabled", 1) > 0)
    {
        properties->setProperty("DataStorm.Node.Adapters.Server.ThreadPool.SizeMax", "1");
        properties->setProperty("DataStorm.Node.Adapters.Server.Endpoints", "tcp");

        const string pfx = "DataStorm.Node.Server";
        auto props = properties->getPropertiesForPrefix(pfx);
        for (const auto& p : props)
        {
            if (p.first != "DataStorm.Node.Server.Enabled" &&
                p.first != "DataStorm.Node.Server.ForwardDiscoveryToMulticast")
            {
                properties->setProperty("DataStorm.Node.Adapters.Server" + p.first.substr(pfx.length()), p.second);
            }
        }

        try
        {
            _adapter = _communicator->createObjectAdapter("DataStorm.Node.Adapters.Server");
        }
        catch (const Ice::LocalException& ex)
        {
            ostringstream os;
            os << "failed to listen on server endpoints `";
            os << properties->getProperty("DataStorm.Node.Adapters.Server.Endpoints") << "':\n";
            os << ex.what();
            throw invalid_argument(os.str());
        }
    }
    else
    {
        _adapter = _communicator->createObjectAdapter("");
    }

    if (properties->getPropertyAsIntWithDefault("DataStorm.Node.Multicast.Enabled", 1) > 0)
    {
        properties->setProperty("DataStorm.Node.Adapters.Multicast.Endpoints", "udp -h 239.255.0.1 -p 10000");
        properties->setProperty("DataStorm.Node.Adapters.Multicast.PublishedHost", "239.255.0.1");
        properties->setProperty("DataStorm.Node.Adapters.Multicast.ProxyOptions", "-d");
        properties->setProperty("DataStorm.Node.Adapters.Multicast.ThreadPool.SizeMax", "1");

        const string pfx = "DataStorm.Node.Multicast";
        auto props = properties->getPropertiesForPrefix(pfx);
        for (const auto& p : props)
        {
            if (p.first != "DataStorm.Node.Multicast.Enabled")
            {
                properties->setProperty("DataStorm.Node.Adapters.Multicast" + p.first.substr(pfx.length()), p.second);
            }
        }

        try
        {
            _multicastAdapter = _communicator->createObjectAdapter("DataStorm.Node.Adapters.Multicast");
        }
        catch (const Ice::LocalException& ex)
        {
            ostringstream os;
            os << "failed to listen on multicast endpoints `";
            os << properties->getProperty("DataStorm.Node.Adapters.Server.Endpoints") << "':\n";
            os << ex.what();
            throw invalid_argument(os.str());
        }
    }

    _retryDelay = chrono::milliseconds(properties->getPropertyAsIntWithDefault("DataStorm.Node.RetryDelay", 500));
    _retryMultiplier = properties->getPropertyAsIntWithDefault("DataStorm.Node.RetryMultiplier", 2);
    _retryCount = properties->getPropertyAsIntWithDefault("DataStorm.Node.RetryCount", 6);

    //
    // Create a collocated object adapter with a random name to prevent user configuration
    // of the adapter.
    //
    auto collocated = Ice::generateUUID();
    properties->setProperty(collocated + ".AdapterId", collocated);
    _collocatedAdapter = _communicator->createObjectAdapter(collocated);

    _collocatedForwarder = make_shared<ForwarderManager>(_collocatedAdapter, "forwarders");
    _collocatedAdapter->addDefaultServant(_collocatedForwarder, "forwarders");

    _executor = make_shared<CallbackExecutor>();
    _connectionManager = make_shared<ConnectionManager>(_executor);
    _timer = make_shared<IceInternal::Timer>();
    _traceLevels = make_shared<TraceLevels>(_communicator);
}

void
Instance::init()
{
    auto self = shared_from_this();

    _topicFactory = make_shared<TopicFactoryI>(self);

    _node = make_shared<NodeI>(self);
    _node->init();

    _nodeSessionManager = make_shared<NodeSessionManager>(self, _node);
    _nodeSessionManager->init();

    auto lookupI = make_shared<LookupI>(_nodeSessionManager, _topicFactory, _node->getProxy());
    _adapter->add(lookupI, {"Lookup", "DataStorm"});
    if (_multicastAdapter)
    {
        auto lookup = _multicastAdapter->add<DataStormContract::LookupPrx>(lookupI, {"Lookup", "DataStorm"});
        _lookup = lookup->ice_collocationOptimized(false);
    }

    _adapter->activate();
    _collocatedAdapter->activate();
    if (_multicastAdapter)
    {
        _multicastAdapter->activate();
    }
}

void
Instance::shutdown()
{
    unique_lock<mutex> lock(_mutex);
    _shutdown = true;
    _cond.notify_all();
    _topicFactory->shutdown();
}

bool
Instance::isShutdown() const
{
    unique_lock<mutex> lock(_mutex);
    return _shutdown;
}

void
Instance::checkShutdown() const
{
    unique_lock<mutex> lock(_mutex);
    if (_shutdown)
    {
        throw DataStorm::NodeShutdownException();
    }
}

void
Instance::waitForShutdown() const
{
    unique_lock<mutex> lock(_mutex);
    _cond.wait(lock, [&]() { return _shutdown; }); // Wait until shutdown is called
}

void
Instance::destroy(bool ownsCommunicator)
{
    if (ownsCommunicator)
    {
        _communicator->destroy();
    }
    else
    {
        _adapter->destroy();
        _collocatedAdapter->destroy();
        if (_multicastAdapter)
        {
            _multicastAdapter->destroy();
        }
    }
    _node->destroy(ownsCommunicator);

    _executor->destroy();
    _connectionManager->destroy();
    _collocatedForwarder->destroy();
    // Destroy the session manager before the timer to avoid scheduling new tasks after the timer has been destroyed.
    _nodeSessionManager->destroy();
    _timer->destroy();
}
