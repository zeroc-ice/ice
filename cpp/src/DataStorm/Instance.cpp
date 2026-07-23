// Copyright (c) ZeroC, Inc.

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
using namespace Ice;

namespace
{
    DataStorm::ClearHistoryPolicy parseClearHistory(const std::string& value)
    {
        if (value == "OnAdd")
        {
            return DataStorm::ClearHistoryPolicy::OnAdd;
        }
        else if (value == "OnRemove")
        {
            return DataStorm::ClearHistoryPolicy::OnRemove;
        }
        else if (value == "OnAll")
        {
            return DataStorm::ClearHistoryPolicy::OnAll;
        }
        else if (value == "OnAllExceptPartialUpdate")
        {
            return DataStorm::ClearHistoryPolicy::OnAllExceptPartialUpdate;
        }
        else if (value == "Never")
        {
            return DataStorm::ClearHistoryPolicy::Never;
        }
        else
        {
            throw ParseException(__FILE__, __LINE__, "Invalid clear history policy: " + value);
        }
    }

    DataStorm::DiscardPolicy parseDiscardPolicy(const std::string& value)
    {
        // "None" is accepted as an alias for "Never" to match the DataStorm::DiscardPolicy::None enumerator name.
        if (value == "Never" || value == "None")
        {
            return DataStorm::DiscardPolicy::None;
        }
        else if (value == "SendTime")
        {
            return DataStorm::DiscardPolicy::SendTime;
        }
        else if (value == "Priority")
        {
            return DataStorm::DiscardPolicy::Priority;
        }
        else
        {
            throw ParseException(__FILE__, __LINE__, "Invalid discard policy: " + value);
        }
    }
}

Instance::Instance(CommunicatorPtr communicator, function<void(function<void()> call)> customExecutor)
    : _communicator(std::move(communicator))
{
    if (_communicator->getDefaultObjectAdapter())
    {
        throw invalid_argument(
            "communicator used to initialize a DataStorm node must not have a default object adapter");
    }

    PropertiesPtr properties = _communicator->getProperties();

    // Parse the topic default configurations eagerly, before any resource is created: an invalid DataStorm.Topic.*
    // property value then surfaces as a catchable exception from the Node constructor. Parsing these lazily when a
    // topic reader or writer is first created would let the exception escape the noexcept Topic methods
    // (hasWriters/hasReaders/setReaderDefaultConfig/setWriterDefaultConfig) and call std::terminate.
    _defaultReaderConfig.clearHistory = parseClearHistory(properties->getIceProperty("DataStorm.Topic.ClearHistory"));
    _defaultReaderConfig.sampleCount = properties->getIcePropertyAsInt("DataStorm.Topic.SampleCount");
    _defaultReaderConfig.sampleLifetime = properties->getIcePropertyAsInt("DataStorm.Topic.SampleLifetime");
    _defaultReaderConfig.discardPolicy =
        parseDiscardPolicy(properties->getIceProperty("DataStorm.Topic.DiscardPolicy"));

    _defaultWriterConfig.clearHistory = *_defaultReaderConfig.clearHistory;
    _defaultWriterConfig.sampleCount = *_defaultReaderConfig.sampleCount;
    _defaultWriterConfig.sampleLifetime = *_defaultReaderConfig.sampleLifetime;
    _defaultWriterConfig.priority = properties->getIcePropertyAsInt("DataStorm.Topic.Priority");

    _retryDelay = chrono::milliseconds(properties->getIcePropertyAsInt("DataStorm.Node.RetryDelay"));
    _retryMultiplier = properties->getIcePropertyAsInt("DataStorm.Node.RetryMultiplier");
    _retryCount = properties->getIcePropertyAsInt("DataStorm.Node.RetryCount");

    _traceLevels = make_shared<TraceLevels>(properties, _communicator->getLogger());
    _executor = make_shared<CallbackExecutor>(std::move(customExecutor));
    _connectionManager = make_shared<ConnectionManager>(_executor);
    _timer = make_shared<IceInternal::Timer>();
}

void
Instance::init(std::optional<Ice::SSL::ServerAuthenticationOptions> serverAuthenticationOptions)
{
    PropertiesPtr properties = _communicator->getProperties();

    if (properties->getIcePropertyAsInt("DataStorm.Node.Server.Enabled") > 0)
    {
        if (properties->getIceProperty("DataStorm.Node.Server.Endpoints").empty())
        {
            properties->setProperty("DataStorm.Node.Server.Endpoints", "tcp");
        }
        // Use a serialized thread pool to ensure that samples are processed in the order they are received. This is
        // especially important for PartialUpdate samples which depend on the order to compute the value.
        properties->setProperty("DataStorm.Node.Server.ThreadPool.Serialize", "1");

        try
        {
            _adapter =
                _communicator->createObjectAdapter("DataStorm.Node.Server", std::move(serverAuthenticationOptions));
        }
        catch (const LocalException& ex)
        {
            ostringstream os;
            os << "failed to listen on server endpoints '";
            os << properties->getIceProperty("DataStorm.Node.Server.Endpoints") << "':\n";
            os << ex.what();
            throw invalid_argument(os.str());
        }
    }
    else
    {
        _adapter = _communicator->createObjectAdapter("");
    }
    _communicator->setDefaultObjectAdapter(_adapter);

    if (properties->getIcePropertyAsInt("DataStorm.Node.Multicast.Enabled") > 0)
    {
        if (properties->getIceProperty("DataStorm.Node.Multicast.Endpoints").empty())
        {
            properties->setProperty("DataStorm.Node.Multicast.Endpoints", "udp -h 239.255.0.1 -p 10000");
            // Set the published host to the multicast address, ensuring that proxies are created with the multicast
            // address.
            properties->setProperty("DataStorm.Node.Multicast.PublishedHost", "239.255.0.1");
        }

        try
        {
            _multicastAdapter = _communicator->createObjectAdapter("DataStorm.Node.Multicast");
        }
        catch (const LocalException& ex)
        {
            ostringstream os;
            os << "failed to listen on multicast endpoints '";
            os << properties->getIceProperty("DataStorm.Node.Multicast.Endpoints") << "':\n";
            os << ex.what();
            throw invalid_argument(os.str());
        }
    }

    // A collocated adapter is used with a unique AdapterId to enable collocation with default servants. Proxies created
    // by this adapter will be indirect, and ObjectAdapter::isLocal will compare the adapter's AdapterId with the
    // reference's AdapterId to determine if a collocated call can be used.
    //
    // A named adapter is required because we cannot assign an AdapterId to a nameless adapter.
    auto collocated = generateUUID();
    properties->setProperty(collocated + ".AdapterId", collocated);
    _collocatedAdapter = _communicator->createObjectAdapter(collocated);

    _collocatedForwarder = make_shared<ForwarderManager>(_collocatedAdapter, "forwarders");
    _collocatedAdapter->addDefaultServant(_collocatedForwarder, "forwarders");

    auto self = shared_from_this();

    _topicFactory = make_shared<TopicFactoryI>(self);

    string name = _communicator->getProperties()->getIceProperty("DataStorm.Node.Name");
    if (name.empty())
    {
        name = generateUUID();
    }
    _node = make_shared<NodeI>(self, std::move(name));
    _node->init();

    _nodeSessionManager = make_shared<NodeSessionManager>(self, _node);
    _nodeSessionManager->init();

    auto lookupI = make_shared<LookupI>(_nodeSessionManager, _topicFactory, _node->getProxy());
    _adapter->add(lookupI, lookupIdentity);
    if (_multicastAdapter)
    {
        auto lookup = _multicastAdapter->add<DataStormContract::LookupPrx>(lookupI, lookupIdentity);
        // The lookup proxy can be customized by setting the property DataStorm.Node.Multicast.Proxy.
        if (!_communicator->getProperties()->getIceProperty("DataStorm.Node.Multicast.Proxy").empty())
        {
            // propertyToProxy only returns a nullopt proxy when the property is empty.
            lookup = *_communicator->propertyToProxy<DataStormContract::LookupPrx>("DataStorm.Node.Multicast.Proxy");
            if (lookup->ice_getIdentity() != lookupIdentity)
            {
                ostringstream os;
                os << "property 'DataStorm.Node.Multicast.Proxy' has an invalid value: the proxy identity must be '"
                   << identityToString(lookupIdentity) << "'";
                throw PropertyException{__FILE__, __LINE__, os.str()};
            }
        }
        _lookup = lookup->ice_datagram();
    }

    _adapter->activate();
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
    IceInternal::TimerPtr timer;
    {
        // Clear the timer before it is destroyed to avoid tasks being scheduled after the timer is destroyed.
        unique_lock<mutex> lock(_mutex);
        timer = _timer;
        _timer = nullptr;
    }

    if (timer)
    {
        timer->destroy();
    }

    _communicator->setDefaultObjectAdapter(nullptr);
    if (ownsCommunicator)
    {
        _communicator->destroy();
    }
    else
    {
        if (_adapter)
        {
            _adapter->destroy();
        }
        if (_collocatedAdapter)
        {
            _collocatedAdapter->destroy();
        }
        if (_multicastAdapter)
        {
            _multicastAdapter->destroy();
        }
    }
    if (_node)
    {
        _node->destroy(ownsCommunicator);
    }

    _executor->destroy();
    _connectionManager->destroy();
    if (_collocatedForwarder)
    {
        _collocatedForwarder->destroy();
    }
}
