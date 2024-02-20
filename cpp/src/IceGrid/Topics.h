//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_TOPICS_H
#define ICEGRID_TOPICS_H

#include <IceStorm/IceStorm.h>
#include <IceGrid/Internal.h>
#include <IceGrid/Registry.h>
#include <set>

namespace IceGrid
{

class ObserverTopic
{
public:

    ObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::string&, int64_t = 0);
    virtual ~ObserverTopic() = default;

    int subscribe(const Ice::ObjectPrxPtr&, const std::string& = std::string());
    void unsubscribe(const Ice::ObjectPrxPtr&, const std::string& = std::string());
    void destroy();

    void receivedUpdate(const std::string&, int, const std::string&);

    virtual void initObserver(const Ice::ObjectPrxPtr&) = 0;

    void waitForSyncedSubscribers(int, const std::string& = std::string());

    int getSerial() const;

protected:

    void addExpectedUpdate(int, const std::string& = std::string());
    void updateSerial(std::int64_t = 0);
    Ice::Context getContext(int, std::int64_t = 0) const;

    template<typename T> std::vector<std::optional<T>> getPublishers() const
    {
        std::vector<std::optional<T>> publishers;
        for(const auto& publisher : _basePublishers)
        {
            publishers.push_back(Ice::uncheckedCast<T>(publisher));
        }
        return publishers;
    }

    std::shared_ptr<Ice::Logger> _logger;
    std::map<Ice::EncodingVersion, IceStorm::TopicPrxPtr> _topics;
    std::vector<Ice::ObjectPrxPtr> _basePublishers;
    int _serial;
    std::int64_t _dbSerial;

    std::set<std::string> _syncSubscribers;
    std::map<int, std::set<std::string> > _waitForUpdates;
    std::map<int, std::map<std::string, std::string> > _updateFailures;

    mutable std::mutex _mutex;
    std::condition_variable _condVar;
};

class RegistryObserverTopic : public ObserverTopic
{
public:

    RegistryObserverTopic(const IceStorm::TopicManagerPrxPtr&);

    void registryUp(const RegistryInfo&);
    void registryDown(const std::string&);

    virtual void initObserver(const Ice::ObjectPrxPtr&);

private:

    std::vector<RegistryObserverPrxPtr> _publishers;
    std::map<std::string, RegistryInfo> _registries;
};

class NodeObserverTopic final : public ObserverTopic, public NodeObserver
{
public:

    static std::shared_ptr<NodeObserverTopic>
    create(const IceStorm::TopicManagerPrxPtr&, const std::shared_ptr<Ice::ObjectAdapter>&);

    void nodeInit(NodeDynamicInfoSeq, const Ice::Current&) override;
    void nodeUp(NodeDynamicInfo, const Ice::Current&) override;
    void nodeDown(std::string, const Ice::Current&) override;
    void updateServer(std::string, ServerDynamicInfo, const Ice::Current&) override;
    void updateAdapter(std::string, AdapterDynamicInfo, const Ice::Current&) override;

    const NodeObserverPrxPtr& getPublisher() { return _externalPublisher; }

    void nodeDown(const std::string&);
    void initObserver(const Ice::ObjectPrxPtr&) override;

    bool isServerEnabled(const std::string&) const;

private:

    NodeObserverTopic(const IceStorm::TopicManagerPrxPtr&);

    const NodeObserverPrxPtr _externalPublisher;
    std::vector<NodeObserverPrxPtr> _publishers;
    std::map<std::string, NodeDynamicInfo> _nodes;
    std::map<std::string, bool> _serverStatus;
};

class ApplicationObserverTopic : public ObserverTopic
{
public:

    ApplicationObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::map<std::string, ApplicationInfo>&, std::int64_t);

    int applicationInit(std::int64_t, const ApplicationInfoSeq&);
    int applicationAdded(std::int64_t, const ApplicationInfo&);
    int applicationRemoved(std::int64_t, const std::string&);
    int applicationUpdated(std::int64_t, const ApplicationUpdateInfo&);

    virtual void initObserver(const Ice::ObjectPrxPtr&);

private:

    std::vector<ApplicationObserverPrxPtr> _publishers;
    std::map<std::string, ApplicationInfo> _applications;
};

class AdapterObserverTopic : public ObserverTopic
{
public:

    AdapterObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::map<std::string, AdapterInfo>&, std::int64_t);

    int adapterInit(std::int64_t, const AdapterInfoSeq&);
    int adapterAdded(std::int64_t, const AdapterInfo&);
    int adapterUpdated(std::int64_t, const AdapterInfo&);
    int adapterRemoved(std::int64_t, const std::string&);

    virtual void initObserver(const Ice::ObjectPrxPtr&);

private:

    std::vector<AdapterObserverPrxPtr> _publishers;
    std::map<std::string, AdapterInfo> _adapters;
};

class ObjectObserverTopic : public ObserverTopic
{
public:

    ObjectObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::map<Ice::Identity, ObjectInfo>&, std::int64_t);

    int objectInit(std::int64_t, const ObjectInfoSeq&);
    int objectAdded(std::int64_t, const ObjectInfo&);
    int objectUpdated(std::int64_t, const ObjectInfo&);
    int objectRemoved(std::int64_t, const Ice::Identity&);

    int wellKnownObjectsAddedOrUpdated(const ObjectInfoSeq&);
    int wellKnownObjectsRemoved(const ObjectInfoSeq&);

    virtual void initObserver(const Ice::ObjectPrxPtr&);

private:

    std::vector<ObjectObserverPrxPtr> _publishers;
    std::map<Ice::Identity, ObjectInfo> _objects;
};

};

#endif
