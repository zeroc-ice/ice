// Copyright (c) ZeroC, Inc.

#include "SessionI.h"
#include "CallbackExecutor.h"
#include "ConnectionManager.h"
#include "Instance.h"
#include "NodeI.h"
#include "TopicFactoryI.h"
#include "TopicI.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;
using namespace Ice;

namespace
{
    class DispatchInterceptorI : public Object
    {
    public:
        DispatchInterceptorI(ObjectPtr servant, shared_ptr<CallbackExecutor> executor)
            : _servant{std::move(servant)},
              _executor{std::move(executor)}
        {
        }

        void dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse) final
        {
            _servant->dispatch(request, sendResponse);
            // Flush the callback executor to ensure that any callbacks queued by the dispatch are executed.
            _executor->flush();
        }

    private:
        ObjectPtr _servant;
        shared_ptr<CallbackExecutor> _executor;
    };
}

SessionI::SessionI(shared_ptr<Instance> instance, shared_ptr<NodeI> parent, NodePrx node, SessionPrx proxy)
    : _instance{std::move(instance)},
      _traceLevels{_instance->getTraceLevels()},
      _parent{std::move(parent)},
      _proxy{std::move(proxy)},
      _id{identityToString(_proxy->ice_getIdentity())},
      _node{std::move(node)}
{
}

void
SessionI::init()
{
    // Even though the node register a default servant for sessions, we still need to register the session servant
    // explicitly here to ensure collocation works. The default servant from the node is used for facet calls.
    _instance->getObjectAdapter()->add(
        make_shared<DispatchInterceptorI>(shared_from_this(), _instance->getCallbackExecutor()),
        _proxy->ice_getIdentity());

    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": created session (peer = '" << _node << "')";
    }
}

void
SessionI::announceTopics(TopicInfoSeq topics, bool, const Current&)
{
    // Retain topics outside the synchronization. This is necessary to ensure the topic destructor doesn't get called
    // within the synchronization. The topic destructor can callback on the session to disconnect.
    vector<shared_ptr<TopicI>> retained;
    {
        lock_guard<mutex> lock(_mutex);
        if (!_session)
        {
            return;
        }

        if (_traceLevels->session > 2)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << _id << ": announcing topics '" << topics << "'";
        }

        for (const auto& info : topics)
        {
            // For each local topic matching the given name:
            // - Attach the topic to all instances of the remote topic, with each instance represented by an entry in
            //   the TopicInfo::ids sequence.
            // - Send an attachTopic request to the peer session to attach to the matching topic.
            runWithTopics(
                info.name,
                retained,
                [&](const shared_ptr<TopicI>& topic)
                {
                    for (const auto& id : info.ids)
                    {
                        topic->attach(id, shared_from_this(), *_session);
                    }
                    _session->attachTopicAsync(topic->getTopicSpec(), nullptr);
                });
        }

        // Reap dead topics corresponding to subscriptions from a previous session instance ID. Subscribers from the
        // previous session instance ID that were not reattached to the new session instance ID are removed.
        auto p = _topics.begin();
        while (p != _topics.end())
        {
            if (p->second.reap(_sessionInstanceId))
            {
                _topics.erase(p++);
            }
            else
            {
                ++p;
            }
        }
    }
}

void
SessionI::attachTopic(TopicSpec spec, const Current&)
{
    // Retain topics outside the synchronization. This is necessary to ensure the topic destructor doesn't get called
    // within the synchronization. The topic destructor can callback on the session to disconnect.
    vector<shared_ptr<TopicI>> retained;
    {
        lock_guard<mutex> lock(_mutex);
        if (!_session)
        {
            // Ignore the session was disconnected.
            return;
        }

        runWithTopics(
            spec.name,
            retained,
            [&](const shared_ptr<TopicI>& topic)
            {
                if (_traceLevels->session > 2)
                {
                    Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                    out << _id << ": attaching topic '" << spec << "' to '" << topic << "'";
                }

                // Attach local topics with the given name to the remote topic.
                topic->attach(spec.id, shared_from_this(), *_session);

                // If the topic spec has tags, decode them and add them to the subscriber.
                if (!spec.tags.empty())
                {
                    auto& subscriber = _topics.at(spec.id).getSubscriber(topic.get());
                    for (const auto& tag : spec.tags)
                    {
                        subscriber.tags[tag.id] =
                            topic->getTagFactory()->decode(_instance->getCommunicator(), tag.value);
                    }
                }

                // Provide the local tags to the remote topic by calling attachTagsAsync.
                auto tags = topic->getTags();
                if (!tags.empty())
                {
                    _session->attachTagsAsync(topic->getId(), tags, true, nullptr);
                }

                // Attach to the remote elements that match the local elements.
                auto specs = topic->getElementSpecs(spec.id, spec.elements, shared_from_this());
                if (!specs.empty())
                {
                    if (_traceLevels->session > 2)
                    {
                        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                        out << _id << ": matched elements '" << spec << "' on '" << topic << "'";
                    }
                    // Don't wait for the response here, the peer session will send an ack.
                    _session->attachElementsAsync(topic->getId(), specs, true, nullptr);
                }
            });
    }
}

void
SessionI::detachTopic(int64_t id, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }

    runWithTopics(
        id,
        [&](TopicI* topic, TopicSubscriber&)
        {
            if (_traceLevels->session > 2)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": detaching topic '" << id << "' from '" << topic << "'";
            }
            topic->detach(id, shared_from_this());
        });

    // Erase the topic
    _topics.erase(id);
}

void
SessionI::attachTags(int64_t topicId, ElementInfoSeq tags, bool initialize, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }

    runWithTopics(
        topicId,
        [&](TopicI* topic, TopicSubscriber& subscriber)
        {
            if (_traceLevels->session > 2)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": attaching tags '[" << tags << "]@" << topicId << "' on topic '" << topic << "'";
            }

            if (initialize)
            {
                subscriber.tags.clear();
            }

            for (const auto& tag : tags)
            {
                subscriber.tags[tag.id] = topic->getTagFactory()->decode(_instance->getCommunicator(), tag.value);
            }
        });
}

void
SessionI::detachTags(int64_t topicId, LongSeq tags, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }

    runWithTopics(
        topicId,
        [&](TopicI* topic, TopicSubscriber& subscriber)
        {
            if (_traceLevels->session > 2)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": detaching tags '[" << tags << "]@" << topicId << "' on topic '" << topic << "'";
            }

            for (const auto& tag : tags)
            {
                subscriber.tags.erase(tag);
            }
        });
}

void
SessionI::announceElements(int64_t topicId, ElementInfoSeq elements, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }

    runWithTopics(
        topicId,
        [&](TopicI* topic, TopicSubscriber&)
        {
            if (_traceLevels->session > 2)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": announcing elements '[" << elements << "]@" << topicId << "' on topic '" << topic
                    << "'";
            }

            auto specs = topic->getElementSpecs(topicId, elements, shared_from_this());
            if (!specs.empty())
            {
                if (_traceLevels->session > 2)
                {
                    Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                    out << _id << ": announcing elements matched '[" << specs << "]@" << topicId << "' on topic '"
                        << topic << "'";
                }
                _session->attachElementsAsync(topic->getId(), specs, false, nullptr);
            }
        });
}

void
SessionI::attachElements(int64_t topicId, ElementSpecSeq elements, bool initialize, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }

    auto now = chrono::system_clock::now();
    // For each local topic that has a subscriber for the remote topic:
    // - Attach the elements to the topic.
    // - ACK the attached elements to the peer session.
    runWithTopics(
        topicId,
        [&](TopicI* topic, TopicSubscriber& subscriber)
        {
            if (_traceLevels->session > 2)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": attaching elements '[" << elements << "]@" << topicId << "' on topic '" << topic
                    << "'";
                if (initialize)
                {
                    out << " (initializing)";
                }
            }

            auto specAck = topic->attachElements(topicId, elements, shared_from_this(), *_session, now);

            if (initialize)
            {
                // Reap unused keys and filters from the topic subscriber
                subscriber.reap(_sessionInstanceId);

                // TODO: reap keys / filters
            }

            if (!specAck.empty())
            {
                if (_traceLevels->session > 2)
                {
                    Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                    out << _id << ": attaching elements matched '[" << specAck << "]@" << topicId << "' on topic '"
                        << topic << "'";
                }
                _session->attachElementsAckAsync(topic->getId(), specAck, nullptr);
            }
        });
}

void
SessionI::attachElementsAck(int64_t topicId, ElementSpecAckSeq elements, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }
    auto now = chrono::system_clock::now();
    runWithTopics(
        topicId,
        [&](TopicI* topic, TopicSubscriber&)
        {
            if (_traceLevels->session > 2)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": attaching elements ack '[" << elements << "]@" << topicId << "' on topic '" << topic
                    << "'";
            }

            LongSeq removedIds;
            auto samples = topic->attachElementsAck(topicId, elements, shared_from_this(), *_session, now, removedIds);
            if (!samples.empty())
            {
                if (_traceLevels->session > 2)
                {
                    Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                    out << _id << ": initializing elements '[" << samples << "]@" << topicId << "' on topic '" << topic
                        << "'";
                }
                _session->initSamplesAsync(topic->getId(), samples, nullptr);
            }

            if (!removedIds.empty())
            {
                _session->detachElementsAsync(topic->getId(), removedIds, nullptr);
            }
        });
}

void
SessionI::detachElements(int64_t id, LongSeq elements, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }

    runWithTopics(
        id,
        [&](TopicI* topic, TopicSubscriber& subscriber)
        {
            if (_traceLevels->session > 2)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": detaching elements '[" << elements << "]@" << id << "' on topic '" << topic << "'";
            }

            for (const auto& element : elements)
            {
                auto k = subscriber.remove(element);
                for (const auto& [dataElement, elementSubscriber] : k.getSubscribers())
                {
                    for (const auto& key : elementSubscriber.keys)
                    {
                        if (element > 0)
                        {
                            dataElement->detachKey(id, element, key, shared_from_this(), elementSubscriber.facet, true);
                        }
                        else
                        {
                            dataElement
                                ->detachFilter(id, -element, key, shared_from_this(), elementSubscriber.facet, true);
                        }
                    }
                }
            }
        });
}

void
SessionI::initSamples(int64_t topicId, DataSamplesSeq samplesSeq, const Current&)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session)
    {
        return;
    }

    auto now = chrono::system_clock::now();
    auto communicator = _instance->getCommunicator();
    for (const auto& samples : samplesSeq)
    {
        runWithTopics(
            topicId,
            [&](TopicI* topic, TopicSubscriber& subscriber)
            {
                ElementSubscribers* elementSubscribers = subscriber.get(samples.id);
                if (elementSubscribers)
                {
                    if (_traceLevels->session > 2)
                    {
                        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                        out << _id << ": initializing samples from '" << samples.id << "'"
                            << " on [";
                        for (auto q = elementSubscribers->getSubscribers().begin();
                             q != elementSubscribers->getSubscribers().end();
                             ++q)
                        {
                            if (q != elementSubscribers->getSubscribers().begin())
                            {
                                out << ", ";
                            }
                            out << q->first;
                            if (!q->second.facet.empty())
                            {
                                out << ":" << q->second.facet;
                            }
                        }
                        out << "]";
                    }

                    vector<shared_ptr<Sample>> samplesI;
                    samplesI.reserve(samples.samples.size());
                    const auto& sampleFactory = topic->getSampleFactory();
                    for (const auto& sample : samples.samples)
                    {
                        shared_ptr<Key> key;
                        if (sample.keyValue.empty())
                        {
                            key = subscriber.keys[sample.keyId].first;
                        }
                        else
                        {
                            key = topic->getKeyFactory()->decode(_instance->getCommunicator(), sample.keyValue);
                        }
                        assert(key);

                        samplesI.push_back(sampleFactory->create(
                            _id,
                            elementSubscribers->name,
                            sample.id,
                            sample.event,
                            key,
                            subscriber.tags[sample.tag],
                            sample.value,
                            sample.timestamp));
                    }

                    for (auto& [element, elementSubscriber] : elementSubscribers->getSubscribers())
                    {
                        if (!elementSubscriber.initialized)
                        {
                            elementSubscriber.initialized = true;
                            if (!samplesI.empty())
                            {
                                elementSubscriber.lastId = samplesI.back()->id;
                                element->initSamples(
                                    samplesI,
                                    topicId,
                                    samples.id,
                                    elementSubscribers->priority,
                                    now,
                                    samples.id < 0);
                            }
                        }
                    }
                }
            });
    }
}

void
SessionI::disconnected(const Current& current)
{
    if (disconnected(current.con, nullptr))
    {
        if (!retry(getNode(), nullptr))
        {
            remove();
        }
    }
}

void
SessionI::connected(SessionPrx session, const ConnectionPtr& newConnection, const TopicInfoSeq& topics)
{
    lock_guard<mutex> lock(_mutex);
    if (_destroyed || _session)
    {
        // Nothing to do, we are either destroyed or already connected.
        return;
    }

    _session = std::move(session);
    _connection = newConnection;
    if (newConnection)
    {
        auto self = shared_from_this();

        // Register a callback with the connection manager to reconnect the session if the connection is closed.
        _instance->getConnectionManager()->add(
            newConnection,
            self,
            [self](const auto& connection, auto ex)
            {
                if (self->disconnected(connection, ex))
                {
                    if (!self->retry(self->getNode(), nullptr))
                    {
                        self->remove();
                    }
                }
            });
    }

    if (_retryTask)
    {
        _instance->cancelTimerTask(_retryTask);
        _retryTask = nullptr;
    }

    ++_sessionInstanceId;

    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": session '" << _session->ice_getIdentity() << "' connected";
        if (_connection)
        {
            out << "\n" << _connection->toString();
        }
    }

    if (!topics.empty())
    {
        try
        {
            // Announce the topics to the peer, don't wait for the result.
            _session->announceTopicsAsync(topics, true, nullptr);
        }
        catch (const LocalException&)
        {
            // Ignore
        }
    }
}

bool
SessionI::disconnected(const ConnectionPtr& connection, exception_ptr ex)
{
    lock_guard<mutex> lock(_mutex);
    if (_destroyed)
    {
        // Ignore already destroyed.
        return false;
    }
    else if (!_session)
    {
        // A recovery attempt was in progress and failed. Return true to let the caller retry.
        return true;
    }
    else if (connection && _connection != connection)
    {
        // Ignore the session has already reconnected using a new connection.
        return false;
    }

    if (_traceLevels->session > 0)
    {
        if (ex)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                out << _id << ": session '" << _session->ice_getIdentity() << "' disconnected:\n";
                out << (_connection ? _connection->toString() : "<no connection>") << "\n";
                out << e.what();
            }
        }
        else
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << _id << ": session '" << _session->ice_getIdentity()
                << "' disconnected, after receiving disconnected notification from the peer:\n";
            out << (_connection ? _connection->toString() : "<no connection>") << "\n";
        }
    }

    // Detach all topics from the session.
    auto self = shared_from_this();
    for (const auto& [topicId, _] : _topics)
    {
        runWithTopics(topicId, [topicId, self](TopicI* topic, TopicSubscriber&) { topic->detach(topicId, self); });
    }

    _session = nullopt;
    _connection = nullptr;
    _retryCount = 0;
    return true;
}

bool
SessionI::retry(NodePrx node, exception_ptr exception)
{
    lock_guard<mutex> lock(_mutex);

    if (exception)
    {
        // Don't retry if we are shutting down.
        try
        {
            rethrow_exception(exception);
        }
        catch (const SessionCreationException& ex)
        {
            if (ex.error == SessionCreationError::NodeShutdown)
            {
                // Don't need to retry if the target node is being shut down.
                return false;
            }
            // Else, for other error codes we can retry.
        }
        catch (const ObjectAdapterDestroyedException&)
        {
            return false;
        }
        catch (const CommunicatorDestroyedException&)
        {
            return false;
        }
        catch (const std::exception&)
        {
            // Ignore other exceptions and retry.
        }
    }

    // Cancel any pending retry task, before we start a new one below.
    if (_retryTask)
    {
        _instance->cancelTimerTask(_retryTask);
        _retryTask = nullptr;
    }

    if (node->ice_getEndpoints().empty() && node->ice_getAdapterId().empty())
    {
        // We cannot retry because we don't have the peer endpoints. Wait twice the last retry interval for the peer to
        // reconnect.
        auto delay = _instance->getRetryDelay(_instance->getRetryCount()) * 2;

        if (_traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << _id << ": can't retry connecting to '" << node->ice_toString() << "', waiting " << delay.count()
                << " (ms) for peer to reconnect";
        }

        // Schedule a timer to remove the session if the peer doesn't reconnect.
        _retryTask = make_shared<IceInternal::InlineTimerTask>([self = shared_from_this()] { self->remove(); });
        _instance->scheduleTimerTask(_retryTask, delay);
    }
    else
    {
        // Schedule a timer to retry. Always retry immediately on the first attempt.
        auto delay = _retryCount == 0 ? 0ms : _instance->getRetryDelay(_retryCount - 1);
        // Increment the retry count, it is reset by disconnected.
        ++_retryCount;

        if (_traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            if (_retryCount <= _instance->getRetryCount())
            {
                out << _id << ": retrying connecting to '" << node->ice_toString() << "' in " << delay.count();
                out << " (ms), retry " << _retryCount << '/' << _instance->getRetryCount();
            }
            else
            {
                out << _id << ": connection to '" << node->ice_toString()
                    << "' failed and the retry limit has been reached";
            }

            if (exception)
            {
                try
                {
                    rethrow_exception(exception);
                }
                catch (const std::exception& ex)
                {
                    out << '\n' << ex.what();
                }
            }
        }

        if (_retryCount > _instance->getRetryCount())
        {
            return false;
        }

        _retryTask = make_shared<IceInternal::InlineTimerTask>([node = std::move(node), self = shared_from_this()]
                                                               { self->reconnect(node); });
        _instance->scheduleTimerTask(_retryTask, delay);
    }
    return true;
}

void
SessionI::destroyImpl(const exception_ptr& ex)
{
    lock_guard<mutex> lock(_mutex);
    assert(!_destroyed);
    _destroyed = true;

    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": destroyed session";
        if (ex)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const LocalException& e)
            {
                out << ":\n" << e.what() << "\n" << e.ice_stackTrace();
            }
            catch (const exception& e)
            {
                out << ":\n" << e.what();
            }
            catch (...)
            {
                out << ":\n unexpected exception";
            }
        }
    }

    if (_session)
    {
        if (_connection)
        {
            _instance->getConnectionManager()->remove(shared_from_this(), _connection);
        }

        _session = nullopt;
        _connection = nullptr;

        auto self = shared_from_this();
        for (const auto& t : _topics)
        {
            runWithTopics(t.first, [id = t.first, self](TopicI* topic, TopicSubscriber&) { topic->detach(id, self); });
        }
        _topics.clear();
    }

    try
    {
        _instance->getObjectAdapter()->remove(_proxy->ice_getIdentity());
    }
    catch (const ObjectAdapterDestroyedException&)
    {
    }
}

ConnectionPtr
SessionI::getConnection() const
{
    lock_guard<mutex> lock(_mutex);
    return _connection;
}

bool
SessionI::checkSession()
{
    while (true)
    {
        unique_lock<mutex> lock(_mutex);
        if (_session)
        {
            if (_connection)
            {
                // Make sure the connection is still established. It's possible that the connection got closed and we
                // were not notified yet by the connection manager. Check session explicitly check for the connection
                // to make sure that if we get a session creation request from a peer (which might detect the connection
                // closure before), it doesn't get ignored.
                try
                {
                    _connection->throwException();
                }
                catch (const LocalException&)
                {
                    auto connection = _connection;
                    lock.unlock();
                    if (!disconnected(connection, current_exception()))
                    {
                        continue;
                    }
                    return false;
                }
            }
            return true;
        }
        else
        {
            return false;
        }
    }
}

optional<SessionPrx>
SessionI::getSession() const
{
    lock_guard<mutex> lock(_mutex);
    return _session;
}

NodePrx
SessionI::getNode() const
{
    lock_guard<mutex> lock(_mutex);
    return _node;
}

void
SessionI::setNode(NodePrx node)
{
    lock_guard<mutex> lock(_mutex);
    _node = std::move(node);
}

void
SessionI::subscribe(int64_t topicId, TopicI* topic)
{
    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": subscribed topic '" << topicId << "' to topic '" << topic << "'";
    }
    // Add the topic as a subscriber of the remote topic with the given ID.
    _topics[topicId].addSubscriber(topic, _sessionInstanceId);
}

void
SessionI::unsubscribe(int64_t topicId, TopicI* topic)
{
    assert(_topics.find(topicId) != _topics.end());
    auto& subscriber = _topics.at(topicId).getSubscriber(topic);
    for (auto& [elementId, elementSubscribers] : subscriber.getAll())
    {
        for (auto& [element, elementSubscriber] : elementSubscribers.getSubscribers())
        {
            for (const auto& key : elementSubscriber.keys)
            {
                if (elementId > 0)
                {
                    element->detachKey(topicId, elementId, key, shared_from_this(), elementSubscriber.facet, false);
                }
                else
                {
                    element->detachFilter(topicId, -elementId, key, shared_from_this(), elementSubscriber.facet, false);
                }
            }
        }
    }

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": unsubscribed topic '" << topicId << "' from topic '" << topic << "'";
    }
}

void
SessionI::disconnect(int64_t topicId, TopicI* topic)
{
    lock_guard<mutex> lock(_mutex); // Called by TopicI::destroy
    if (!_session)
    {
        return;
    }

    if (_topics.find(topicId) == _topics.end())
    {
        return; // Peer topic detached first.
    }

    runWithTopic(topicId, topic, [&](TopicSubscriber&) { unsubscribe(topicId, topic); });

    auto& subscriber = _topics.at(topicId);
    subscriber.removeSubscriber(topic);
    if (subscriber.getSubscribers().empty())
    {
        _topics.erase(topicId);
    }
}

void
SessionI::subscribeToKey(
    int64_t topicId,
    int64_t elementId,
    const std::shared_ptr<DataElementI>& element,
    const string& facet,
    const shared_ptr<Key>& key,
    int64_t keyId,
    const string& name,
    int priority)
{
    // Called with the session and topic mutex locked.
    assert(_topics.find(topicId) != _topics.end());
    TopicSubscriber& subscriber = _topics.at(topicId).getSubscriber(element->getTopic());
    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": subscribed 'e" << elementId << ":[k" << keyId << "]@" << topicId << "' to '" << element << "'";
        if (!facet.empty())
        {
            out << " (facet=" << facet << ')';
        }
    }

    subscriber.add(elementId, name, priority)->addSubscriber(element, key, facet, _sessionInstanceId);

    auto& p = subscriber.keys[keyId];
    if (!p.first)
    {
        p.first = key;
    }
    ++p.second[elementId];
}

void
SessionI::unsubscribeFromKey(
    int64_t topicId,
    int64_t elementId,
    const std::shared_ptr<DataElementI>& element,
    int64_t keyId)
{
    assert(_topics.find(topicId) != _topics.end());
    TopicSubscriber& topicSubscriber = _topics.at(topicId).getSubscriber(element->getTopic());
    ElementSubscribers* elementSubscribers = topicSubscriber.get(elementId);
    if (elementSubscribers)
    {
        if (_traceLevels->session > 1)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << _id << ": unsubscribed 'e" << elementId << "[k" << keyId << "]@" << topicId << "' from '" << element
                << "'";
        }
        elementSubscribers->removeSubscriber(element);
        if (elementSubscribers->getSubscribers().empty())
        {
            topicSubscriber.remove(elementId);
        }
    }

    auto& elementSubscribersMap = topicSubscriber.keys[keyId].second;
    if (--elementSubscribersMap[elementId] == 0)
    {
        elementSubscribersMap.erase(elementId);
        if (elementSubscribersMap.empty())
        {
            topicSubscriber.keys.erase(keyId);
        }
    }
}

void
SessionI::disconnectFromKey(
    int64_t topicId,
    int64_t elementId,
    const std::shared_ptr<DataElementI>& element,
    int64_t keyId)
{
    lock_guard<mutex> lock(_mutex); // Called by DataElementI::destroy
    if (!_session)
    {
        return;
    }

    runWithTopic(
        topicId,
        element->getTopic(),
        [&](TopicSubscriber&) { unsubscribeFromKey(topicId, elementId, element, keyId); });
}

void
SessionI::subscribeToFilter(
    int64_t topicId,
    int64_t elementId,
    const std::shared_ptr<DataElementI>& element,
    const string& facet,
    const shared_ptr<Key>& key,
    const string& name,
    int priority)
{
    assert(_topics.find(topicId) != _topics.end());
    auto& subscriber = _topics.at(topicId).getSubscriber(element->getTopic());
    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": subscribed 'e" << elementId << '@' << topicId << "' to '" << element << "'";
        if (!facet.empty())
        {
            out << " (facet=" << facet << ')';
        }
    }
    subscriber.add(-elementId, name, priority)->addSubscriber(element, key, facet, _sessionInstanceId);
}

void
SessionI::unsubscribeFromFilter(
    int64_t topicId,
    int64_t elementId,
    const std::shared_ptr<DataElementI>& element,
    int64_t)
{
    assert(_topics.find(topicId) != _topics.end());
    auto& subscriber = _topics.at(topicId).getSubscriber(element->getTopic());
    auto f = subscriber.get(-elementId);
    if (f)
    {
        if (_traceLevels->session > 1)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << _id << ": unsubscribed 'e" << elementId << '@' << topicId << "' from '" << element << "'";
        }
        f->removeSubscriber(element);
        if (f->getSubscribers().empty())
        {
            subscriber.remove(-elementId);
        }
    }
}

void
SessionI::disconnectFromFilter(
    int64_t topicId,
    int64_t elementId,
    const std::shared_ptr<DataElementI>& element,
    int64_t filterId)
{
    lock_guard<mutex> lock(_mutex); // Called by DataElementI::destroy
    if (!_session)
    {
        return;
    }

    runWithTopic(
        topicId,
        element->getTopic(),
        [&](TopicSubscriber&) { unsubscribeFromFilter(topicId, elementId, element, filterId); });
}

LongLongDict
SessionI::getLastIds(int64_t topicId, int64_t keyId, const std::shared_ptr<DataElementI>& element)
{
    LongLongDict lastIds;
    auto p = _topics.find(topicId);
    if (p != _topics.end())
    {
        TopicSubscriber& subscriber = p->second.getSubscriber(element->getTopic());
        for (const auto& [elementId, _] : subscriber.keys[keyId].second)
        {
            lastIds.emplace(elementId, subscriber.get(elementId)->getSubscriber(element)->lastId);
        }
    }
    return lastIds;
}

vector<shared_ptr<Sample>>
SessionI::subscriberInitialized(
    int64_t topicId,
    int64_t elementId,
    const DataSampleSeq& samples,
    const shared_ptr<Key>& key,
    const std::shared_ptr<DataElementI>& element)
{
    // Called with the session locked, from DataElementI::attach.
    assert(_topics.find(topicId) != _topics.end());
    TopicSubscriber& subscriber = _topics.at(topicId).getSubscriber(element->getTopic());
    ElementSubscribers* elementSubscribers = subscriber.get(elementId);
    ElementSubscriber* elementSubscriber = elementSubscribers->getSubscriber(element);
    assert(elementSubscriber);

    if (_traceLevels->session > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": initialized '" << element << "' from 'e" << elementId << '@' << topicId << "'";
    }
    elementSubscriber->initialized = true;

    // If the samples collection is empty, the element subscriber's lastId remains unchanged:
    // - If no samples have been received, lastId is 0.
    // - If the element subscriber has been initialized before, lastId represents the ID of the latest received sample.
    //
    // If the samples collection is not empty:
    // - It contains samples queued in the peer writer for the element that are valid according to the element's
    //   configuration.
    // - These samples have not yet been processed by the element subscriber, according to the subscriber's lastId.
    if (samples.empty())
    {
        return {};
    }
    else
    {
        assert(samples.front().id > elementSubscriber->lastId);
        elementSubscriber->lastId = samples.back().id;

        vector<shared_ptr<Sample>> samplesI;
        samplesI.reserve(samples.size());
        auto sampleFactory = element->getTopic()->getSampleFactory();
        auto keyFactory = element->getTopic()->getKeyFactory();
        for (const auto& sample : samples)
        {
            assert((!key && !sample.keyValue.empty()) || key == subscriber.keys[sample.keyId].first);

            samplesI.push_back(sampleFactory->create(
                _id,
                elementSubscribers->name,
                sample.id,
                sample.event,
                key ? key : keyFactory->decode(_instance->getCommunicator(), sample.keyValue),
                subscriber.tags[sample.tag],
                sample.value,
                sample.timestamp));
            assert(samplesI.back()->key);
        }
        return samplesI;
    }
}

void
SessionI::runWithTopics(
    const std::string& name,
    vector<shared_ptr<TopicI>>& retained,
    const function<void(const shared_ptr<TopicI>&)>& callback)
{
    auto topics = getTopics(name);
    for (const auto& topic : topics)
    {
        retained.push_back(topic);
        unique_lock<mutex> lock(topic->getMutex());
        if (topic->isDestroyed())
        {
            continue;
        }
        _topicLock = &lock;
        callback(topic);
        _topicLock = nullptr;
    }
}

void
SessionI::runWithTopics(int64_t topicId, const function<void(TopicI*, TopicSubscriber&)>& callback)
{
    auto t = _topics.find(topicId);
    if (t != _topics.end())
    {
        for (auto& [topic, subscriber] : t->second.getSubscribers())
        {
            unique_lock<mutex> lock(topic->getMutex());
            if (topic->isDestroyed())
            {
                continue;
            }
            _topicLock = &lock;
            callback(topic, subscriber);
            _topicLock = nullptr;
        }
    }
}

void
SessionI::runWithTopic(int64_t topicId, TopicI* topic, const function<void(TopicSubscriber&)>& callback)
{
    auto t = _topics.find(topicId);
    if (t != _topics.end())
    {
        auto p = t->second.getSubscribers().find(topic);
        if (p != t->second.getSubscribers().end())
        {
            unique_lock<mutex> lock(topic->getMutex());
            if (topic->isDestroyed())
            {
                return;
            }
            _topicLock = &lock;
            callback(p->second);
            _topicLock = nullptr;
        }
    }
}

SubscriberSessionI::SubscriberSessionI(
    shared_ptr<Instance> instance,
    std::shared_ptr<NodeI> parent,
    NodePrx node,
    SessionPrx proxy)
    : SessionI(std::move(instance), std::move(parent), std::move(node), std::move(proxy))
{
}

vector<shared_ptr<TopicI>>
SubscriberSessionI::getTopics(const string& name) const
{
    return _instance->getTopicFactory()->getTopicReaders(name);
}

void
SubscriberSessionI::s(int64_t topicId, int64_t elementId, DataSample dataSample, const Current& current)
{
    lock_guard<mutex> lock(_mutex);
    if (!_session || current.con != _connection)
    {
        if (current.con != _connection)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << _id << ": discarding sample '" << dataSample.id << "' from 'e" << elementId << '@' << topicId
                << "'\n";
            if (_connection)
            {
                out << current.con->toString() << "\n" << _connection->toString();
            }
            else
            {
                out << "<not connected>";
            }
        }
        return;
    }

    // Queue the received sample with all matching subscribers.
    auto now = chrono::system_clock::now();
    runWithTopics(
        topicId,
        [&](TopicI* topic, TopicSubscriber& topicSubscriber)
        {
            auto elementSubscribers = topicSubscriber.get(elementId);
            if (elementSubscribers && !elementSubscribers->getSubscribers().empty())
            {
                if (_traceLevels->session > 2)
                {
                    Trace out(_traceLevels->logger, _traceLevels->sessionCat);
                    out << _id << ": queuing sample '" << dataSample.id << "[k" << dataSample.keyId << "]' from 'e"
                        << elementId << '@' << topicId << "'";
                    if (!current.facet.empty())
                    {
                        out << " facet=" << current.facet;
                    }
                    out << " to [";
                    bool first = true;
                    for (const auto& [element, elementSubscriber] : elementSubscribers->getSubscribers())
                    {
                        if (!first)
                        {
                            out << ", ";
                        }
                        out << element;
                        if (!elementSubscriber.facet.empty())
                        {
                            out << ":" << elementSubscriber.facet;
                        }
                        first = false;
                    }
                    out << "]";
                }

                shared_ptr<Key> key;
                if (dataSample.keyValue.empty())
                {
                    key = topicSubscriber.keys[dataSample.keyId].first;
                }
                else
                {
                    key = topic->getKeyFactory()->decode(_instance->getCommunicator(), dataSample.keyValue);
                }
                assert(key);

                auto sample = topic->getSampleFactory()->create(
                    _id,
                    elementSubscribers->name,
                    dataSample.id,
                    dataSample.event,
                    key,
                    topicSubscriber.tags[dataSample.tag],
                    dataSample.value,
                    dataSample.timestamp);

                for (auto& [element, elementSubscriber] : elementSubscribers->getSubscribers())
                {
                    if (elementSubscriber.initialized &&
                        (dataSample.keyId <= 0 || elementSubscriber.keys.find(key) != elementSubscriber.keys.end()))
                    {
                        elementSubscriber.lastId = dataSample.id;
                        element->queue(
                            sample,
                            elementSubscribers->priority,
                            shared_from_this(),
                            current.facet,
                            now,
                            !dataSample.keyValue.empty());
                    }
                }
            }
        });
}

void
SubscriberSessionI::reconnect(NodePrx node)
{
    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": trying to reconnect session with '" << node->ice_toString() << "'";
    }
    _parent->createPublisherSession(node, nullptr, dynamic_pointer_cast<SubscriberSessionI>(shared_from_this()));
}

void
SubscriberSessionI::remove()
{
    _parent->removeSubscriberSession(getNode(), dynamic_pointer_cast<SubscriberSessionI>(shared_from_this()), nullptr);
}

PublisherSessionI::PublisherSessionI(
    shared_ptr<Instance> instance,
    std::shared_ptr<NodeI> parent,
    NodePrx node,
    SessionPrx proxy)
    : SessionI(std::move(instance), std::move(parent), std::move(node), std::move(proxy))
{
}

vector<shared_ptr<TopicI>>
PublisherSessionI::getTopics(const string& name) const
{
    return _instance->getTopicFactory()->getTopicWriters(name);
}

void
PublisherSessionI::reconnect(NodePrx node)
{
    if (_traceLevels->session > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _id << ": trying to reconnect session with '" << node->ice_toString() << "'";
    }
    _parent->createSubscriberSession(node, nullptr, dynamic_pointer_cast<PublisherSessionI>(shared_from_this()));
}

void
PublisherSessionI::remove()
{
    _parent->removePublisherSession(getNode(), dynamic_pointer_cast<PublisherSessionI>(shared_from_this()), nullptr);
}
