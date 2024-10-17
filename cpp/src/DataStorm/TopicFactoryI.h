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
    class TopicReaderI;
    class TopicWriterI;

    class TopicFactoryI : public TopicFactory, public std::enable_shared_from_this<TopicFactoryI>
    {
    public:
        TopicFactoryI(const std::shared_ptr<Instance>&);

        virtual std::shared_ptr<TopicReader> createTopicReader(
            const std::string&,
            const std::shared_ptr<KeyFactory>&,
            const std::shared_ptr<TagFactory>&,
            const std::shared_ptr<SampleFactory>&,
            const std::shared_ptr<FilterManager>&,
            const std::shared_ptr<FilterManager>&) override;

        virtual std::shared_ptr<TopicWriter> createTopicWriter(
            const std::string&,
            const std::shared_ptr<KeyFactory>&,
            const std::shared_ptr<TagFactory>&,
            const std::shared_ptr<SampleFactory>&,
            const std::shared_ptr<FilterManager>&,
            const std::shared_ptr<FilterManager>&) override;

        virtual Ice::CommunicatorPtr getCommunicator() const override;

        void removeTopicReader(const std::string&, const std::shared_ptr<TopicI>&);
        void removeTopicWriter(const std::string&, const std::shared_ptr<TopicI>&);

        std::vector<std::shared_ptr<TopicI>> getTopicReaders(const std::string&) const;
        std::vector<std::shared_ptr<TopicI>> getTopicWriters(const std::string&) const;

        void createSubscriberSession(const std::string&, DataStormContract::NodePrx, const Ice::ConnectionPtr&);
        void createPublisherSession(const std::string&, DataStormContract::NodePrx, const Ice::ConnectionPtr&);

        std::shared_ptr<Instance> getInstance() const
        {
            auto instance = _instance.lock();
            assert(instance);
            return instance;
        }

        DataStormContract::TopicInfoSeq getTopicReaders() const;
        DataStormContract::TopicInfoSeq getTopicWriters() const;

        DataStormContract::StringSeq getTopicReaderNames() const;
        DataStormContract::StringSeq getTopicWriterNames() const;

        void shutdown() const;

    private:
        mutable std::mutex _mutex;
        std::weak_ptr<Instance> _instance;
        std::shared_ptr<TraceLevels> _traceLevels;
        std::map<std::string, std::vector<std::shared_ptr<TopicI>>> _readers;
        std::map<std::string, std::vector<std::shared_ptr<TopicI>>> _writers;
        std::int64_t _nextReaderId;
        std::int64_t _nextWriterId;
    };

}
#endif
