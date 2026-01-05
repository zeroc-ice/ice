// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_TRANSIENT_TOPIC_MANAGER_I_H
#define ICESTORM_TRANSIENT_TOPIC_MANAGER_I_H

#include "IceStormInternal.h"

namespace IceStorm
{
    class Instance;
    class TransientTopicImpl;

    class TransientTopicManagerImpl final : public TopicManagerInternal
    {
    public:
        TransientTopicManagerImpl(std::shared_ptr<Instance>);

        // TopicManager methods.
        std::optional<TopicPrx> create(std::string, const Ice::Current&) final;
        std::optional<TopicPrx> retrieve(std::string, const Ice::Current&) final;
        std::optional<TopicPrx> createOrRetrieve(std::string name, const Ice::Current&) final;

        TopicDict retrieveAll(const Ice::Current&) final;
        [[nodiscard]] std::optional<IceStormElection::NodePrx> getReplicaNode(const Ice::Current&) const final;

        void reap();
        void shutdown();

    private:
        using TopicMap = std::map<std::string, std::shared_ptr<TransientTopicImpl>>;

        std::optional<TopicPrx> createImpl(std::string);
        void removeDestroyedTopic(TopicMap::iterator);

        const std::shared_ptr<Instance> _instance;
        TopicMap _topics;

        std::mutex _mutex;
    };

} // End namespace IceStorm

#endif
