// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_LOOKUPI_H
#define DATASTORM_LOOKUPI_H

#include "DataStorm/Contract.h"

namespace DataStormI
{
    class NodeSessionManager;
    class TopicFactoryI;

    class LookupI final : public DataStormContract::Lookup
    {
    public:
        LookupI(std::shared_ptr<NodeSessionManager>, std::shared_ptr<TopicFactoryI>, DataStormContract::NodePrx);

        void announceTopicReader(std::string, std::optional<DataStormContract::NodePrx>, const Ice::Current&) final;

        void announceTopicWriter(std::string, std::optional<DataStormContract::NodePrx>, const Ice::Current&) final;

        void
        announceTopics(Ice::StringSeq, Ice::StringSeq, std::optional<DataStormContract::NodePrx>, const Ice::Current&)
            final;

        std::optional<DataStormContract::NodePrx>
        createSession(std::optional<DataStormContract::NodePrx>, const Ice::Current&) final;

    private:
        std::shared_ptr<NodeSessionManager> _nodeSessionManager;
        std::shared_ptr<TopicFactoryI> _topicFactory;
        DataStormContract::NodePrx _nodePrx;
    };
}
#endif
