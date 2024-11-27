//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_SESSIONI_H
#define DATASTORM_SESSIONI_H

#include "DataStorm/Contract.h"
#include "Ice/Ice.h"
#include "NodeI.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace DataStormI
{
    class TopicI;
    class DataElementI;
    class Instance;
    class TraceLevels;

    class SessionI : public virtual DataStormContract::Session, public std::enable_shared_from_this<SessionI>
    {
    protected:
        struct ElementSubscriber
        {
            ElementSubscriber(const std::string& facet, const std::shared_ptr<Key>& key, int sessionInstanceId)
                : facet(facet),
                  initialized(false),
                  lastId(0),
                  sessionInstanceId(sessionInstanceId)
            {
                keys.insert(key);
            }

            const std::string facet;
            bool initialized;
            std::int64_t lastId;
            std::set<std::shared_ptr<Key>> keys;
            int sessionInstanceId;
        };

        class ElementSubscribers
        {
        public:
            ElementSubscribers(std::string name, int priority)
                : name(std::move(name)),
                  priority(priority),
                  _sessionInstanceId(0)
            {
            }

            void addSubscriber(
                const std::shared_ptr<DataElementI>& element,
                const std::shared_ptr<Key>& key,
                const std::string& facet,
                int sessionInstanceId)
            {
                _sessionInstanceId = sessionInstanceId;
                auto p = _subscribers.find(element);
                if (p != _subscribers.end())
                {
                    p->second.keys.insert(key);
                    p->second.sessionInstanceId = sessionInstanceId;
                    p->second.initialized = false;
                }
                else
                {
                    _subscribers.emplace(element, ElementSubscriber(facet, key, sessionInstanceId));
                }
            }

            void removeSubscriber(const std::shared_ptr<DataElementI>& element) { _subscribers.erase(element); }

            std::map<std::shared_ptr<DataElementI>, ElementSubscriber>& getSubscribers() { return _subscribers; }

            ElementSubscriber* getSubscriber(const std::shared_ptr<DataElementI>& element)
            {
                auto p = _subscribers.find(element);
                if (p != _subscribers.end())
                {
                    return &p->second;
                }
                return nullptr;
            }

            bool reap(int sessionInstanceId)
            {
                if (_sessionInstanceId != sessionInstanceId)
                {
                    return true;
                }

                auto p = _subscribers.begin();
                while (p != _subscribers.end())
                {
                    if (p->second.sessionInstanceId != sessionInstanceId)
                    {
                        _subscribers.erase(p++);
                    }
                    else
                    {
                        ++p;
                    }
                }
                return _subscribers.empty();
            }

            std::string name;
            int priority;

        private:
            std::map<std::shared_ptr<DataElementI>, ElementSubscriber> _subscribers;
            int _sessionInstanceId;
        };

        /// Represents the subscription from a local topic object to a remote topic.
        class TopicSubscriber
        {
        public:
            TopicSubscriber(int sessionInstanceId) : sessionInstanceId(sessionInstanceId) {}

            ElementSubscribers* add(std::int64_t id, std::string name, int priority)
            {
                auto p = _elements.find(id);
                if (p == _elements.end())
                {
                    p = _elements.emplace(id, ElementSubscribers(std::move(name), priority)).first;
                }
                return &p->second;
            }

            ElementSubscribers* get(std::int64_t id)
            {
                auto p = _elements.find(id);
                if (p == _elements.end())
                {
                    return nullptr;
                }
                return &p->second;
            }

            ElementSubscribers remove(std::int64_t id)
            {
                auto p = _elements.find(id);
                if (p != _elements.end())
                {
                    ElementSubscribers tmp(std::move(p->second));
                    _elements.erase(p);
                    return tmp;
                }
                return ElementSubscribers("", 0);
            }

            std::map<std::int64_t, ElementSubscribers>& getAll() { return _elements; }

            void reap(int id)
            {
                auto p = _elements.begin();
                while (p != _elements.end())
                {
                    if (p->second.reap(id))
                    {
                        _elements.erase(p++);
                    }
                    else
                    {
                        ++p;
                    }
                }
            }

            std::map<std::int64_t, std::pair<std::shared_ptr<Key>, std::map<std::int64_t, int>>> keys;
            std::map<std::int64_t, std::shared_ptr<Tag>> tags;
            int sessionInstanceId;

        private:
            std::map<std::int64_t, ElementSubscribers> _elements;
        };

        // Tracks the subscribers of a remote topic using a given session instance.
        class TopicSubscribers
        {
        public:
            // Add the give topic as a subscriber, if a subscription already exists, update the session instance id.
            void addSubscriber(TopicI* topic, int sessionInstanceId)
            {
                _sessionInstanceId = sessionInstanceId;
                auto p = _subscribers.find(topic);
                if (p != _subscribers.end())
                {
                    p->second.sessionInstanceId = sessionInstanceId;
                }
                else
                {
                    _subscribers.emplace(topic, TopicSubscriber(sessionInstanceId));
                }
            }

            TopicSubscriber& getSubscriber(TopicI* topic)
            {
                assert(_subscribers.find(topic) != _subscribers.end());
                return _subscribers.at(topic);
            }

            void removeSubscriber(TopicI* topic) { _subscribers.erase(topic); }

            std::map<TopicI*, TopicSubscriber>& getSubscribers() { return _subscribers; }

            // Determine if the subscriber should be reaped.
            bool reap(int sessionInstanceId)
            {
                if (sessionInstanceId != _sessionInstanceId)
                {
                    // If using a prior session instance id, we can remove all subscribers.
                    return true;
                }

                auto p = _subscribers.begin();
                while (p != _subscribers.end())
                {
                    if (p->second.sessionInstanceId != sessionInstanceId)
                    {
                        // Remove the subscriber if it is using a prior session instance id.
                        _subscribers.erase(p++);
                    }
                    else
                    {
                        ++p;
                    }
                }

                // If there are no subscribers left, we can reap this object.
                return _subscribers.empty();
            }

        private:
            // Each entry in the map represents a subscriber to the same remote topic.
            // The key is a pointer to the local topic object subscribing to the remote topic, and the TopicSubscriber
            // object contains the subscription details.
            std::map<TopicI*, TopicSubscriber> _subscribers;

            // The session instance id for the last subscription.
            int _sessionInstanceId;
        };

    public:
        SessionI(const std::shared_ptr<NodeI>&, DataStormContract::NodePrx, DataStormContract::SessionPrx);
        void init();

        void announceTopics(DataStormContract::TopicInfoSeq, bool, const Ice::Current&) final;
        void attachTopic(DataStormContract::TopicSpec, const Ice::Current&) final;
        void detachTopic(std::int64_t, const Ice::Current&) final;

        void attachTags(std::int64_t, DataStormContract::ElementInfoSeq, bool, const Ice::Current&) final;
        void detachTags(std::int64_t, Ice::LongSeq, const Ice::Current&) final;

        void announceElements(std::int64_t, DataStormContract::ElementInfoSeq, const Ice::Current&) final;
        void attachElements(std::int64_t, DataStormContract::ElementSpecSeq, bool, const Ice::Current&) final;
        void attachElementsAck(std::int64_t, DataStormContract::ElementSpecAckSeq, const Ice::Current&) final;
        void detachElements(std::int64_t, Ice::LongSeq, const Ice::Current&) final;

        void initSamples(std::int64_t, DataStormContract::DataSamplesSeq, const Ice::Current&) final;

        void disconnected(const Ice::Current&) final;

        void
        connected(DataStormContract::SessionPrx, const Ice::ConnectionPtr&, const DataStormContract::TopicInfoSeq&);
        bool disconnected(const Ice::ConnectionPtr&, std::exception_ptr);
        bool retry(DataStormContract::NodePrx, std::exception_ptr);
        void destroyImpl(const std::exception_ptr&);

        const std::string& getId() const { return _id; }

        Ice::ConnectionPtr getConnection() const;
        std::optional<DataStormContract::SessionPrx> getSession() const;
        bool checkSession();

        template<typename T = DataStormContract::SessionPrx> std::optional<T> getProxy() const
        {
            return Ice::uncheckedCast<T>(_proxy);
        }

        DataStormContract::NodePrx getNode() const;
        void setNode(DataStormContract::NodePrx);

        std::unique_lock<std::mutex>& getTopicLock() { return *_topicLock; }

        void subscribe(std::int64_t, TopicI*);
        void unsubscribe(std::int64_t, TopicI*);
        void disconnect(std::int64_t, TopicI*);

        void subscribeToKey(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<DataElementI>&,
            const std::string&,
            const std::shared_ptr<Key>&,
            std::int64_t,
            const std::string&,
            int);
        void unsubscribeFromKey(std::int64_t, std::int64_t, const std::shared_ptr<DataElementI>&, std::int64_t);
        void disconnectFromKey(std::int64_t, std::int64_t, const std::shared_ptr<DataElementI>&, std::int64_t);

        void subscribeToFilter(
            std::int64_t,
            std::int64_t,
            const std::shared_ptr<DataElementI>&,
            const std::string&,
            const std::shared_ptr<Key>&,
            const std::string&,
            int);
        void unsubscribeFromFilter(std::int64_t, std::int64_t, const std::shared_ptr<DataElementI>&, std::int64_t);
        void disconnectFromFilter(std::int64_t, std::int64_t, const std::shared_ptr<DataElementI>&, std::int64_t);

        /**
         * Return a map containing the last sample IDs read by the subscribers of the given topic and key.
         *
         * @param topic The ID of the peer topic.
         * @param key The ID of the peer key.
         * @param element The data element.
         */
        DataStormContract::LongLongDict
        getLastIds(std::int64_t topic, std::int64_t key, const std::shared_ptr<DataElementI>& element);

        std::vector<std::shared_ptr<Sample>> subscriberInitialized(
            std::int64_t,
            std::int64_t,
            const DataStormContract::DataSampleSeq&,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<DataElementI>&);

    protected:
        void runWithTopics(
            const std::string&,
            std::vector<std::shared_ptr<TopicI>>&,
            std::function<void(const std::shared_ptr<TopicI>&)>);
        void runWithTopics(std::int64_t, std::function<void(TopicI*, TopicSubscriber&)>);
        void runWithTopics(std::int64_t, std::function<void(TopicI*, TopicSubscriber&, TopicSubscribers&)>);
        void runWithTopic(std::int64_t, TopicI*, std::function<void(TopicSubscriber&)>);

        virtual std::vector<std::shared_ptr<TopicI>> getTopics(const std::string&) const = 0;
        virtual void reconnect(DataStormContract::NodePrx) = 0;
        virtual void remove() = 0;

        const std::shared_ptr<Instance> _instance;
        std::shared_ptr<TraceLevels> _traceLevels;
        mutable std::mutex _mutex;
        std::shared_ptr<NodeI> _parent;
        std::string _id;
        DataStormContract::SessionPrx _proxy;
        DataStormContract::NodePrx _node;
        bool _destroyed;
        int _sessionInstanceId;
        int _retryCount;
        IceInternal::TimerTaskPtr _retryTask;

        // Keeps track of the topics that this session is subscribed to. The key represents the topic ID in the remote
        // node.
        std::map<std::int64_t, TopicSubscribers> _topics;
        std::unique_lock<std::mutex>* _topicLock;

        std::optional<DataStormContract::SessionPrx> _session;
        Ice::ConnectionPtr _connection;
    };

    class SubscriberSessionI : public SessionI, public DataStormContract::SubscriberSession
    {
    public:
        SubscriberSessionI(const std::shared_ptr<NodeI>&, DataStormContract::NodePrx, DataStormContract::SessionPrx);

        void s(std::int64_t, std::int64_t, DataStormContract::DataSample, const Ice::Current&) final;

    private:
        std::vector<std::shared_ptr<TopicI>> getTopics(const std::string&) const final;
        void reconnect(DataStormContract::NodePrx) final;
        void remove() final;
    };

    class PublisherSessionI : public SessionI, public DataStormContract::PublisherSession
    {
    public:
        PublisherSessionI(const std::shared_ptr<NodeI>&, DataStormContract::NodePrx, DataStormContract::SessionPrx);

    private:
        std::vector<std::shared_ptr<TopicI>> getTopics(const std::string&) const final;
        void reconnect(DataStormContract::NodePrx) final;
        void remove() final;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
