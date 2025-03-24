// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_DATA_ELEMENTI_H
#define DATASTORM_DATA_ELEMENTI_H

#include "DataStorm/Contract.h"
#include "DataStorm/InternalI.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace DataStormI
{
    class SessionI;
    class TopicI;
    class TopicReaderI;
    class TopicWriterI;
    class CallbackExecutor;
    class TraceLevels;

    // Base class for DataReaderI and DataWriterI.
    class DataElementI : public virtual DataElement, public std::enable_shared_from_this<DataElementI>
    {
    protected:
        struct Subscriber
        {
            Subscriber(
                std::int64_t id,
                const std::shared_ptr<Filter>& filter,
                const std::shared_ptr<Filter>& sampleFilter,
                std::string name,
                int priority)
                : id(id),
                  filter(filter),
                  sampleFilter(sampleFilter),
                  name(std::move(name)),
                  priority(priority)
            {
            }

            std::int64_t topicId;
            std::int64_t elementId;
            std::int64_t id;
            std::set<std::shared_ptr<Key>> keys;
            std::shared_ptr<Filter> filter;
            std::shared_ptr<Filter> sampleFilter;
            std::string name;
            int priority;
        };

    private:
        struct ListenerKey
        {
            std::shared_ptr<SessionI> session;
            std::string facet;

            bool operator<(const ListenerKey& other) const
            {
                if (session < other.session)
                {
                    return true;
                }
                else if (other.session < session)
                {
                    return false;
                }
                return facet < other.facet;
            }
        };

        struct Listener
        {
            Listener(DataStormContract::SessionPrx proxy, std::string facet)
                : proxy(
                      facet.empty() ? std::move(proxy)
                                    : proxy->ice_facet<DataStormContract::SessionPrx>(std::move(facet)))
            {
            }

            /// Determines if any subscriber matches the given sample.
            ///
            /// @param sample The sample to evaluate against the subscribers.
            /// @param matchKey If true, the sample's key is matched against subscriber keys.
            ///                 If false, the key match is skipped.
            /// @return `true` if at least one subscriber matches the sample, otherwise false.
            [[nodiscard]] bool matchOne(const std::shared_ptr<Sample>& sample, bool matchKey) const
            {
                for (const auto& [_, subscriber] : subscribers)
                {
                    if ((!matchKey || subscriber->keys.empty() ||
                         subscriber->keys.find(sample->key) != subscriber->keys.end()) &&
                        (!subscriber->filter || subscriber->filter->match(sample->key)) &&
                        (!subscriber->sampleFilter || subscriber->sampleFilter->match(sample)))
                    {
                        return true;
                    }
                }
                return false;
            }

            [[nodiscard]] std::shared_ptr<Subscriber> addOrGet(
                std::int64_t topicId,
                std::int64_t elementId,
                std::int64_t id,
                const std::shared_ptr<Filter>& filter,
                const std::shared_ptr<Filter>& sampleFilter,
                const std::string& name,
                int priority,
                bool& added)
            {
                auto k = std::make_pair(topicId, elementId);
                auto p = subscribers.find(k);
                if (p == subscribers.end())
                {
                    added = true;
                    p = subscribers.emplace(k, std::make_shared<Subscriber>(id, filter, sampleFilter, name, priority))
                            .first;
                }
                return p->second;
            }

            [[nodiscard]] std::shared_ptr<Subscriber> get(std::int64_t topicId, std::int64_t elementId)
            {
                return subscribers.find(std::make_pair(topicId, elementId))->second;
            }

            [[nodiscard]] bool remove(std::int64_t topicId, std::int64_t elementId)
            {
                subscribers.erase(std::make_pair(topicId, elementId));
                return subscribers.empty();
            }

            // The proxy to the peer session.
            DataStormContract::SessionPrx proxy;
            // A map containing the data element subscribers, indexed by the topic ID and the element ID.
            std::map<std::pair<std::int64_t, std::int64_t>, std::shared_ptr<Subscriber>> subscribers;
        };

    public:
        DataElementI(TopicI*, std::string, std::int64_t, const DataStorm::Config&);
        ~DataElementI() override;

        void destroy() override;

        /// Attaches a local data element to a remote data element identified by the provided ID.
        ///
        /// @param topicId The unique identifier for the topic to which the remote data element belongs.
        /// @param id The unique identifier of the remote data element.
        /// @param key The key associated with the local element. Set to `nullptr` if the local element represents a
        /// filter.
        /// @param filter The filter associated with this element:
        /// - If the remote element represents a filter, this is the filter decoded from the peer.
        /// - If the local element represents a filter, this is the local filter.
        /// @param session The session that initiated the attachment request.
        /// @param prx The proxy to the peer session.
        /// @param data The data associated with the remote element.
        /// @param now The timestamp indicating when the attachment was requested.
        /// @param acks Output parameter filled with acknowledgment data for the matching elements.
        void attach(
            std::int64_t topicId,
            std::int64_t id,
            const std::shared_ptr<Key>& key,
            const std::shared_ptr<Filter>& filter,
            const std::shared_ptr<SessionI>& session,
            DataStormContract::SessionPrx prx,
            const DataStormContract::ElementData& data,
            const std::chrono::time_point<std::chrono::system_clock>& now,
            DataStormContract::ElementDataAckSeq& acks);

        /// Attaches a local data element to a remote data element identified by the provided ID.
        ///
        /// This method establishes a connection between a local and remote data element. It prepares the necessary
        /// data for synchronization, with behavior varying between publisher and subscriber sides.
        ///
        /// @param topicId The unique identifier for the topic to which the remote data element belongs.
        /// @param id The unique identifier of the remote data element.
        /// @param key The key associated with the local element. Set to `nullptr` if the local element represents a
        /// filter.
        /// @param filter The filter associated with this element:
        /// - If the remote element represents a filter, this is the filter key provided by the peer.
        /// - If the local element represents a filter, this is the local filter.
        /// @param session The session that initiated the attachment request.
        /// @param prx The proxy to the peer session.
        /// @param data The acknowledgment data associated with the remote element, describing its configuration or
        /// state.
        /// @param now The timestamp indicating when the attachment was requested.
        /// @param samples Output parameter filled with the data samples in the publisher's queue. This parameter is
        /// always empty when the method is called on the subscriber side.
        /// @return A function that initializes the reader with the prepared samples:
        /// - For a publisher, this method always returns a `nullptr` function.
        /// - For a subscriber, this method returns a function that initializes the reader with samples provided by the
        /// peer.
        [[nodiscard]] std::function<void()> attach(
            std::int64_t topicId,
            std::int64_t id,
            const std::shared_ptr<Key>& key,
            const std::shared_ptr<Filter>& filter,
            const std::shared_ptr<SessionI>& session,
            DataStormContract::SessionPrx prx,
            const DataStormContract::ElementDataAck& data,
            const std::chrono::time_point<std::chrono::system_clock>& now,
            DataStormContract::DataSamplesSeq& samples);

        [[nodiscard]] bool attachKey(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<Filter>&,
            const std::shared_ptr<SessionI>&,
            DataStormContract::SessionPrx,
            const std::string&,
            std::int64_t,
            const std::string&,
            int);

        void detachKey(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<SessionI>&,
            const std::string&,
            bool);

        [[nodiscard]] bool attachFilter(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<Filter>&,
            const std::shared_ptr<SessionI>&,
            DataStormContract::SessionPrx,
            const std::string&,
            std::int64_t,
            const std::shared_ptr<Filter>&,
            const std::string&,
            int);

        void detachFilter(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<SessionI>&,
            const std::string&,
            bool);

        [[nodiscard]] std::vector<std::shared_ptr<Key>> getConnectedKeys() const override;
        [[nodiscard]] std::vector<std::string> getConnectedElements() const override;

        void onConnectedKeys(
            std::function<void(std::vector<std::shared_ptr<Key>>)>,
            std::function<void(DataStorm::CallbackReason, std::shared_ptr<Key>)>) override;

        void onConnectedElements(
            std::function<void(std::vector<std::string>)>,
            std::function<void(DataStorm::CallbackReason, std::string)>) override;

        virtual void initSamples(
            const std::vector<std::shared_ptr<Sample>>&,
            std::int64_t,
            std::int64_t,
            int,
            const std::chrono::time_point<std::chrono::system_clock>&,
            bool);

        virtual DataStormContract::DataSamples getSamples(
            const std::shared_ptr<Key>&,
            const std::shared_ptr<Filter>&,
            const std::shared_ptr<DataStormContract::ElementConfig>&,
            std::int64_t,
            const std::chrono::time_point<std::chrono::system_clock>&);

        virtual void queue(
            const std::shared_ptr<Sample>&,
            int,
            const std::shared_ptr<SessionI>&,
            const std::string&,
            const std::chrono::time_point<std::chrono::system_clock>&,
            bool);

        [[nodiscard]] virtual std::string toString() const = 0;

        [[nodiscard]] Ice::CommunicatorPtr getCommunicator() const override;

        [[nodiscard]] std::int64_t getId() const { return _id; }

        [[nodiscard]] std::shared_ptr<DataStormContract::ElementConfig> getConfig() const;

        void waitForListeners(int count) const;
        [[nodiscard]] bool hasListeners() const;

        [[nodiscard]] TopicI* getTopic() const { return _parent.get(); }

    protected:
        virtual bool addConnectedKey(const std::shared_ptr<Key>& key, const std::shared_ptr<Subscriber>& subscriber);
        virtual bool removeConnectedKey(const std::shared_ptr<Key>&, const std::shared_ptr<Subscriber>&);

        void notifyListenerWaiters(std::unique_lock<std::mutex>&) const;
        void disconnect();
        virtual void destroyImpl() = 0;

        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::string _name;
        const std::int64_t _id;
        const std::shared_ptr<DataStormContract::ElementConfig> _config;
        const std::shared_ptr<CallbackExecutor> _executor;

        size_t _listenerCount{0};
        mutable std::shared_ptr<Sample> _sample;
        DataStormContract::SessionPrx _forwarder;
        // A map containing the connected keys, these are keys attached to a peer key.
        // The map is indexed by the key, and the value is a vector of subscribers for the given key.
        std::map<std::shared_ptr<Key>, std::vector<std::shared_ptr<Subscriber>>> _connectedKeys;

        // A map containing the element listeners, indexed by the session servant and the target facet. The
        // implementation of forward utilizes the listener map to forward calls to the peer sessions.
        std::map<ListenerKey, Listener> _listeners;

    private:
        virtual void forward(const Ice::ByteSeq&, const Ice::Current&) const;

        const std::shared_ptr<TopicI> _parent;
        mutable size_t _waiters{0};
        mutable size_t _notified{0};
        bool _destroyed{false};

        std::function<void(DataStorm::CallbackReason, std::shared_ptr<Key>)> _onConnectedKeys;
        std::function<void(DataStorm::CallbackReason, std::string)> _onConnectedElements;
    };

    class DataReaderI : public DataElementI, public DataReader
    {
    public:
        DataReaderI(
            TopicReaderI*,
            std::string,
            std::int64_t,
            std::string,
            Ice::ByteSeq,
            const DataStorm::ReaderConfig&);

        [[nodiscard]] int getInstanceCount() const override;

        [[nodiscard]] std::vector<std::shared_ptr<Sample>> getAllUnread() override;
        void waitForUnread(unsigned int) const override;
        [[nodiscard]] bool hasUnread() const override;
        [[nodiscard]] std::shared_ptr<Sample> getNextUnread() override;

        void initSamples(
            const std::vector<std::shared_ptr<Sample>>&,
            std::int64_t,
            std::int64_t,
            int,
            const std::chrono::time_point<std::chrono::system_clock>&,
            bool) override;

        void queue(
            const std::shared_ptr<Sample>&,
            int,
            const std::shared_ptr<SessionI>&,
            const std::string&,
            const std::chrono::time_point<std::chrono::system_clock>&,
            bool) override;

        void onSamples(
            std::function<void(const std::vector<std::shared_ptr<Sample>>&)>,
            std::function<void(const std::shared_ptr<Sample>&)>) override;

    protected:
        [[nodiscard]] virtual bool matchKey(const std::shared_ptr<Key>&) const = 0;
        [[nodiscard]] bool addConnectedKey(const std::shared_ptr<Key>&, const std::shared_ptr<Subscriber>&) override;

        TopicReaderI* _parent;

        std::deque<std::shared_ptr<Sample>> _samples;
        std::shared_ptr<Sample> _last;
        int _instanceCount;
        DataStorm::DiscardPolicy _discardPolicy;
        std::chrono::time_point<std::chrono::system_clock> _lastSendTime;
        std::function<void(const std::shared_ptr<Sample>&)> _onSamples;
    };

    class DataWriterI : public DataElementI, public DataWriter
    {
    public:
        DataWriterI(TopicWriterI*, std::string, std::int64_t, const DataStorm::WriterConfig&);

        void publish(const std::shared_ptr<Key>&, const std::shared_ptr<Sample>&) override;

    protected:
        virtual void send(const std::shared_ptr<Key>&, const std::shared_ptr<Sample>&) const = 0;

        TopicWriterI* _parent;
        DataStormContract::SubscriberSessionPrx _subscribers;
        std::deque<std::shared_ptr<Sample>> _samples;
        std::shared_ptr<Sample> _last;
    };

    class KeyDataReaderI final : public DataReaderI
    {
    public:
        KeyDataReaderI(
            TopicReaderI*,
            std::string,
            std::int64_t,
            const std::vector<std::shared_ptr<Key>>&,
            std::string,
            Ice::ByteSeq,
            const DataStorm::ReaderConfig&);

        void destroyImpl() final;

        void waitForWriters(int) final;
        [[nodiscard]] bool hasWriters() final;

        [[nodiscard]] std::string toString() const final;

    private:
        [[nodiscard]] bool matchKey(const std::shared_ptr<Key>&) const final;

        const std::vector<std::shared_ptr<Key>> _keys;
    };

    class KeyDataWriterI final : public DataWriterI
    {
    public:
        KeyDataWriterI(
            TopicWriterI*,
            std::string,
            std::int64_t,
            const std::vector<std::shared_ptr<Key>>&,
            const DataStorm::WriterConfig&);

        void destroyImpl() final;

        void waitForReaders(int) const final;
        [[nodiscard]] bool hasReaders() const final;

        [[nodiscard]] std::shared_ptr<Sample> getLast() const final;
        [[nodiscard]] std::vector<std::shared_ptr<Sample>> getAll() const final;

        [[nodiscard]] std::string toString() const final;

        [[nodiscard]] DataStormContract::DataSamples getSamples(
            const std::shared_ptr<Key>&,
            const std::shared_ptr<Filter>&,
            const std::shared_ptr<DataStormContract::ElementConfig>&,
            std::int64_t,
            const std::chrono::time_point<std::chrono::system_clock>&) final;

    private:
        void send(const std::shared_ptr<Key>&, const std::shared_ptr<Sample>&) const final;
        void forward(const Ice::ByteSeq&, const Ice::Current&) const final;

        const std::vector<std::shared_ptr<Key>> _keys;
    };

    class FilteredDataReaderI final : public DataReaderI
    {
    public:
        FilteredDataReaderI(
            TopicReaderI*,
            std::string,
            std::int64_t,
            std::shared_ptr<Filter>,
            std::string,
            Ice::ByteSeq,
            const DataStorm::ReaderConfig&);

        void destroyImpl() final;

        void waitForWriters(int) final;
        [[nodiscard]] bool hasWriters() final;

        [[nodiscard]] std::string toString() const final;

    private:
        [[nodiscard]] bool matchKey(const std::shared_ptr<Key>&) const final;

        const std::shared_ptr<Filter> _filter;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
