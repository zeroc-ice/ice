// Copyright (c) ZeroC, Inc.

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
        TopicFactoryI(const std::shared_ptr<Instance>&);

        [[nodiscard]] std::shared_ptr<TopicReader> createTopicReader(
            std::string,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>) final;

        [[nodiscard]] std::shared_ptr<TopicWriter> createTopicWriter(
            std::string,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>) final;

        [[nodiscard]] Ice::CommunicatorPtr getCommunicator() const final;

        void removeTopicReader(const std::string&, const std::shared_ptr<TopicI>&);
        void removeTopicWriter(const std::string&, const std::shared_ptr<TopicI>&);

        std::vector<std::shared_ptr<TopicI>> getTopicReaders(const std::string&) const;
        std::vector<std::shared_ptr<TopicI>> getTopicWriters(const std::string&) const;

        void createSubscriberSession(const std::string&, const DataStormContract::NodePrx&, const Ice::ConnectionPtr&);
        void createPublisherSession(const std::string&, const DataStormContract::NodePrx&, const Ice::ConnectionPtr&);

        [[nodiscard]] DataStormContract::TopicInfoSeq getTopicReaders() const;
        [[nodiscard]] DataStormContract::TopicInfoSeq getTopicWriters() const;

        [[nodiscard]] Ice::StringSeq getTopicReaderNames() const;
        [[nodiscard]] Ice::StringSeq getTopicWriterNames() const;

        void shutdown() const;

    private:
        std::weak_ptr<Instance> _instance;
        mutable std::mutex _mutex;
        std::int64_t _nextReaderId{0};
        std::int64_t _nextWriterId{0};

        // A map of topic readers indexed by the topic name.
        // Each key is a topic name, and the corresponding value is a vector of readers for that topic.
        std::map<std::string, std::vector<std::shared_ptr<TopicI>>> _readers;

        // A map of topic writers indexed by the topic name.
        // Each key is a topic name, and the corresponding value is a vector of writers for that topic.
        std::map<std::string, std::vector<std::shared_ptr<TopicI>>> _writers;
    };
}
#endif
