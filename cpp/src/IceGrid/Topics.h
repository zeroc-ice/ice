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

    ObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::string&, long long = 0);
    virtual ~ObserverTopic() = default;

    int subscribe(const Ice::ObjectPrx&, const std::string& = std::string());
    void unsubscribe(const Ice::ObjectPrx&, const std::string& = std::string());
    void destroy();

    void receivedUpdate(const std::string&, int, const std::string&);

    virtual void initObserver(const Ice::ObjectPrx&) = 0;

    void waitForSyncedSubscribers(int, const std::string& = std::string());

    int getSerial() const;

protected:

    void addExpectedUpdate(int, const std::string& = std::string());
    void updateSerial(long long = 0);
    Ice::Context getContext(int, long long = 0) const;

    template<typename T> std::vector<std::shared_ptr<T>> getPublishers() const
    {
        std::vector<std::shared_ptr<T>> publishers;
        for(const auto& publisher :_basePublishers)
        {
            publishers.push_back(Ice::uncheckedCast<T>(publisher));
        }
        return publishers;
    }

    std::shared_ptr<Ice::Logger> _logger;
    std::map<Ice::EncodingVersion, IceStorm::TopicPrxPtr> _topics;
    std::vector<Ice::ObjectPrx> _basePublishers;
    int _serial;
    long long _dbSerial;

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

    virtual void initObserver(const Ice::ObjectPrx&);

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
    void initObserver(const Ice::ObjectPrx&) override;

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

    ApplicationObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::map<std::string, ApplicationInfo>&, long long);

    int applicationInit(long long, const ApplicationInfoSeq&);
    int applicationAdded(long long, const ApplicationInfo&);
    int applicationRemoved(long long, const std::string&);
    int applicationUpdated(long long, const ApplicationUpdateInfo&);

    virtual void initObserver(const Ice::ObjectPrx&);

private:

    std::vector<ApplicationObserverPrxPtr> _publishers;
    std::map<std::string, ApplicationInfo> _applications;
};

class AdapterObserverTopic : public ObserverTopic
{
public:

    AdapterObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::map<std::string, AdapterInfo>&, long long);

    int adapterInit(long long, const AdapterInfoSeq&);
    int adapterAdded(long long, const AdapterInfo&);
    int adapterUpdated(long long, const AdapterInfo&);
    int adapterRemoved(long long, const std::string&);

    virtual void initObserver(const Ice::ObjectPrx&);

private:

    std::vector<AdapterObserverPrxPtr> _publishers;
    std::map<std::string, AdapterInfo> _adapters;
};

class ObjectObserverTopic : public ObserverTopic
{
public:

    ObjectObserverTopic(const IceStorm::TopicManagerPrxPtr&, const std::map<Ice::Identity, ObjectInfo>&, long long);

    int objectInit(long long, const ObjectInfoSeq&);
    int objectAdded(long long, const ObjectInfo&);
    int objectUpdated(long long, const ObjectInfo&);
    int objectRemoved(long long, const Ice::Identity&);

    int wellKnownObjectsAddedOrUpdated(const ObjectInfoSeq&);
    int wellKnownObjectsRemoved(const ObjectInfoSeq&);

    virtual void initObserver(const Ice::ObjectPrx&);

private:

    std::vector<ObjectObserverPrxPtr> _publishers;
    std::map<Ice::Identity, ObjectInfo> _objects;
};

};

#endif
