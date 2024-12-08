//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_TOPIC_FACTORYI_H
#define DATASTORM_TOPIC_FACTORYI_H

#include "DataStorm/Contract.h"
#include "DataStorm/InternalI.h"

namespace DataStormI
{
    class Instance;
    class TraceLevels;

    class TopicI;

    class TopicFactoryI final : public TopicFactory, public std::enable_shared_from_this<TopicFactoryI>
    {
    public:
        TopicFactoryI(std::shared_ptr<Instance>);

        std::shared_ptr<TopicReader> createTopicReader(
            std::string,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>) final;

        std::shared_ptr<TopicWriter> createTopicWriter(
            std::string,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>) final;

        Ice::CommunicatorPtr getCommunicator() const final;

        void removeTopicReader(const std::string&, const std::shared_ptr<TopicI>&);
        void removeTopicWriter(const std::string&, const std::shared_ptr<TopicI>&);

        std::vector<std::shared_ptr<TopicI>> getTopicReaders(const std::string&) const;
        std::vector<std::shared_ptr<TopicI>> getTopicWriters(const std::string&) const;

        void createSubscriberSession(const std::string&, DataStormContract::NodePrx, const Ice::ConnectionPtr&);
        void createPublisherSession(const std::string&, DataStormContract::NodePrx, const Ice::ConnectionPtr&);

        DataStormContract::TopicInfoSeq getTopicReaders() const;
        DataStormContract::TopicInfoSeq getTopicWriters() const;

        Ice::StringSeq getTopicReaderNames() const;
        Ice::StringSeq getTopicWriterNames() const;

        void shutdown() const;

    private:
        std::weak_ptr<Instance> _instance;
        mutable std::mutex _mutex;
        std::int64_t _nextReaderId;
        std::int64_t _nextWriterId;

        // A map of topic readers indexed by the topic name.
        // Each key is a topic name, and the corresponding value is a vector of readers for that topic.
        std::map<std::string, std::vector<std::shared_ptr<TopicI>>> _readers;

        // A map of topic writers indexed by the topic name.
        // Each key is a topic name, and the corresponding value is a vector of writers for that topic.
        std::map<std::string, std::vector<std::shared_ptr<TopicI>>> _writers;
    };
}
#endif
