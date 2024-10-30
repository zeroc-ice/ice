//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    class DataElementI : virtual public DataElement, public std::enable_shared_from_this<DataElementI>
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
            Listener(DataStormContract::SessionPrx proxy, const std::string& facet)
                : proxy(facet.empty() ? proxy : proxy->ice_facet<DataStormContract::SessionPrx>(facet))
            {
            }

            bool matchOne(const std::shared_ptr<Sample>& sample, bool matchKey) const
            {
                for (const auto& s : subscribers)
                {
                    if ((!matchKey || s.second->keys.empty() ||
                         s.second->keys.find(sample->key) != s.second->keys.end()) &&
                        (!s.second->filter || s.second->filter->match(sample->key)) &&
                        (!s.second->sampleFilter || s.second->sampleFilter->match(sample)))
                    {
                        return true;
                    }
                }
                return false;
            }

            std::shared_ptr<Subscriber> addOrGet(
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

            std::shared_ptr<Subscriber> get(std::int64_t topicId, std::int64_t elementId)
            {
                return subscribers.find(std::make_pair(topicId, elementId))->second;
            }

            bool remove(std::int64_t topicId, std::int64_t elementId)
            {
                subscribers.erase(std::make_pair(topicId, elementId));
                return subscribers.empty();
            }

            DataStormContract::SessionPrx proxy;
            std::map<std::pair<std::int64_t, std::int64_t>, std::shared_ptr<Subscriber>> subscribers;
        };

    public:
        DataElementI(TopicI*, std::string, std::int64_t, const DataStorm::Config&);
        virtual ~DataElementI();

        virtual void destroy() override;

        void attach(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<Filter>&,
            const std::shared_ptr<SessionI>&,
            DataStormContract::SessionPrx,
            const DataStormContract::ElementData&,
            const std::chrono::time_point<std::chrono::system_clock>&,
            DataStormContract::ElementDataAckSeq&);

        std::function<void()> attach(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<Filter>&,
            const std::shared_ptr<SessionI>&,
            DataStormContract::SessionPrx,
            const DataStormContract::ElementDataAck&,
            const std::chrono::time_point<std::chrono::system_clock>&,
            DataStormContract::DataSamplesSeq&);

        bool attachKey(
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

        bool attachFilter(
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

        virtual std::vector<std::shared_ptr<Key>> getConnectedKeys() const override;
        virtual std::vector<std::string> getConnectedElements() const override;
        virtual void onConnectedKeys(
            std::function<void(std::vector<std::shared_ptr<Key>>)>,
            std::function<void(DataStorm::CallbackReason, std::shared_ptr<Key>)>) override;
        virtual void onConnectedElements(
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

        virtual std::string toString() const = 0;
        virtual Ice::CommunicatorPtr getCommunicator() const override;

        std::int64_t getId() const { return _id; }

        std::shared_ptr<DataStormContract::ElementConfig> getConfig() const;

        void waitForListeners(int count) const;
        bool hasListeners() const;

        TopicI* getTopic() const { return _parent.get(); }

    protected:
        virtual bool addConnectedKey(const std::shared_ptr<Key>&, const std::shared_ptr<Subscriber>&);
        virtual bool removeConnectedKey(const std::shared_ptr<Key>&, const std::shared_ptr<Subscriber>&);

        void notifyListenerWaiters(std::unique_lock<std::mutex>&) const;
        void disconnect();
        virtual void destroyImpl() = 0;

        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::string _name;
        const std::int64_t _id;
        const std::shared_ptr<DataStormContract::ElementConfig> _config;
        const std::shared_ptr<CallbackExecutor> _executor;

        size_t _listenerCount;
        mutable std::shared_ptr<Sample> _sample;
        DataStormContract::SessionPrx _forwarder;
        std::map<std::shared_ptr<Key>, std::vector<std::shared_ptr<Subscriber>>> _connectedKeys;
        std::map<ListenerKey, Listener> _listeners;

    private:
        virtual void forward(const Ice::ByteSeq&, const Ice::Current&) const;

        const std::shared_ptr<TopicI> _parent;
        mutable size_t _waiters;
        mutable size_t _notified;
        bool _destroyed;

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

        virtual int getInstanceCount() const override;

        virtual std::vector<std::shared_ptr<Sample>> getAllUnread() override;
        virtual void waitForUnread(unsigned int) const override;
        virtual bool hasUnread() const override;
        virtual std::shared_ptr<Sample> getNextUnread() override;

        virtual void initSamples(
            const std::vector<std::shared_ptr<Sample>>&,
            std::int64_t,
            std::int64_t,
            int,
            const std::chrono::time_point<std::chrono::system_clock>&,
            bool) override;
        virtual void queue(
            const std::shared_ptr<Sample>&,
            int,
            const std::shared_ptr<SessionI>&,
            const std::string&,
            const std::chrono::time_point<std::chrono::system_clock>&,
            bool) override;

        virtual void onSamples(
            std::function<void(const std::vector<std::shared_ptr<Sample>>&)>,
            std::function<void(const std::shared_ptr<Sample>&)>) override;

    protected:
        virtual bool matchKey(const std::shared_ptr<Key>&) const = 0;
        virtual bool addConnectedKey(const std::shared_ptr<Key>&, const std::shared_ptr<Subscriber>&) override;

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

        virtual void publish(const std::shared_ptr<Key>&, const std::shared_ptr<Sample>&) override;

    protected:
        virtual void send(const std::shared_ptr<Key>&, const std::shared_ptr<Sample>&) const = 0;

        TopicWriterI* _parent;
        DataStormContract::SubscriberSessionPrx _subscribers;
        std::deque<std::shared_ptr<Sample>> _samples;
        std::shared_ptr<Sample> _last;
    };

    class KeyDataReaderI : public DataReaderI
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

        virtual void destroyImpl() override;

        virtual void waitForWriters(int) override;
        virtual bool hasWriters() override;

        virtual std::string toString() const override;

    private:
        virtual bool matchKey(const std::shared_ptr<Key>&) const override;

        const std::vector<std::shared_ptr<Key>> _keys;
    };

    class KeyDataWriterI : public DataWriterI
    {
    public:
        KeyDataWriterI(
            TopicWriterI*,
            std::string,
            std::int64_t,
            const std::vector<std::shared_ptr<Key>>&,
            const DataStorm::WriterConfig&);

        virtual void destroyImpl() override;

        virtual void waitForReaders(int) const override;
        virtual bool hasReaders() const override;

        virtual std::shared_ptr<Sample> getLast() const override;
        virtual std::vector<std::shared_ptr<Sample>> getAll() const override;

        virtual std::string toString() const override;
        virtual DataStormContract::DataSamples getSamples(
            const std::shared_ptr<Key>&,
            const std::shared_ptr<Filter>&,
            const std::shared_ptr<DataStormContract::ElementConfig>&,
            std::int64_t,
            const std::chrono::time_point<std::chrono::system_clock>&) override;

    private:
        virtual void send(const std::shared_ptr<Key>&, const std::shared_ptr<Sample>&) const override;
        virtual void forward(const Ice::ByteSeq&, const Ice::Current&) const override;

        const std::vector<std::shared_ptr<Key>> _keys;
    };

    class FilteredDataReaderI : public DataReaderI
    {
    public:
        FilteredDataReaderI(
            TopicReaderI*,
            std::string,
            std::int64_t,
            const std::shared_ptr<Filter>&,
            std::string,
            Ice::ByteSeq,
            const DataStorm::ReaderConfig&);

        virtual void destroyImpl() override;

        virtual void waitForWriters(int) override;
        virtual bool hasWriters() override;

        virtual std::string toString() const override;

    private:
        virtual bool matchKey(const std::shared_ptr<Key>&) const override;

        const std::shared_ptr<Filter> _filter;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
