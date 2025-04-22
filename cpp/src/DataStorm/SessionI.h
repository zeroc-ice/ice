// Copyright (c) ZeroC, Inc.

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
        /// The element subscriber structure contains the details of a local data element subscribing to a remote data
        /// element.
        struct ElementSubscriber
        {
            ElementSubscriber(std::string facet, std::shared_ptr<Key> key, int sessionInstanceId)
                : facet(std::move(facet)),
                  sessionInstanceId(sessionInstanceId)
            {
                keys.insert(std::move(key));
            }

            const std::string facet;
            bool initialized{false};
            // The ID of the last processed sample.
            std::int64_t lastId{0};
            std::set<std::shared_ptr<Key>> keys;
            int sessionInstanceId;
        };

        class ElementSubscribers
        {
        public:
            ElementSubscribers(std::string name, int priority) : name(std::move(name)), priority(priority) {}

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

            [[nodiscard]] std::map<std::shared_ptr<DataElementI>, ElementSubscriber>& getSubscribers()
            {
                return _subscribers;
            }

            [[nodiscard]] ElementSubscriber* getSubscriber(const std::shared_ptr<DataElementI>& element)
            {
                auto p = _subscribers.find(element);
                if (p != _subscribers.end())
                {
                    return &p->second;
                }
                return nullptr;
            }

            [[nodiscard]] bool reap(int sessionInstanceId)
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
            // Each entry in the map represents a subscriber to the same remote element. The key is a pointer to the
            // local data element subscribing to the remote data element, and the ElementSubscriber object contains the
            // subscription details.
            std::map<std::shared_ptr<DataElementI>, ElementSubscriber> _subscribers;
            int _sessionInstanceId{0};
        };

        /// Represents the subscription from a local topic object to a remote topic.
        class TopicSubscriber
        {
        public:
            TopicSubscriber(int sessionInstanceId) : sessionInstanceId(sessionInstanceId) {}

            [[nodiscard]] ElementSubscribers* add(std::int64_t id, std::string name, int priority)
            {
                auto p = _elements.find(id);
                if (p == _elements.end())
                {
                    p = _elements.emplace(id, ElementSubscribers(std::move(name), priority)).first;
                }
                return &p->second;
            }

            [[nodiscard]] ElementSubscribers* get(std::int64_t elementId)
            {
                auto p = _elements.find(elementId);
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
                    ElementSubscribers tmp{std::move(p->second)};
                    _elements.erase(p);
                    return tmp;
                }
                return {"", 0};
            }

            [[nodiscard]] std::map<std::int64_t, ElementSubscribers>& getAll() { return _elements; }

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

            // A map of remote keys to local keys and the number of local elements subscribing to a remote element.
            // - Key: The remote key ID.
            // - Value: A pair containing the local key and a map of remote element IDs to the number of local elements
            // subscribing to the remote element.
            std::map<std::int64_t, std::pair<std::shared_ptr<Key>, std::map<std::int64_t, int>>> keys;

            std::map<std::int64_t, std::shared_ptr<Tag>> tags;
            int sessionInstanceId;

        private:
            // This map tracks subscriptions to remote elements.
            // - Key: The remote element ID.
            // - Value: An `ElementSubscribers` object containing details of the local data elements subscribing to the
            // remote data element.
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

            [[nodiscard]] TopicSubscriber& getSubscriber(TopicI* topic)
            {
                assert(_subscribers.find(topic) != _subscribers.end());
                return _subscribers.at(topic);
            }

            void removeSubscriber(TopicI* topic) { _subscribers.erase(topic); }

            std::map<TopicI*, TopicSubscriber>& getSubscribers() { return _subscribers; }

            // Determine if the subscriber should be reaped.
            [[nodiscard]] bool reap(int sessionInstanceId)
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

            // The session instance id is incremented each time a session is reconnected. Subscribers with a different
            // session instance id can be discarded.
            int _sessionInstanceId;
        };

    public:
        SessionI(
            std::shared_ptr<Instance>,
            std::shared_ptr<NodeI>,
            DataStormContract::NodePrx,
            DataStormContract::SessionPrx);
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
        [[nodiscard]] bool disconnected(const Ice::ConnectionPtr&, std::exception_ptr);
        [[nodiscard]] bool retry(DataStormContract::NodePrx, std::exception_ptr);
        void destroyImpl(const std::exception_ptr&);

        [[nodiscard]] const std::string& getId() const { return _id; }

        [[nodiscard]] Ice::ConnectionPtr getConnection() const;
        [[nodiscard]] std::optional<DataStormContract::SessionPrx> getSession() const;
        [[nodiscard]] bool checkSession();

        [[nodiscard]] DataStormContract::SessionPrx getProxy() const { return _proxy; }

        [[nodiscard]] DataStormContract::NodePrx getNode() const;
        void setNode(DataStormContract::NodePrx);

        [[nodiscard]] std::unique_lock<std::mutex>& getTopicLock() { return *_topicLock; }

        void subscribe(std::int64_t, TopicI*);
        void unsubscribe(std::int64_t, TopicI*);
        void disconnect(std::int64_t, TopicI*);

        void subscribeToKey(
            std::int64_t topicId,
            std::int64_t elementId,
            const std::shared_ptr<DataElementI>& element,
            const std::string& facet,
            const std::shared_ptr<Key>& key,
            std::int64_t keyId,
            const std::string& name,
            int priority);

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

        /// Retrieves a map of the last sample IDs read by a data element for a specified topic and key.
        ///
        /// @param topic The unique identifier of the topic.
        /// @param key The unique identifier of the ke.
        /// @param element The data element for which the last sample IDs are retrieved.
        /// @return A map where the key represents the remote element ID, and the value represents the last sample ID
        /// read by the specified data element.
        [[nodiscard]] DataStormContract::LongLongDict
        getLastIds(std::int64_t topic, std::int64_t key, const std::shared_ptr<DataElementI>& element);

        [[nodiscard]] std::vector<std::shared_ptr<Sample>> subscriberInitialized(
            std::int64_t,
            std::int64_t,
            const DataStormContract::DataSampleSeq&,
            const std::shared_ptr<Key>&,
            const std::shared_ptr<DataElementI>&);

        /// Attempts to reconnect the session with the specified node.
        ///
        /// @param node The node to reconnect to.
        virtual void reconnect(DataStormContract::NodePrx node) = 0;

    protected:
        /// Runs the provided callback function for each topic with the specified name.
        /// The callback is executed with the topic's mutex locked.
        ///
        /// The topics are appended to the `retained` vector, allowing the caller to release them after unlocking the
        /// session mutex. This method must be called with the session mutex already locked.
        ///
        /// @param name The name of the topics to process.
        /// @param retained A vector to store the topics, ensuring they are retained until released by the caller.
        /// @param callback The callback function to execute for each topic
        void runWithTopics(
            const std::string& name,
            std::vector<std::shared_ptr<TopicI>>& retained,
            const std::function<void(const std::shared_ptr<TopicI>&)>& callback);

        /// Runs the provided callback function for each subscriber of the specified topic.
        /// The callback is executed with the topic's mutex locked.
        ///
        /// @param topicId The ID of the topic to process.
        /// @param callback The callback function to execute for each subscriber.
        void runWithTopics(std::int64_t topicId, const std::function<void(TopicI*, TopicSubscriber&)>& callback);

        /// Runs the provided callback function for the specified topic, if it is among the subscribers for the given
        /// topic ID.
        /// The callback is executed with the topic's mutex locked.
        ///
        /// @param topicId The ID of the topic to process.
        /// @param topic The topic to process.
        /// @param callback The callback function to execute for the subscriber.
        void runWithTopic(std::int64_t topicId, TopicI* topic, const std::function<void(TopicSubscriber&)>& callback);

        /// Returns the topics that match the specified name.
        ///
        /// This method is implemented by the derived classes `PublisherSessionI` and `SubscriberSessionI`:
        /// - `PublisherSessionI` returns the topic writers for the specified topic name.
        /// - `SubscriberSessionI` returns the topic readers for the specified topic name.
        ///
        /// @param name The name of the topics to retrieve.
        /// @return A vector containing the topics that match the specified name.
        virtual std::vector<std::shared_ptr<TopicI>> getTopics(const std::string& name) const = 0;

        virtual void remove() = 0;

        const std::shared_ptr<Instance> _instance;
        std::shared_ptr<TraceLevels> _traceLevels;
        mutable std::mutex _mutex;

        // The parent node that created this session.
        std::shared_ptr<NodeI> _parent;

        // The proxy representing this session instance.
        DataStormContract::SessionPrx _proxy;

        // The stringified identity of the session.
        std::string _id;

        // The proxy to the peer node that established the session.
        DataStormContract::NodePrx _node;

        // Indicates whether the session has been destroyed.
        bool _destroyed{false};

        // The instance ID of the session, incremented each time the session is reconnected.
        int _sessionInstanceId{0};

        // The number of attempts made to reconnect the session.
        int _retryCount{0};

        // A retry task, scheduled if an attempt to reconnect the session is underway; nullptr if no retry is scheduled.
        IceInternal::TimerTaskPtr _retryTask;

        // Tracks the topics this session is subscribed to.
        // - Key: The topic ID on the remote node.
        // - Value: A `TopicSubscribers` object containing the subscribers for the remote topic.
        std::map<std::int64_t, TopicSubscribers> _topics;

        // The lock of the topic being processed by the callback function. See runWithTopics.
        std::unique_lock<std::mutex>* _topicLock;

        // The proxy to the peer session, or `std::nullopt` if the session is disconnected.
        std::optional<DataStormContract::SessionPrx> _session;

        // The connection to the peer node, or `nullptr` if the session is disconnected.
        Ice::ConnectionPtr _connection;
    };

    class SubscriberSessionI : public SessionI, public DataStormContract::SubscriberSession
    {
    public:
        SubscriberSessionI(
            std::shared_ptr<Instance>,
            std::shared_ptr<NodeI>,
            DataStormContract::NodePrx,
            DataStormContract::SessionPrx);

        void s(std::int64_t, std::int64_t, DataStormContract::DataSample, const Ice::Current&) final;
        void reconnect(DataStormContract::NodePrx) final;

    private:
        [[nodiscard]] std::vector<std::shared_ptr<TopicI>> getTopics(const std::string&) const final;
        void remove() final;
    };

    class PublisherSessionI : public SessionI, public DataStormContract::PublisherSession
    {
    public:
        PublisherSessionI(
            std::shared_ptr<Instance>,
            std::shared_ptr<NodeI>,
            DataStormContract::NodePrx,
            DataStormContract::SessionPrx);
        void reconnect(DataStormContract::NodePrx) final;

    private:
        [[nodiscard]] std::vector<std::shared_ptr<TopicI>> getTopics(const std::string&) const final;
        void remove() final;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
