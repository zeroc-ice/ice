//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/Topics.h>
#include <IceGrid/DescriptorHelper.h>

using namespace std;
using namespace IceGrid;

namespace
{

//
// Encodings supported by the observers. We create one topic per
// encoding version and subscribe the observer to the appropriate
// topic depending on its encoding.
//
Ice::EncodingVersion encodings[] = {
    { 1, 0 },
    { 1, 1 }
};

}

ObserverTopic::ObserverTopic(const shared_ptr<IceStorm::TopicManagerPrx>& topicManager, const string& name,
                             long long dbSerial) :
    _logger(topicManager->ice_getCommunicator()->getLogger()), _serial(0), _dbSerial(dbSerial)
{
    for(int i = 0; i < static_cast<int>(sizeof(encodings) / sizeof(Ice::EncodingVersion)); ++i)
    {
        ostringstream os;
        os << name << "-" << Ice::encodingVersionToString(encodings[i]);
        shared_ptr<IceStorm::TopicPrx> t;
        try
        {
            t = topicManager->create(os.str());
        }
        catch(const IceStorm::TopicExists&)
        {
            t = topicManager->retrieve(os.str());
        }

        //
        // NOTE: collocation optimization needs to be turned on for the
        // topic because the subscribe() method is given a fixed proxy
        // which can't be marshalled.
        //
        _topics[encodings[i]] = t;
        _basePublishers.push_back(t->getPublisher()->ice_encodingVersion(encodings[i]));
    }
}

int
ObserverTopic::subscribe(const shared_ptr<Ice::ObjectPrx>& obsv, const string& name)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }

    assert(obsv);
    try
    {
        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        Ice::EncodingVersion v = IceInternal::getCompatibleEncoding(obsv->ice_getEncodingVersion());
        auto p = _topics.find(v);
        if(p == _topics.end())
        {
            Ice::Warning out(_logger);
            out << "unsupported encoding version for observer `" << obsv << "'";
            return -1;
        }
        initObserver(p->second->subscribeAndGetPublisher(qos, obsv->ice_twoway()));
    }
    catch(const IceStorm::AlreadySubscribed&)
    {
        throw ObserverAlreadyRegisteredException(obsv->ice_getIdentity());
    }

    if(!name.empty())
    {
        assert(_syncSubscribers.find(name) == _syncSubscribers.end());
        _syncSubscribers.insert(name);
        addExpectedUpdate(_serial, name);
        return _serial;
    }
    return -1;
}

void
ObserverTopic::unsubscribe(const shared_ptr<Ice::ObjectPrx>& observer, const string& name)
{
    lock_guard lock(_mutex);
    Ice::EncodingVersion v = IceInternal::getCompatibleEncoding(observer->ice_getEncodingVersion());
    auto q = _topics.find(v);
    if(q == _topics.end())
    {
        return;
    }
    try
    {
        q->second->unsubscribe(observer);
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }

    assert(observer);

    if(!name.empty())
    {
        assert(_syncSubscribers.find(name) != _syncSubscribers.end());
        _syncSubscribers.erase(name);

        auto p = _waitForUpdates.begin();
        bool notifyMonitor = false;
        while(p != _waitForUpdates.end())
        {
            p->second.erase(name);
            if(p->second.empty())
            {
                _waitForUpdates.erase(p++);
                notifyMonitor = true;
            }
            else
            {
                ++p;
            }
        }

        if(notifyMonitor)
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
    map<int, set<string> >::iterator p = _waitForUpdates.find(serial);
    if(p != _waitForUpdates.end())
    {
        p->second.erase(name);

        if(!failure.empty())
        {
            map<int, map<string, string> >::iterator q = _updateFailures.find(serial);
            if(q == _updateFailures.end())
            {
                q = _updateFailures.insert(make_pair(serial, map<string ,string>())).first;
            }
            q->second.insert(make_pair(name, failure));
        }

        if(p->second.empty())
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

    if(serial < 0)
    {
        return;
    }

    //
    // Wait until all the updates are received or the service shutdown.
    //
    while(!_topics.empty())
    {
        auto p = _waitForUpdates.find(serial);
        if(p == _waitForUpdates.end())
        {
            auto q = _updateFailures.find(serial);
            if(q != _updateFailures.end())
            {
                map<string, string> failures = q->second;
                _updateFailures.erase(q);
                ostringstream os;
                for(map<string, string>::const_iterator r = failures.begin(); r != failures.end(); ++r)
                {
                    os << "replication failed on replica `" << r->first << "':\n" << r->second << "\n";
                }

                Ice::Error err(_logger);
                err << os.str();
            }
            return;
        }
        else
        {
            if(!name.empty() && p->second.find(name) == p->second.end())
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
    if(_syncSubscribers.empty() && name.empty())
    {
        return;
    }

    // Must be called with the lock held.
    if(name.empty())
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
ObserverTopic::updateSerial(long long dbSerial)
{
    ++_serial;
    if(dbSerial > 0)
    {
        _dbSerial = dbSerial;
    }
}

Ice::Context
ObserverTopic::getContext(int serial, long long dbSerial) const
{
    Ice::Context context;
    {
        ostringstream os;
        os << serial;
        context["serial"] = os.str();
    }
    if(dbSerial > 0)
    {
        ostringstream os;
        os << dbSerial;
        context["dbSerial"] = os.str();
    }
    return context;
}

RegistryObserverTopic::RegistryObserverTopic(const shared_ptr<IceStorm::TopicManagerPrx>& topicManager) :
    ObserverTopic(topicManager, "RegistryObserver")
{
    _publishers = getPublishers<RegistryObserverPrx>();
}

void
RegistryObserverTopic::registryUp(const RegistryInfo& info)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return;
    }
    updateSerial();
    _registries.insert({ info.name, info });
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->registryUp(info);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `registryUp' update:\n" << ex;
    }
}

void
RegistryObserverTopic::registryDown(const string& name)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return;
    }

    if(_registries.find(name) == _registries.end())
    {
        return;
    }

    updateSerial();
    _registries.erase(name);
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->registryDown(name);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `registryDown' update:\n" << ex;
    }
}

void
RegistryObserverTopic::initObserver(const shared_ptr<Ice::ObjectPrx>& obsv)
{
    auto observer = Ice::uncheckedCast<RegistryObserverPrx>(obsv);
    RegistryInfoSeq registries;
    registries.reserve(_registries.size());
    for(const auto& registry : _registries)
    {
        registries.push_back(registry.second);
    }
    observer->registryInit(registries, getContext(_serial));
}

shared_ptr<NodeObserverTopic>
NodeObserverTopic::create(const shared_ptr<IceStorm::TopicManagerPrx>& topicManager,
                          const shared_ptr<Ice::ObjectAdapter>& adapter)
{
    shared_ptr<NodeObserverTopic> topic(new NodeObserverTopic(topicManager));

    try
    {
        const_cast<shared_ptr<NodeObserverPrx>&>(topic->_externalPublisher) =
            Ice::uncheckedCast<NodeObserverPrx>(adapter->addWithUUID(topic));
    }
    catch(const Ice::LocalException&)
    {
    }

    return topic;
}

NodeObserverTopic::NodeObserverTopic(const shared_ptr<IceStorm::TopicManagerPrx>& topicManager) :
    ObserverTopic(topicManager, "NodeObserver")
{
    _publishers = getPublishers<NodeObserverPrx>();
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
    if(_topics.empty())
    {
        return;
    }
    updateSerial();
    _nodes.insert({ info.info.name, info });
    for(const auto& server : info.servers)
    {
        _serverStatus[server.id] = server.enabled;
    }
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->nodeUp(info);
        }
    }
    catch(const Ice::LocalException& ex)
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
    if(_topics.empty())
    {
        return;
    }

    if(_nodes.find(node) == _nodes.end())
    {
        //
        // If the node isn't known anymore, we ignore the update.
        //
        return;
    }

    updateSerial();

    ServerDynamicInfoSeq& servers = _nodes[node].servers;
    ServerDynamicInfoSeq::iterator p = servers.begin();
    while(p != servers.end())
    {
        if(p->id == server.id)
        {
            if(server.state == ServerState::Destroyed || (server.state == ServerState::Inactive && server.enabled))
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
    if(server.state != ServerState::Destroyed && (server.state != ServerState::Inactive
                                                  || !server.enabled) && p == servers.end())
    {
        servers.push_back(server);
    }

    if(server.state != ServerState::Destroyed)
    {
        _serverStatus[server.id] = server.enabled;
    }
    else
    {
        _serverStatus.erase(server.id);
    }

    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->updateServer(node, server);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `updateServer' update:\n" << ex;
    }
}

void
NodeObserverTopic::updateAdapter(string node, AdapterDynamicInfo adapter, const Ice::Current&)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return;
    }

    if(_nodes.find(node) == _nodes.end())
    {
        //
        // If the node isn't known anymore, we ignore the update.
        //
        return;
    }

    updateSerial();

    AdapterDynamicInfoSeq& adapters = _nodes[node].adapters;
    AdapterDynamicInfoSeq::iterator p = adapters.begin();
    while(p != adapters.end())
    {
        if(p->id == adapter.id)
        {
            if(adapter.proxy)
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
    if(adapter.proxy && p == adapters.end())
    {
        adapters.push_back(adapter);
    }

    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->updateAdapter(node, adapter);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `updateAdapter' update:\n" << ex;
    }
}

void
NodeObserverTopic::nodeDown(const string& name)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return;
    }

    updateSerial();

    if(_nodes.find(name) == _nodes.end())
    {
        return;
    }

    ServerDynamicInfoSeq& servers = _nodes[name].servers;
    for(const auto& server : servers)
    {
        _serverStatus.erase(server.id);
    }

    _nodes.erase(name);
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->nodeDown(name);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `nodeDown' update:\n" << ex;
    }
}

void
NodeObserverTopic::initObserver(const shared_ptr<Ice::ObjectPrx>& obsv)
{
    auto observer = Ice::uncheckedCast<NodeObserverPrx>(obsv);
    NodeDynamicInfoSeq nodes;
    nodes.reserve(_nodes.size());
    for(const auto& node : _nodes )
    {
        nodes.push_back(node.second);
    }
    observer->nodeInit(nodes, getContext(_serial));
}

bool
NodeObserverTopic::isServerEnabled(const string& server) const
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return false;
    }
    map<string, bool>::const_iterator p = _serverStatus.find(server);
    if(p != _serverStatus.end())
    {
        return p->second;
    }
    else
    {
        return true; // Assume the server is enabled if we don't know its status.
    }
}

ApplicationObserverTopic::ApplicationObserverTopic(const shared_ptr<IceStorm::TopicManagerPrx>& topicManager,
                                                   const map<string, ApplicationInfo>& applications, long long serial) :
    ObserverTopic(topicManager, "ApplicationObserver", serial),
    _applications(applications)
{
    _publishers = getPublishers<ApplicationObserverPrx>();
}

int
ApplicationObserverTopic::applicationInit(long long dbSerial, const ApplicationInfoSeq& apps)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _applications.clear();
    for(ApplicationInfoSeq::const_iterator p = apps.begin(); p != apps.end(); ++p)
    {
        _applications.insert(make_pair(p->descriptor.name, *p));
    }
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->applicationInit(_serial, apps, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `applicationInit' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ApplicationObserverTopic::applicationAdded(long long dbSerial, const ApplicationInfo& info)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }

    updateSerial(dbSerial);
    _applications.insert(make_pair(info.descriptor.name, info));
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->applicationAdded(_serial, info, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `applicationAdded' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ApplicationObserverTopic::applicationRemoved(long long dbSerial, const string& name)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _applications.erase(name);
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->applicationRemoved(_serial, name, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `applicationRemoved' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ApplicationObserverTopic::applicationUpdated(long long dbSerial, const ApplicationUpdateInfo& info)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }

    updateSerial(dbSerial);
    try
    {
        map<string, ApplicationInfo>::iterator p = _applications.find(info.descriptor.name);
        if(p != _applications.end())
        {
            ApplicationHelper helper(_publishers[0]->ice_getCommunicator(), p->second.descriptor);
            p->second.descriptor = helper.update(info.descriptor);
            p->second.updateTime = info.updateTime;
            p->second.updateUser = info.updateUser;
            p->second.revision = info.revision;
        }
    }
    catch(const DeploymentException& ex)
    {
        Ice::Error out(_logger);
        out << "unexpected exception while instantiating application `" << info.descriptor.name << "':\n" << ex.reason;
        assert(false);
    }
    catch(const std::exception& ex)
    {
        Ice::Error out(_logger);
        out << "unexpected exception while instantiating application `" << info.descriptor.name << "':\n" << ex.what();
        assert(false);
    }
    catch(...)
    {
        Ice::Error out(_logger);
        out << "unexpected exception while instantiating application `" << info.descriptor.name << "'";
        assert(false);
    }
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->applicationUpdated(_serial, info, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `applicationUpdated' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

void
ApplicationObserverTopic::initObserver(const shared_ptr<Ice::ObjectPrx>& obsv)
{
    auto observer = Ice::uncheckedCast<ApplicationObserverPrx>(obsv);
    ApplicationInfoSeq applications;
    for(const auto& application : _applications)
    {
        applications.push_back(application.second);
    }
    observer->applicationInit(_serial, applications, getContext(_serial, _dbSerial));
}

AdapterObserverTopic::AdapterObserverTopic(const shared_ptr<IceStorm::TopicManagerPrx>& topicManager,
                                           const map<string, AdapterInfo>& adapters, long long serial) :
    ObserverTopic(topicManager, "AdapterObserver", serial),
    _adapters(adapters)
{
    _publishers = getPublishers<AdapterObserverPrx>();
}

int
AdapterObserverTopic::adapterInit(long long dbSerial, const AdapterInfoSeq& adpts)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters.clear();
    for(const auto& adpt : adpts)
    {
        _adapters.insert({ adpt.id, adpt });
    }
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->adapterInit(adpts, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `adapterInit' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
AdapterObserverTopic::adapterAdded(long long dbSerial, const AdapterInfo& info)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters.insert(make_pair(info.id, info));
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->adapterAdded(info, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `adapterAdded' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
AdapterObserverTopic::adapterUpdated(long long dbSerial, const AdapterInfo& info)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters[info.id] = info;
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->adapterUpdated(info, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `adapterUpdated' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
AdapterObserverTopic::adapterRemoved(long long dbSerial, const string& id)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _adapters.erase(id);
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->adapterRemoved(id, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `adapterRemoved' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

void
AdapterObserverTopic::initObserver(const shared_ptr<Ice::ObjectPrx>& obsv)
{
    auto observer = Ice::uncheckedCast<AdapterObserverPrx>(obsv);
    AdapterInfoSeq adapters;
    for(const auto& adapter : _adapters)
    {
        adapters.push_back(adapter.second);
    }
    observer->adapterInit(adapters, getContext(_serial, _dbSerial));
}

ObjectObserverTopic::ObjectObserverTopic(const shared_ptr<IceStorm::TopicManagerPrx>& topicManager,
                                         const map<Ice::Identity, ObjectInfo>& objects, long long serial) :
    ObserverTopic(topicManager, "ObjectObserver", serial),
    _objects(objects)
{
    _publishers = getPublishers<ObjectObserverPrx>();
}

int
ObjectObserverTopic::objectInit(long long dbSerial, const ObjectInfoSeq& objects)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects.clear();
    for(const auto& object : objects)
    {
        _objects.insert(make_pair(object.proxy->ice_getIdentity(), object));
    }
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->objectInit(objects, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `objectInit' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::objectAdded(long long dbSerial, const ObjectInfo& info)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->objectAdded(info, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `objectAdded' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::objectUpdated(long long dbSerial, const ObjectInfo& info)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects[info.proxy->ice_getIdentity()] = info;
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->objectUpdated(info, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `objectUpdated' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::objectRemoved(long long dbSerial, const Ice::Identity& id)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }
    updateSerial(dbSerial);
    _objects.erase(id);
    try
    {
        for(const auto& publisher : _publishers)
        {
            publisher->objectRemoved(id, getContext(_serial, dbSerial));
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_logger);
        out << "unexpected exception while publishing `objectRemoved' update:\n" << ex;
    }
    addExpectedUpdate(_serial);
    return _serial;
}

int
ObjectObserverTopic::wellKnownObjectsAddedOrUpdated(const ObjectInfoSeq& infos)
{
    lock_guard lock(_mutex);
    if(_topics.empty())
    {
        return -1;
    }

    for(const auto& info : infos)
    {
        updateSerial();
        auto q = _objects.find(info.proxy->ice_getIdentity());
        if(q != _objects.end())
        {
            q->second = info;
            try
            {
                for(const auto& publisher : _publishers)
                {
                    publisher->objectUpdated(info, getContext(_serial));
                }
            }
            catch(const Ice::LocalException& ex)
            {
                Ice::Warning out(_logger);
                out << "unexpected exception while publishing `objectUpdated' update:\n" << ex;
            }
        }
        else
        {
            _objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
            try
            {
                for(const auto& publisher : _publishers)
                {
                    publisher->objectAdded(info, getContext(_serial));
                }
            }
            catch(const Ice::LocalException& ex)
            {
                Ice::Warning out(_logger);
                out << "unexpected exception while publishing `objectAdded' update:\n" << ex;
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
    if(_topics.empty())
    {
        return -1;
    }

    for(const auto& info : infos)
    {
        updateSerial();
        _objects.erase(info.proxy->ice_getIdentity());
        try
        {
            for(const auto& publisher : _publishers)
            {
                publisher->objectRemoved(info.proxy->ice_getIdentity(), getContext(_serial));
            }
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Warning out(_logger);
            out << "unexpected exception while publishing `objectUpdated' update:\n" << ex;
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
ObjectObserverTopic::initObserver(const shared_ptr<Ice::ObjectPrx>& obsv)
{
    auto observer = Ice::uncheckedCast<ObjectObserverPrx>(obsv);
    ObjectInfoSeq objects;
    for(const auto& object : _objects)
    {
        objects.push_back(object.second);
    }
    observer->objectInit(objects, getContext(_serial, _dbSerial));
}
