// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_TOPICI_H
#define DATASTORM_TOPICI_H

#include "DataElementI.h"
#include "DataStorm/InternalI.h"
#include "DataStorm/Types.h"
#include "Instance.h"

namespace DataStormI
{
    class SessionI;
    class TopicFactoryI;

    class TopicI : public virtual Topic, public std::enable_shared_from_this<TopicI>
    {
        struct Listener
        {
            Listener(DataStormContract::SessionPrx sessionPrx) : proxy(std::move(sessionPrx)) {}

            std::set<std::int64_t> topics;
            DataStormContract::SessionPrx proxy;
        };

    public:
        TopicI(
            std::shared_ptr<Instance>,
            const std::shared_ptr<TopicFactoryI>&,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>,
            std::string,
            std::int64_t);

        ~TopicI() override;

        [[nodiscard]] std::string getName() const override;
        void destroy() override;

        void shutdown();

        // const getter for _instance
        [[nodiscard]] const std::shared_ptr<Instance>& instance() const noexcept { return _instance; }

        [[nodiscard]] DataStormContract::TopicSpec getTopicSpec() const;
        [[nodiscard]] DataStormContract::ElementInfoSeq getTags() const;

        /// Compute the element specs for the local elements that match the given element infos.
        ///
        /// @param topicId The remote topic ID for the provided element infos.
        /// @param infos The element infos to match.
        /// @param session The session that requested the element specs.
        /// @return The element specs for the local elements that match the given element infos.
        [[nodiscard]] DataStormContract::ElementSpecSeq getElementSpecs(
            std::int64_t topicId,
            const DataStormContract::ElementInfoSeq& infos,
            const std::shared_ptr<SessionI>& session);

        /// Attach this topic with the remote topic with the given topic ID.
        ///
        /// @param topicId The remote topic ID to attach with.
        /// @param session The session servant in the current node.
        /// @param peerSession The peer session proxy.
        void attach(std::int64_t topicId, std::shared_ptr<SessionI> session, DataStormContract::SessionPrx peerSession);

        /// Detach this topic from the remote topic with the given topic ID.
        ///
        /// @param topicId The remote topic ID to detach from.
        /// @param session The session servant in the current node.
        void detach(std::int64_t topicId, const std::shared_ptr<SessionI>& session);

        [[nodiscard]] DataStormContract::ElementSpecAckSeq attachElements(
            std::int64_t,
            const DataStormContract::ElementSpecSeq&,
            const std::shared_ptr<SessionI>&,
            const DataStormContract::SessionPrx&,
            const std::chrono::time_point<std::chrono::system_clock>&);

        [[nodiscard]] DataStormContract::DataSamplesSeq attachElementsAck(
            std::int64_t,
            const DataStormContract::ElementSpecAckSeq&,
            const std::shared_ptr<SessionI>&,
            const DataStormContract::SessionPrx&,
            const std::chrono::time_point<std::chrono::system_clock>&,
            Ice::LongSeq&);

        void setUpdater(const std::shared_ptr<Tag>&, Updater) override;
        [[nodiscard]] const Updater& getUpdater(const std::shared_ptr<Tag>&) const;

        void setUpdaters(std::map<std::shared_ptr<Tag>, Updater>) override;
        [[nodiscard]] std::map<std::shared_ptr<Tag>, Updater> getUpdaters() const override;

        [[nodiscard]] bool isDestroyed() const { return _destroyed; }

        [[nodiscard]] std::int64_t getId() const { return _id; }

        [[nodiscard]] std::mutex& getMutex() { return _mutex; }

        [[nodiscard]] const std::shared_ptr<KeyFactory>& getKeyFactory() const { return _keyFactory; }

        [[nodiscard]] const std::shared_ptr<TagFactory>& getTagFactory() const { return _tagFactory; }

        [[nodiscard]] const std::shared_ptr<SampleFactory>& getSampleFactory() const { return _sampleFactory; }

        [[nodiscard]] const std::shared_ptr<FilterManager>& getSampleFilterFactories() const
        {
            return _sampleFilterFactories;
        }

        void incListenerCount(const std::shared_ptr<SessionI>&);
        void decListenerCount(const std::shared_ptr<SessionI>&);
        void decListenerCount(size_t);

        void removeFiltered(const std::shared_ptr<DataElementI>&, const std::shared_ptr<Filter>&);
        void remove(const std::shared_ptr<DataElementI>&, const std::vector<std::shared_ptr<Key>>&);

    protected:
        void waitForListeners(int count) const;
        [[nodiscard]] bool hasListeners() const;
        void notifyListenerWaiters(std::unique_lock<std::mutex>&) const;

        void disconnect();

        void forward(const Ice::ByteSeq&, const Ice::Current&) const;
        void forwarderException() const;

        void add(const std::shared_ptr<DataElementI>&, const std::vector<std::shared_ptr<Key>>&);
        void addFiltered(const std::shared_ptr<DataElementI>&, const std::shared_ptr<Filter>&);

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
        bool _destroyed{false};

        // A map containing the data readers or data writers for this topic.
        // The map's key is a pointer returned by the topic's key factory, and the value is a set of data elements
        // associated with that key.
        //
        // - When this class is an instance of the derived `TopicReaderI` class, the data elements are data readers.
        // - When this class is an instance of the derived `TopicWriterI` class, the data elements are data writers.
        std::map<std::shared_ptr<Key>, std::set<std::shared_ptr<DataElementI>>> _keyElements;

        // A map containing the filtered data readers for this topic.
        // The map's key is a pointer returned by the topic's filter factory, and the value is a set of data elements
        // representing the filtered data readers.
        std::map<std::shared_ptr<Filter>, std::set<std::shared_ptr<DataElementI>>> _filteredElements;

        // A map containing the per-session topic listeners.
        // The map's key is the session servant pointer, and the value is a listener object that contains:
        // - A set of remote topic IDs, and
        // - The peer session proxy.
        std::map<std::shared_ptr<SessionI>, Listener> _listeners;

        // A map containing the tag updaters for this topic. The tag updaters are used for partial updates.
        // The map's key is a pointer returned by the topic's tag factory, and the value is the updater function.
        std::map<std::shared_ptr<Tag>, Updater> _updaters;

        // The number of connected listeners.
        size_t _listenerCount{0};

        // The number of threads waiting for a listener notification. See waitForListeners().
        mutable size_t _waiters{0};
        mutable size_t _notified{0};
        std::int64_t _nextId{0};
        std::int64_t _nextFilteredId{0};
        std::int64_t _nextSampleId{0};
    };

    class TopicReaderI final : public TopicReader, public TopicI
    {
    public:
        TopicReaderI(
            std::shared_ptr<Instance>,
            const std::shared_ptr<TopicFactoryI>&,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>,
            std::string,
            std::int64_t);

        [[nodiscard]] std::shared_ptr<DataReader>
        createFiltered(const std::shared_ptr<Filter>&, std::string, DataStorm::ReaderConfig, std::string, Ice::ByteSeq)
            final;

        [[nodiscard]] std::shared_ptr<DataReader> create(
            const std::vector<std::shared_ptr<Key>>&,
            std::string,
            DataStorm::ReaderConfig,
            std::string,
            Ice::ByteSeq) final;

        void setDefaultConfig(DataStorm::ReaderConfig) final;
        void waitForWriters(int) const final;
        [[nodiscard]] bool hasWriters() const final;
        void destroy() final;

    private:
        [[nodiscard]] DataStorm::ReaderConfig parseConfig() const;
        [[nodiscard]] DataStorm::ReaderConfig mergeConfigs(DataStorm::ReaderConfig) const;

        DataStorm::ReaderConfig _defaultConfig;
    };

    class TopicWriterI final : public TopicWriter, public TopicI
    {
    public:
        TopicWriterI(
            std::shared_ptr<Instance>,
            const std::shared_ptr<TopicFactoryI>&,
            std::shared_ptr<KeyFactory>,
            std::shared_ptr<TagFactory>,
            std::shared_ptr<SampleFactory>,
            std::shared_ptr<FilterManager>,
            std::shared_ptr<FilterManager>,
            std::string,
            std::int64_t);

        [[nodiscard]] std::shared_ptr<DataWriter>
        create(const std::vector<std::shared_ptr<Key>>&, std::string, DataStorm::WriterConfig) final;

        void setDefaultConfig(DataStorm::WriterConfig) final;
        void waitForReaders(int) const final;
        [[nodiscard]] bool hasReaders() const final;
        void destroy() final;

    private:
        [[nodiscard]] DataStorm::WriterConfig parseConfig() const;
        [[nodiscard]] DataStorm::WriterConfig mergeConfigs(DataStorm::WriterConfig) const;

        DataStorm::WriterConfig _defaultConfig;
    };
}

#endif
