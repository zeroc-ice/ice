// Copyright (c) ZeroC, Inc.

#include "Topics.h"
#include "DescriptorHelper.h"
#include "Ice/Ice.h"

using namespace std;
using namespace IceGrid;

namespace
{
    //
    // Encodings supported by the observers. We create one topic per
    // encoding version and subscribe the observer to the appropriate
    // topic depending on its encoding.
    //
    Ice::EncodingVersion encodings[] = {{1, 0}, {1, 1}};
}

ObserverTopic::ObserverTopic(const IceStorm::TopicManagerPrx& topicManager, const string& name, int64_t dbSerial)
    : _logger(topicManager->ice_getCommunicator()->getLogger()),
      _dbSerial(dbSerial)
{
    for (const auto& encoding : encodings)
    {
        ostringstream os;
        os << name << "-" << Ice::encodingVersionToString(encoding);

        optional<IceStorm::TopicPrx> topic;
        try
        {
            topic = topicManager->create(os.str());
        }
        catch (const IceStorm::TopicExists&)
        {
            topic = topicManager->retrieve(os.str());
        }

        if (!topic)
        {
            throw Ice::MarshalException(__FILE__, __LINE__, "failed to create or retrieve topic '" + os.str() + "'");
        }

        //
        // NOTE: collocation optimization needs to be turned on for the
        // topic because the subscribe() method is given a fixed proxy
        // which can't be marshaled.
        //
        _topics.insert({encoding, *topic});

        optional<Ice::ObjectPrx> publisher = topic->getPublisher();

        if (!publisher)
        {
            throw Ice::MarshalException(
                __FILE__,
                __LINE__,
                "failed to get publisher for topic '" + topic->ice_toString() + "'");
        }

        _basePublishers.push_back((*publisher)->ice_encodingVersion(encoding));
    }
}

int
ObserverTopic::subscribe(const Ice::ObjectPrx& observer, const string& name)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }

    try
    {
        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        Ice::EncodingVersion v = observer->ice_getEncodingVersion();
        auto p = _topics.find(v);
        if (p == _topics.end())
        {
            Ice::Warning out(_logger);
            out << "unsupported encoding version for observer '" << observer << "'";
            return -1;
        }
        auto publisher = p->second->subscribeAndGetPublisher(qos, observer->ice_twoway());
        if (!publisher)
        {
            ostringstream os;
            os << "topic: '" << p->second->ice_toString() << "' returned null publisher proxy.";
            throw Ice::MarshalException(__FILE__, __LINE__, os.str());
        }
        initObserver(*publisher);
    }
    catch (const IceStorm::AlreadySubscribed&)
    {
        throw ObserverAlreadyRegisteredException(observer->ice_getIdentity());
    }

    if (!name.empty())
    {
        assert(_syncSubscribers.find(name) == _syncSubscribers.end());
        _syncSubscribers.insert(name);
        addExpectedUpdate(_serial, name);
        return _serial;
    }
    return -1;
}

void
ObserverTopic::unsubscribe(const Ice::ObjectPrx& observer, const string& name)
{
    lock_guard lock(_mutex);
    Ice::EncodingVersion v = observer->ice_getEncodingVersion();
    auto q = _topics.find(v);
    if (q == _topics.end())
    {
        return;
    }
    try
    {
        q->second->unsubscribe(observer);
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
    }

    if (!name.empty())
    {
        assert(_syncSubscribers.find(name) != _syncSubscribers.end());
        _syncSubscribers.erase(name);

        auto p = _waitForUpdates.begin();
        bool notifyMonitor = false;
        while (p != _waitForUpdates.end())
        {
            p->second.erase(name);
            if (p->second.empty())
            {
                _waitForUpdates.erase(p++);
                notifyMonitor = true;
            }
            else
            {
                ++p;
            }
        }

        if (notifyMonitor)
        {
            _condVar.notify_all();
        }
    }
}

void
ObserverTopic::destroy()
{
    lock_guard lock(_mutex);
    _topics.clear();
    _condVar.notify_all();
}

void
ObserverTopic::receivedUpdate(const string& name, int serial, const string& failure)
{
    lock_guard lock(_mutex);
    auto p = _waitForUpdates.find(serial);
    if (p != _waitForUpdates.end())
    {
        p->second.erase(name);

        if (!failure.empty())
        {
            auto q = _updateFailures.find(serial);
            if (q == _updateFailures.end())
            {
                q = _updateFailures.insert(make_pair(serial, map<string, string>())).first;
            }
            q->second.insert(make_pair(name, failure));
        }

        if (p->second.empty())
        {
            _waitForUpdates.erase(p);
        }

        _condVar.notify_all();
    }
}

void
ObserverTopic::waitForSyncedSubscribers(int serial, const string& name)
{
    unique_lock lock(_mutex);

    if (serial < 0)
    {
        return;
    }

    //
    // Wait until all the updates are received or the service shutdown.
    //
    while (!_topics.empty())
    {
        auto p = _waitForUpdates.find(serial);
        if (p == _waitForUpdates.end())
        {
            auto q = _updateFailures.find(serial);
            if (q != _updateFailures.end())
            {
                map<string, string> failures = q->second;
                _updateFailures.erase(q);
                ostringstream os;
                for (const auto& failure : failures)
                {
                    os << "replication failed on replica '" << failure.first << "':\n" << failure.second << "\n";
                }

                Ice::Error err(_logger);
                err << os.str();
            }
            return;
        }
        else
        {
            if (!name.empty() && p->second.find(name) == p->second.end())
            {
                return;
            }
            _condVar.wait(lock);
        }
    }
}

int
ObserverTopic::getSerial() const
{
    lock_guard lock(_mutex);
    return _serial;
}

void
ObserverTopic::addExpectedUpdate(int serial, const string& name)
{
    if (_syncSubscribers.empty() && name.empty())
    {
        return;
    }

    // Must be called with the lock held.
    if (name.empty())
    {
        assert(_waitForUpdates[serial].empty());
        _waitForUpdates[serial] = _syncSubscribers;
    }
    else
    {
        _waitForUpdates[serial].insert(name);
    }
}

void
ObserverTopic::updateSerial(int64_t dbSerial)
{
    ++_serial;
    if (dbSerial > 0)
    {
        _dbSerial = dbSerial;
    }
}

Ice::Context
ObserverTopic::getContext(int serial, int64_t dbSerial) const
{
    Ice::Context context;
    {
        ostringstream os;
        os << serial;
        context["serial"] = os.str();
    }
    if (dbSerial > 0)
    {
        ostringstream os;
        os << dbSerial;
        context["dbSerial"] = os.str();
    }
    return context;
}

RegistryObserverTopic::RegistryObserverTopic(const IceStorm::TopicManagerPrx& topicManager)
    : ObserverTopic(topicManager, "RegistryObserver"),
      _publishers(getPublishers<RegistryObserverPrx>())
{
}

void
RegistryObserverTopic::registryUp(const RegistryInfo& info)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return;
    }
    updateSerial();
    _registries.insert({info.name, info});
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->registryUp(info);
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'registryUp' update:\n" << ex;
    }
}

void
RegistryObserverTopic::registryDown(const string& name)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return;
    }

    if (_registries.find(name) == _registries.end())
    {
        return;
    }

    updateSerial();
    _registries.erase(name);
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->registryDown(name);
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'registryDown' update:\n" << ex;
    }
}

void
RegistryObserverTopic::initObserver(Ice::ObjectPrx observer)
{
    auto registryObserver = Ice::uncheckedCast<RegistryObserverPrx>(observer);
    RegistryInfoSeq registries;
    registries.reserve(_registries.size());
    for (const auto& registry : _registries)
    {
        registries.push_back(registry.second);
    }
    registryObserver->registryInit(registries, getContext(_serial));
}

shared_ptr<NodeObserverTopic>
NodeObserverTopic::create(const IceStorm::TopicManagerPrx& topicManager, const Ice::ObjectAdapterPtr& adapter)
{
    Ice::Identity id{Ice::generateUUID(), ""};
    shared_ptr<NodeObserverTopic> topic(new NodeObserverTopic(topicManager, adapter->createProxy<NodeObserverPrx>(id)));
    adapter->add(topic, id);
    return topic;
}

NodeObserverTopic::NodeObserverTopic(const IceStorm::TopicManagerPrx& topicManager, NodeObserverPrx externalPublisher)
    : ObserverTopic(topicManager, "NodeObserver"),
      _externalPublisher(std::move(externalPublisher)),
      _publishers(getPublishers<NodeObserverPrx>())
{
}

void
NodeObserverTopic::nodeInit(NodeDynamicInfoSeq, const Ice::Current&)
{
    assert(false);
}

void
NodeObserverTopic::nodeUp(NodeDynamicInfo info, const Ice::Current&)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return;
    }
    updateSerial();
    _nodes.insert({info.info.name, info});
    for (const auto& server : info.servers)
    {
        _serverStatus[server.id] = server.enabled;
    }
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->nodeUp(info);
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'nodeUp' update:\n" << ex;
    }
}

void
NodeObserverTopic::nodeDown(string, const Ice::Current&)
{
    assert(false);
}

void
NodeObserverTopic::updateServer(string node, ServerDynamicInfo server, const Ice::Current&)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return;
    }

    if (_nodes.find(node) == _nodes.end())
    {
        //
        // If the node isn't known anymore, we ignore the update.
        //
        return;
    }

    updateSerial();

    ServerDynamicInfoSeq& servers = _nodes[node].servers;
    auto p = servers.begin();
    while (p != servers.end())
    {
        if (p->id == server.id)
        {
            if (server.state == ServerState::Destroyed || (server.state == ServerState::Inactive && server.enabled))
            {
                servers.erase(p);
            }
            else
            {
                *p = server;
            }
            break;
        }
        ++p;
    }
    if (server.state != ServerState::Destroyed && (server.state != ServerState::Inactive || !server.enabled) &&
        p == servers.end())
    {
        servers.push_back(server);
    }

    if (server.state != ServerState::Destroyed)
    {
        _serverStatus[server.id] = server.enabled;
    }
    else
    {
        _serverStatus.erase(server.id);
    }

    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->updateServer(node, server);
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'updateServer' update:\n" << ex;
    }
}

void
NodeObserverTopic::updateAdapter(string node, AdapterDynamicInfo adapter, const Ice::Current&)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return;
    }

    if (_nodes.find(node) == _nodes.end())
    {
        //
        // If the node isn't known anymore, we ignore the update.
        //
        return;
    }

    updateSerial();

    AdapterDynamicInfoSeq& adapters = _nodes[node].adapters;
    auto p = adapters.begin();
    while (p != adapters.end())
    {
        if (p->id == adapter.id)
        {
            if (adapter.proxy)
            {
                *p = adapter;
            }
            else
            {
                adapters.erase(p);
            }
            break;
        }
        ++p;
    }
    if (adapter.proxy && p == adapters.end())
    {
        adapters.push_back(adapter);
    }

    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->updateAdapter(node, adapter);
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'updateAdapter' update:\n" << ex;
    }
}

void
NodeObserverTopic::nodeDown(const string& name)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return;
    }

    updateSerial();

    if (_nodes.find(name) == _nodes.end())
    {
        return;
    }

    ServerDynamicInfoSeq& servers = _nodes[name].servers;
    for (const auto& server : servers)
    {
        _serverStatus.erase(server.id);
    }

    _nodes.erase(name);
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->nodeDown(name);
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'nodeDown' update:\n" << ex;
    }
}

void
NodeObserverTopic::initObserver(Ice::ObjectPrx observer)
{
    auto nodeObserver = Ice::uncheckedCast<NodeObserverPrx>(observer);
    NodeDynamicInfoSeq nodes;
    nodes.reserve(_nodes.size());
    for (const auto& node : _nodes)
    {
        nodes.push_back(node.second);
    }
    nodeObserver->nodeInit(nodes, getContext(_serial));
}

bool
NodeObserverTopic::isServerEnabled(const string& server) const
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return false;
    }
    auto p = _serverStatus.find(server);
    if (p != _serverStatus.end())
    {
        return p->second;
    }
    else
    {
        return true; // Assume the server is enabled if we don't know its status.
    }
}

ApplicationObserverTopic::ApplicationObserverTopic(
    const IceStorm::TopicManagerPrx& topicManager,
    const map<string, ApplicationInfo>& applications,
    int64_t serial)
    : ObserverTopic(topicManager, "ApplicationObserver", serial),
      _publishers(getPublishers<ApplicationObserverPrx>()),
      _applications(applications)
{
}

int
ApplicationObserverTopic::applicationInit(int64_t dbSerial, const ApplicationInfoSeq& apps)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _applications.clear();
    for (const auto& app : apps)
    {
        _applications.insert(make_pair(app.descriptor.name, app));
    }
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->applicationInit(_serial, apps, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'applicationInit' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ApplicationObserverTopic::applicationAdded(int64_t dbSerial, const ApplicationInfo& info)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }

    updateSerial(dbSerial);
    _applications.insert(make_pair(info.descriptor.name, info));
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->applicationAdded(_serial, info, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'applicationAdded' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ApplicationObserverTopic::applicationRemoved(int64_t dbSerial, const string& name)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _applications.erase(name);
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->applicationRemoved(_serial, name, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'applicationRemoved' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ApplicationObserverTopic::applicationUpdated(int64_t dbSerial, const ApplicationUpdateInfo& info)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }

    updateSerial(dbSerial);
    try
    {
        auto p = _applications.find(info.descriptor.name);
        if (p != _applications.end())
        {
            ApplicationHelper helper(_publishers[0]->ice_getCommunicator(), p->second.descriptor);
            p->second.descriptor = helper.update(info.descriptor);
            p->second.updateTime = info.updateTime;
            p->second.updateUser = info.updateUser;
            p->second.revision = info.revision;
        }
    }
    catch (const DeploymentException& ex)
    {
        Ice::Error out(_logger);
        out << "unexpected exception while instantiating application '" << info.descriptor.name << "':\n" << ex.reason;
        assert(false);
    }
    catch (const std::exception& ex)
    {
        Ice::Error out(_logger);
        out << "unexpected exception while instantiating application '" << info.descriptor.name << "':\n" << ex.what();
        assert(false);
    }
    catch (...)
    {
        Ice::Error out(_logger);
        out << "unexpected exception while instantiating application '" << info.descriptor.name << "'";
        assert(false);
    }
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->applicationUpdated(_serial, info, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'applicationUpdated' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

void
ApplicationObserverTopic::initObserver(Ice::ObjectPrx observer)
{
    auto applicationObserver = Ice::uncheckedCast<ApplicationObserverPrx>(observer);
    ApplicationInfoSeq applications;
    for (const auto& application : _applications)
    {
        applications.push_back(application.second);
    }
    applicationObserver->applicationInit(_serial, applications, getContext(_serial, _dbSerial));
}

AdapterObserverTopic::AdapterObserverTopic(
    const IceStorm::TopicManagerPrx& topicManager,
    const map<string, AdapterInfo>& adapters,
    int64_t serial)
    : ObserverTopic(topicManager, "AdapterObserver", serial),
      _publishers(getPublishers<AdapterObserverPrx>()),
      _adapters(adapters)
{
}

int
AdapterObserverTopic::adapterInit(int64_t dbSerial, const AdapterInfoSeq& adpts)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters.clear();
    for (const auto& adpt : adpts)
    {
        _adapters.insert({adpt.id, adpt});
    }
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->adapterInit(adpts, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'adapterInit' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
AdapterObserverTopic::adapterAdded(int64_t dbSerial, const AdapterInfo& info)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters.insert(make_pair(info.id, info));
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->adapterAdded(info, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'adapterAdded' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
AdapterObserverTopic::adapterUpdated(int64_t dbSerial, const AdapterInfo& info)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters[info.id] = info;
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->adapterUpdated(info, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'adapterUpdated' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
AdapterObserverTopic::adapterRemoved(int64_t dbSerial, const string& id)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters.erase(id);
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->adapterRemoved(id, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'adapterRemoved' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

void
AdapterObserverTopic::initObserver(Ice::ObjectPrx observer)
{
    auto adapterObserver = Ice::uncheckedCast<AdapterObserverPrx>(observer);
    AdapterInfoSeq adapters;
    for (const auto& adapter : _adapters)
    {
        adapters.push_back(adapter.second);
    }
    adapterObserver->adapterInit(adapters, getContext(_serial, _dbSerial));
}

ObjectObserverTopic::ObjectObserverTopic(
    const IceStorm::TopicManagerPrx& topicManager,
    const map<Ice::Identity, ObjectInfo>& objects,
    int64_t serial)
    : ObserverTopic(topicManager, "ObjectObserver", serial),
      _publishers(getPublishers<ObjectObserverPrx>()),
      _objects(objects)
{
}

int
ObjectObserverTopic::objectInit(int64_t dbSerial, const ObjectInfoSeq& objects)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects.clear();
    for (const auto& object : objects)
    {
        _objects.insert(make_pair(object.proxy->ice_getIdentity(), object));
    }
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->objectInit(objects, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'objectInit' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::objectAdded(int64_t dbSerial, const ObjectInfo& info)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->objectAdded(info, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'objectAdded' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::objectUpdated(int64_t dbSerial, const ObjectInfo& info)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects[info.proxy->ice_getIdentity()] = info;
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->objectUpdated(info, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'objectUpdated' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::objectRemoved(int64_t dbSerial, const Ice::Identity& id)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects.erase(id);
    try
    {
        for (const auto& publisher : _publishers)
        {
            publisher->objectRemoved(id, getContext(_serial, dbSerial));
        }
    }
    catch (const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing 'objectRemoved' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::wellKnownObjectsAddedOrUpdated(const ObjectInfoSeq& infos)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }

    for (const auto& info : infos)
    {
        updateSerial();
        auto q = _objects.find(info.proxy->ice_getIdentity());
        if (q != _objects.end())
        {
            q->second = info;
            try
            {
                for (const auto& publisher : _publishers)
                {
                    publisher->objectUpdated(info, getContext(_serial));
                }
            }
            catch (const Ice::LocalException& ex)
            {
                Ice::Warning out(_logger);
                out << "unexpected exception while publishing 'objectUpdated' update:\n" << ex;
            }
        }
        else
        {
            _objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
            try
            {
                for (const auto& publisher : _publishers)
                {
                    publisher->objectAdded(info, getContext(_serial));
                }
            }
            catch (const Ice::LocalException& ex)
            {
                Ice::Warning out(_logger);
                out << "unexpected exception while publishing 'objectAdded' update:\n" << ex;
            }
        }
    }

    //
    // We don't wait for the update to be received by the replicas
    // here. This operation is called by ReplicaSessionI.
    //
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::wellKnownObjectsRemoved(const ObjectInfoSeq& infos)
{
    lock_guard lock(_mutex);
    if (_topics.empty())
    {
        return -1;
    }

    for (const auto& info : infos)
    {
        updateSerial();
        _objects.erase(info.proxy->ice_getIdentity());
        try
        {
            for (const auto& publisher : _publishers)
            {
                publisher->objectRemoved(info.proxy->ice_getIdentity(), getContext(_serial));
            }
        }
        catch (const Ice::LocalException& ex)
        {
            Ice::Warning out(_logger);
            out << "unexpected exception while publishing 'objectUpdated' update:\n" << ex;
        }
    }

    //
    // We don't need to wait for the update to be received by the
    // replicas here. This operation is only called internally by
    // IceGrid.
    //
    addExpectedUpdate(_serial);
    return _serial;
}

void
ObjectObserverTopic::initObserver(Ice::ObjectPrx observer)
{
    auto objectObserver = Ice::uncheckedCast<ObjectObserverPrx>(observer);
    ObjectInfoSeq objects;
    for (const auto& object : _objects)
    {
        objects.push_back(object.second);
    }
    objectObserver->objectInit(objects, getContext(_serial, _dbSerial));
}
