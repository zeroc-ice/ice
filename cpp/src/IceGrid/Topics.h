// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_TOPICS_H
#define ICEGRID_TOPICS_H

#include "IceGrid/Registry.h"
#include "IceStorm/IceStorm.h"
#include "Internal.h"
#include <set>

namespace IceGrid
{
    class ObserverTopic
    {
    public:
        ObserverTopic(const IceStorm::TopicManagerPrx&, const std::string&, int64_t = 0);
        virtual ~ObserverTopic() = default;

        int subscribe(const Ice::ObjectPrx&, const std::string& = std::string());
        void unsubscribe(const Ice::ObjectPrx&, const std::string& = std::string());
        void destroy();

        void receivedUpdate(const std::string&, int, const std::string&);

        virtual void initObserver(Ice::ObjectPrx) = 0;

        void waitForSyncedSubscribers(int, const std::string& = std::string());

        [[nodiscard]] int getSerial() const;

    protected:
        void addExpectedUpdate(int, const std::string& = std::string());
        void updateSerial(std::int64_t = 0);
        [[nodiscard]] Ice::Context getContext(int, std::int64_t = 0) const;

        template<typename T> [[nodiscard]] std::vector<T> getPublishers() const
        {
            std::vector<T> publishers;
            publishers.reserve(_basePublishers.size());
            for (const auto& publisher : _basePublishers)
            {
                publishers.push_back(Ice::uncheckedCast<T>(publisher));
            }
            return publishers;
        }

        Ice::LoggerPtr _logger;
        std::map<Ice::EncodingVersion, IceStorm::TopicPrx> _topics;
        std::vector<Ice::ObjectPrx> _basePublishers;
        int _serial{0};
        std::int64_t _dbSerial;

        std::set<std::string> _syncSubscribers;
        std::map<int, std::set<std::string>> _waitForUpdates;
        std::map<int, std::map<std::string, std::string>> _updateFailures;

        mutable std::mutex _mutex;
        std::condition_variable _condVar;
    };

    class RegistryObserverTopic final : public ObserverTopic
    {
    public:
        RegistryObserverTopic(const IceStorm::TopicManagerPrx&);

        void registryUp(const RegistryInfo&);
        void registryDown(const std::string&);

        void initObserver(Ice::ObjectPrx) final;

    private:
        std::vector<RegistryObserverPrx> _publishers;
        std::map<std::string, RegistryInfo> _registries;
    };

    class NodeObserverTopic final : public ObserverTopic, public NodeObserver
    {
    public:
        static std::shared_ptr<NodeObserverTopic>
        create(const IceStorm::TopicManagerPrx&, const Ice::ObjectAdapterPtr&);

        void nodeInit(NodeDynamicInfoSeq, const Ice::Current&) override;
        void nodeUp(NodeDynamicInfo, const Ice::Current&) override;
        void nodeDown(std::string, const Ice::Current&) override;
        void updateServer(std::string, ServerDynamicInfo, const Ice::Current&) override;
        void updateAdapter(std::string, AdapterDynamicInfo, const Ice::Current&) override;

        const NodeObserverPrx& getPublisher() { return _externalPublisher; }

        void nodeDown(const std::string&);
        void initObserver(Ice::ObjectPrx) final;

        [[nodiscard]] bool isServerEnabled(const std::string&) const;

    private:
        NodeObserverTopic(const IceStorm::TopicManagerPrx&, NodeObserverPrx);

        const NodeObserverPrx _externalPublisher;
        std::vector<NodeObserverPrx> _publishers;
        std::map<std::string, NodeDynamicInfo> _nodes;
        std::map<std::string, bool> _serverStatus;
    };

    class ApplicationObserverTopic final : public ObserverTopic
    {
    public:
        ApplicationObserverTopic(
            const IceStorm::TopicManagerPrx&,
            const std::map<std::string, ApplicationInfo>&,
            std::int64_t);

        int applicationInit(std::int64_t, const ApplicationInfoSeq&);
        int applicationAdded(std::int64_t, const ApplicationInfo&);
        int applicationRemoved(std::int64_t, const std::string&);
        int applicationUpdated(std::int64_t, const ApplicationUpdateInfo&);

        void initObserver(Ice::ObjectPrx) final;

    private:
        std::vector<ApplicationObserverPrx> _publishers;
        std::map<std::string, ApplicationInfo> _applications;
    };

    class AdapterObserverTopic final : public ObserverTopic
    {
    public:
        AdapterObserverTopic(const IceStorm::TopicManagerPrx&, const std::map<std::string, AdapterInfo>&, std::int64_t);

        int adapterInit(std::int64_t, const AdapterInfoSeq&);
        int adapterAdded(std::int64_t, const AdapterInfo&);
        int adapterUpdated(std::int64_t, const AdapterInfo&);
        int adapterRemoved(std::int64_t, const std::string&);

        void initObserver(Ice::ObjectPrx) final;

    private:
        std::vector<AdapterObserverPrx> _publishers;
        std::map<std::string, AdapterInfo> _adapters;
    };

    class ObjectObserverTopic final : public ObserverTopic
    {
    public:
        ObjectObserverTopic(const IceStorm::TopicManagerPrx&, const std::map<Ice::Identity, ObjectInfo>&, std::int64_t);

        int objectInit(std::int64_t, const ObjectInfoSeq&);
        int objectAdded(std::int64_t, const ObjectInfo&);
        int objectUpdated(std::int64_t, const ObjectInfo&);
        int objectRemoved(std::int64_t, const Ice::Identity&);

        int wellKnownObjectsAddedOrUpdated(const ObjectInfoSeq&);
        int wellKnownObjectsRemoved(const ObjectInfoSeq&);

        void initObserver(Ice::ObjectPrx) final;

    private:
        std::vector<ObjectObserverPrx> _publishers;
        std::map<Ice::Identity, ObjectInfo> _objects;
    };

};

#endif
