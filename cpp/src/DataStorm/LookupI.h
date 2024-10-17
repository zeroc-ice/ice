//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_LOOKUPI_H
#define DATASTORM_LOOKUPI_H

#include "DataStorm/Contract.h"

namespace DataStormI
{

    class NodeSessionManager;
    class TopicFactoryI;

    class LookupI : public DataStormContract::Lookup
    {
    public:
        LookupI(std::shared_ptr<NodeSessionManager>, std::shared_ptr<TopicFactoryI>, DataStormContract::NodePrx);

        virtual void
        announceTopicReader(std::string, std::optional<DataStormContract::NodePrx>, const Ice::Current&) override;

        virtual void
        announceTopicWriter(std::string, std::optional<DataStormContract::NodePrx>, const Ice::Current&) override;

        virtual void announceTopics(
            DataStormContract::StringSeq,
            DataStormContract::StringSeq,
            std::optional<DataStormContract::NodePrx>,
            const Ice::Current&) override;

        virtual std::optional<DataStormContract::NodePrx>
        createSession(std::optional<DataStormContract::NodePrx>, const Ice::Current&) override;

    private:
        std::shared_ptr<NodeSessionManager> _nodeSessionManager;
        std::shared_ptr<TopicFactoryI> _topicFactory;
        DataStormContract::NodePrx _nodePrx;
    };

}
#endif
