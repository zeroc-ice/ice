//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_TOPICI_H
#define DATASTORM_TOPICI_H

#include "DataElementI.h"
#include "DataStorm/InternalI.h"
#include "DataStorm/Types.h"
#include "ForwarderManager.h"
#include "Instance.h"

namespace DataStormI
{
    class SessionI;
    class TopicFactoryI;

    class TopicI : public virtual Topic, public std::enable_shared_from_this<TopicI>
    {
        struct ListenerKey
        {
            std::shared_ptr<SessionI> session;

            bool operator<(const ListenerKey& other) const { return session < other.session; }
        };

        struct Listener
        {
            Listener(DataStormContract::SessionPrx sessionPrx) : proxy(std::move(sessionPrx)) {}

            std::set<std::int64_t> topics;
            DataStormContract::SessionPrx proxy;
        };

    public:
        TopicI(
            std::weak_ptr<TopicFactoryI>,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>,
            std::string,
            std::int64_t);

        ~TopicI() override;

        std::string getName() const override;
        void destroy() override;

        void shutdown();

        const std::shared_ptr<Instance>& getInstance() const { return _instance; }

        DataStormContract::TopicSpec getTopicSpec() const;
        DataStormContract::ElementInfoSeq getTags() const;
        DataStormContract::ElementSpecSeq
        getElementSpecs(std::int64_t, const DataStormContract::ElementInfoSeq&, const std::shared_ptr<SessionI>&);

        void attach(std::int64_t, const std::shared_ptr<SessionI>&, DataStormContract::SessionPrx);
        void detach(std::int64_t, const std::shared_ptr<SessionI>&);

        DataStormContract::ElementSpecAckSeq attachElements(
            std::int64_t,
            const DataStormContract::ElementSpecSeq&,
            const std::shared_ptr<SessionI>&,
            DataStormContract::SessionPrx,
            const std::chrono::time_point<std::chrono::system_clock>&);

        DataStormContract::DataSamplesSeq attachElementsAck(
            std::int64_t,
            const DataStormContract::ElementSpecAckSeq&,
            const std::shared_ptr<SessionI>&,
            DataStormContract::SessionPrx,
            const std::chrono::time_point<std::chrono::system_clock>&,
            Ice::LongSeq&);

        void setUpdater(const std::shared_ptr<Tag>&, Updater) override;
        const Updater& getUpdater(const std::shared_ptr<Tag>&) const;

        void setUpdaters(std::map<std::shared_ptr<Tag>, Updater>) override;
        std::map<std::shared_ptr<Tag>, Updater> getUpdaters() const override;

        bool isDestroyed() const { return _destroyed; }

        std::int64_t getId() const { return _id; }

        std::mutex& getMutex() { return _mutex; }

        const std::shared_ptr<KeyFactory>& getKeyFactory() const { return _keyFactory; }

        const std::shared_ptr<TagFactory>& getTagFactory() const { return _tagFactory; }

        const std::shared_ptr<SampleFactory>& getSampleFactory() const { return _sampleFactory; }

        const std::shared_ptr<FilterManager>& getSampleFilterFactories() const { return _sampleFilterFactories; }

        void incListenerCount(const std::shared_ptr<SessionI>&);
        void decListenerCount(const std::shared_ptr<SessionI>&);
        void decListenerCount(size_t);

        void removeFiltered(const std::shared_ptr<DataElementI>&, const std::shared_ptr<Filter>&);
        void remove(const std::shared_ptr<DataElementI>&, const std::vector<std::shared_ptr<Key>>&);

    protected:
        void waitForListeners(int count) const;
        bool hasListeners() const;
        void notifyListenerWaiters(std::unique_lock<std::mutex>&) const;

        void disconnect();

        void forward(const Ice::ByteSeq&, const Ice::Current&) const;
        void forwarderException() const;

        void add(const std::shared_ptr<DataElementI>&, const std::vector<std::shared_ptr<Key>>&);
        void addFiltered(const std::shared_ptr<DataElementI>&, const std::shared_ptr<Filter>&);

        void parseConfigImpl(const Ice::PropertyDict&, const std::string&, DataStorm::Config&) const;

        friend class DataElementI;
        friend class DataReaderI;
        friend class FilteredDataReaderI;
        friend class DataWriterI;
        friend class KeyDataWriterI;
        friend class KeyDataReaderI;

        const std::weak_ptr<TopicFactoryI> _factory;
        const std::shared_ptr<KeyFactory> _keyFactory;
        const std::shared_ptr<TagFactory> _tagFactory;
        const std::shared_ptr<SampleFactory> _sampleFactory;
        const std::shared_ptr<FilterManager> _keyFilterFactories;
        const std::shared_ptr<FilterManager> _sampleFilterFactories;
        const std::string _name;
        const std::shared_ptr<Instance> _instance;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::int64_t _id;
        DataStormContract::SessionPrx _forwarder;

        mutable std::mutex _mutex;
        mutable std::condition_variable _cond;
        bool _destroyed;
        std::map<std::shared_ptr<Key>, std::set<std::shared_ptr<DataElementI>>> _keyElements;
        std::map<std::shared_ptr<Filter>, std::set<std::shared_ptr<DataElementI>>> _filteredElements;
        std::map<ListenerKey, Listener> _listeners;
        std::map<std::shared_ptr<Tag>, Updater> _updaters;
        size_t _listenerCount;
        mutable size_t _waiters;
        mutable size_t _notified;
        std::int64_t _nextId;
        std::int64_t _nextFilteredId;
        std::int64_t _nextSampleId;
    };

    class TopicReaderI final : public TopicReader, public TopicI
    {
    public:
        TopicReaderI(
            std::shared_ptr<TopicFactoryI>,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>,
            std::string,
            std::int64_t);

        std::shared_ptr<DataReader>
        createFiltered(const std::shared_ptr<Filter>&, std::string, DataStorm::ReaderConfig, std::string, Ice::ByteSeq)
            final;

        std::shared_ptr<DataReader> create(
            const std::vector<std::shared_ptr<Key>>&,
            std::string,
            DataStorm::ReaderConfig,
            std::string,
            Ice::ByteSeq) final;

        void setDefaultConfig(DataStorm::ReaderConfig) final;
        void waitForWriters(int) const final;
        bool hasWriters() const final;
        void destroy() final;

    private:
        DataStorm::ReaderConfig parseConfig(const std::string&) const;
        DataStorm::ReaderConfig mergeConfigs(DataStorm::ReaderConfig) const;

        DataStorm::ReaderConfig _defaultConfig;
    };

    class TopicWriterI final : public TopicWriter, public TopicI
    {
    public:
        TopicWriterI(
            std::shared_ptr<TopicFactoryI>,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>,
            std::string,
            std::int64_t);

        std::shared_ptr<DataWriter>
        create(const std::vector<std::shared_ptr<Key>>&, std::string, DataStorm::WriterConfig) final;

        void setDefaultConfig(DataStorm::WriterConfig) final;
        void waitForReaders(int) const final;
        bool hasReaders() const final;
        void destroy() final;

    private:
        DataStorm::WriterConfig parseConfig(const std::string&) const;
        DataStorm::WriterConfig mergeConfigs(DataStorm::WriterConfig) const;

        DataStorm::WriterConfig _defaultConfig;
    };
}

#endif
