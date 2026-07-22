// Copyright (c) ZeroC, Inc.

#include "DataElementI.h"
#include "CallbackExecutor.h"
#include "Ice/Ice.h"
#include "Instance.h"
#include "NodeI.h"
#include "TopicI.h"
#include "TraceUtil.h"

#include <algorithm>
#include <set>

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;
using namespace Ice;

namespace
{
    DataSample toSample(const shared_ptr<Sample>& sample, const CommunicatorPtr& communicator, bool marshalKey)
    {
        return DataSample{
            .id = sample->id,
            .keyId = marshalKey ? 0 : sample->key->getId(),
            .keyValue = marshalKey ? sample->key->encode(communicator) : ByteSeq{},
            .timestamp = chrono::time_point_cast<chrono::microseconds>(sample->timestamp).time_since_epoch().count(),
            .tag = sample->tag ? sample->tag->getId() : 0,
            .event = sample->event,
            .value = sample->encode(communicator)};
    }

    void cleanOldSamples(
        deque<shared_ptr<Sample>>& samples,
        const chrono::time_point<chrono::system_clock>& now,
        int lifetime)
    {
        chrono::time_point<chrono::system_clock> staleTime = now - chrono::milliseconds(lifetime);
        auto p = stable_partition(
            samples.begin(),
            samples.end(),
            [&](const shared_ptr<Sample>& s) { return s->timestamp < staleTime; });
        if (p != samples.begin())
        {
            samples.erase(samples.begin(), p);
        }
    }
}

DataElementI::DataElementI(TopicI* parent, string name, int64_t id, const DataStorm::Config& config)
    : _traceLevels(parent->instance()->getTraceLevels()),
      _name(std::move(name)),
      _id(id),
      _config(make_shared<ElementConfig>()),
      _executor(parent->instance()->getCallbackExecutor()),
      // The collocated forwarder is initalized here to avoid using a nullable proxy. The forwarder is only used by
      // the instance that owns it and is removed in destroy implementation.
      _forwarder{parent->instance()->getCollocatedForwarder()->add<SessionPrx>(
          [this](const ByteSeq& inParams, const Current& current) { forward(inParams, current); })},
      _parent(parent->shared_from_this())
{
    _config->sampleCount = config.sampleCount;
    _config->sampleLifetime = config.sampleLifetime;
    if (!_name.empty())
    {
        _config->name = _name;
    }
    if (config.clearHistory)
    {
        _config->clearHistory = static_cast<ClearHistoryPolicy>(*config.clearHistory);
    }
}

DataElementI::~DataElementI()
{
    assert(_destroyed);
    assert(_listeners.empty());
    assert(_listenerCount == 0);
}

void
DataElementI::destroy()
{
    {
        unique_lock<mutex> lock(_parent->_mutex);
        assert(!_destroyed);
        _destroyed = true;
        destroyImpl(); // Must be called first.
    }
    disconnect();
    _parent->instance()->getCollocatedForwarder()->remove(_forwarder->ice_getIdentity());
}

void
DataElementI::attach(
    int64_t topicId,
    int64_t id,
    const shared_ptr<Key>& key,
    const shared_ptr<Filter>& filter,
    const shared_ptr<SessionI>& session,
    SessionPrx prx,
    const ElementData& data,
    const chrono::time_point<chrono::system_clock>& now,
    ElementDataAckSeq& acks)
{
    shared_ptr<Filter> sampleFilter;
    if (auto info = data.config->sampleFilter)
    {
        sampleFilter = _parent->getSampleFilterFactories()->decode(getCommunicator(), info->name, info->criteria);
    }

    string facet = data.config->facet.value_or(string{});
    int priority = data.config->priority.value_or(0);

    string name;
    if (data.config->name)
    {
        name = *data.config->name;
    }
    else
    {
        ostringstream os;
        os << session->getId() << '-' << topicId << '-' << data.id;
        name = os.str();
    }

    // Attach the key or filter, and if attach success compute the ACK data to send to the peer.
    if ((id > 0 &&
         attachKey(topicId, data.id, key, sampleFilter, session, std::move(prx), facet, id, name, priority)) ||
        (id < 0 &&
         attachFilter(topicId, data.id, key, sampleFilter, session, std::move(prx), facet, id, filter, name, priority)))
    {
        auto q = data.lastIds.find(_id);
        int64_t lastId = q != data.lastIds.end() ? q->second : 0;
        LongLongDict lastIds = key ? session->getLastIds(topicId, id, shared_from_this()) : LongLongDict{};
        DataSamples samples = getSamples(key, sampleFilter, data.config, lastId, now);

        acks.push_back(ElementDataAck{
            .id = _id,
            .config = _config,
            .lastIds = std::move(lastIds),
            .samples = std::move(samples.samples),
            .peerId = data.id});
    }
}

std::function<void()>
DataElementI::attach(
    int64_t topicId,
    int64_t id,
    const shared_ptr<Key>& key,
    const shared_ptr<Filter>& filter,
    const shared_ptr<SessionI>& session,
    SessionPrx prx,
    const ElementDataAck& data,
    const chrono::time_point<chrono::system_clock>& now,
    DataSamplesSeq& samples)
{
    // Called with the topic and session from TopicI::attachElementsAck locked.
    shared_ptr<Filter> sampleFilter;
    if (auto info = data.config->sampleFilter)
    {
        sampleFilter = _parent->getSampleFilterFactories()->decode(getCommunicator(), info->name, info->criteria);
    }

    string facet = data.config->facet.value_or(string{});
    int priority = data.config->priority.value_or(0);

    string name;
    if (data.config->name)
    {
        name = *data.config->name;
    }
    else
    {
        ostringstream os;
        os << session->getId() << '-' << topicId << '-' << data.id;
        name = os.str();
    }

    // Attach a key or filter. If the attachment is successful, compute the queued samples to send to the peer.
    // - If this data element is a data reader, the computed samples will be empty.
    // - If this data element is a data writer, the computed samples will contain the samples to send to the peer
    //   based on the peer reader configuration.
    if ((id > 0 &&
         attachKey(topicId, data.id, key, sampleFilter, session, std::move(prx), facet, id, name, priority)) ||
        (id < 0 &&
         attachFilter(topicId, data.id, key, sampleFilter, session, std::move(prx), facet, id, filter, name, priority)))
    {
        auto q = data.lastIds.find(_id);
        int64_t lastId = q != data.lastIds.end() ? q->second : 0;
        samples.push_back(getSamples(key, sampleFilter, data.config, lastId, now));
    }

    auto samplesI =
        session->subscriberInitialized(topicId, id > 0 ? data.id : -data.id, data.samples, key, shared_from_this());
    if (!samplesI.empty())
    {
        return [=, samplesI = std::move(samplesI), self = shared_from_this()]()
        { self->initSamples(samplesI, topicId, data.id, priority, now, id < 0); };
    }
    return nullptr;
}

bool
DataElementI::attachKey(
    int64_t topicId,
    int64_t elementId,
    const shared_ptr<Key>& key,
    const shared_ptr<Filter>& sampleFilter,
    const shared_ptr<SessionI>& session,
    SessionPrx prx,
    const string& facet,
    int64_t keyId,
    const string& name,
    int priority)
{
    // No locking necessary, called by the session with the mutex locked

    ListenerKey listenerKey{.session = session, .facet = facet};
    auto p = _listeners.find(listenerKey);
    if (p == _listeners.end())
    {
        p = _listeners.emplace(std::move(listenerKey), Listener{std::move(prx), facet}).first;
    }

    bool added = false;
    auto subscriber = p->second.addOrGet(topicId, elementId, keyId, nullptr, sampleFilter, name, priority, added);

    if (_onConnectedElements && added)
    {
        _executor->queue([callback = _onConnectedElements, name]
                         { callback(DataStorm::CallbackReason::Connect, name); });
    }

    if (addConnectedKey(key, subscriber))
    {
        if (key)
        {
            subscriber->keys.insert(key);
        }

        if (_traceLevels->data > 1)
        {
            Trace out(_traceLevels->logger, _traceLevels->dataCat);
            out << this << ": attach e" << elementId << ":" << name;
            if (!facet.empty())
            {
                out << ":" << facet;
            }
            out << ":[" << key << "]@" << topicId;
        }

        ++_listenerCount;
        _parent->incListenerCount();
        session->subscribeToKey(topicId, elementId, shared_from_this(), facet, key, keyId, name, priority);
        notifyListenerWaiters();
        return true;
    }
    return false;
}

void
DataElementI::detachKey(
    int64_t topicId,
    int64_t elementId,
    const shared_ptr<Key>& key,
    const shared_ptr<SessionI>& session,
    const string& facet,
    bool unsubscribe)
{
    // No locking necessary, called by the session with the mutex locked
    auto p = _listeners.find({session, facet});
    if (p == _listeners.end())
    {
        return;
    }

    auto subscriber = p->second.get(topicId, elementId);
    if (removeConnectedKey(key, subscriber))
    {
        if (key)
        {
            subscriber->keys.erase(key);
        }
        if (subscriber->keys.empty())
        {
            if (_onConnectedElements)
            {
                _executor->queue([callback = _onConnectedElements, subscriber]
                                 { callback(DataStorm::CallbackReason::Disconnect, subscriber->name); });
            }
            if (p->second.remove(topicId, elementId))
            {
                _listeners.erase(p);
            }
        }

        if (_traceLevels->data > 1)
        {
            Trace out(_traceLevels->logger, _traceLevels->dataCat);
            out << this << ": detach e" << elementId << ":" << subscriber->name;
            if (!facet.empty())
            {
                out << ":" << facet;
            }
            out << ":[" << key << "]@" << topicId;
        }
        --_listenerCount;
        _parent->decListenerCount();
        if (unsubscribe)
        {
            session->unsubscribeFromKey(topicId, elementId, shared_from_this(), subscriber->id);
        }
        notifyListenerWaiters();
    }
}

bool
DataElementI::attachFilter(
    int64_t topicId,
    int64_t elementId,
    const shared_ptr<Key>& key,
    const shared_ptr<Filter>& sampleFilter,
    const shared_ptr<SessionI>& session,
    SessionPrx prx,
    const string& facet,
    int64_t subscriberId,
    const shared_ptr<Filter>& filter,
    const string& name,
    int priority)
{
    // No locking necessary, called by the session with the mutex locked
    ListenerKey listenerKey{.session = session, .facet = facet};
    auto p = _listeners.find(listenerKey);
    if (p == _listeners.end())
    {
        p = _listeners.emplace(std::move(listenerKey), Listener{std::move(prx), facet}).first;
    }

    // Negate the element ID for internal storage — filter subscriptions use negative IDs to distinguish them from
    // key subscriptions. All subsequent internal functions receive and use this negated ID directly.
    assert(elementId > 0);
    const int64_t filterId = -elementId;

    bool added = false;
    auto subscriber = p->second.addOrGet(topicId, filterId, subscriberId, filter, sampleFilter, name, priority, added);
    if (_onConnectedElements && added)
    {
        _executor->queue([callback = _onConnectedElements, name]
                         { callback(DataStorm::CallbackReason::Connect, name); });
    }

    if (addConnectedKey(key, subscriber))
    {
        if (key)
        {
            subscriber->keys.insert(key);
        }

        if (_traceLevels->data > 1)
        {
            Trace out(_traceLevels->logger, _traceLevels->dataCat);
            out << this << ": attach e" << abs(filterId) << ":" << name;
            if (!facet.empty())
            {
                out << ":" << facet;
            }
            out << ":[" << filter << "]@" << topicId;
        }

        ++_listenerCount;
        _parent->incListenerCount();
        session->subscribeToFilter(topicId, filterId, shared_from_this(), facet, key, name, priority);
        notifyListenerWaiters();
        return true;
    }
    return false;
}

void
DataElementI::detachFilter(
    int64_t topicId,
    int64_t filterId,
    const shared_ptr<Key>& key,
    const shared_ptr<SessionI>& session,
    const string& facet,
    bool unsubscribe)
{
    assert(filterId < 0);
    // No locking necessary, called by the session with the mutex locked
    auto p = _listeners.find({session, facet});
    if (p == _listeners.end())
    {
        return;
    }

    auto subscriber = p->second.get(topicId, filterId);
    if (removeConnectedKey(key, subscriber))
    {
        if (key)
        {
            subscriber->keys.erase(key);
        }
        if (subscriber->keys.empty())
        {
            if (_onConnectedElements)
            {
                _executor->queue([callback = _onConnectedElements, subscriber]
                                 { callback(DataStorm::CallbackReason::Disconnect, subscriber->name); });
            }
            if (p->second.remove(topicId, filterId))
            {
                _listeners.erase(p);
            }
        }

        if (_traceLevels->data > 1)
        {
            Trace out(_traceLevels->logger, _traceLevels->dataCat);
            out << this << ": detach e" << abs(filterId) << ":" << subscriber->name;
            if (!facet.empty())
            {
                out << ":" << facet;
            }
            out << ":[" << subscriber->filter << "]@" << topicId;
        }

        --_listenerCount;
        _parent->decListenerCount();
        if (unsubscribe)
        {
            session->unsubscribeFromFilter(topicId, filterId, shared_from_this());
        }
        notifyListenerWaiters();
    }
}

vector<shared_ptr<Key>>
DataElementI::getConnectedKeys() const
{
    unique_lock<mutex> lock(_parent->_mutex);
    vector<shared_ptr<Key>> keys;
    keys.reserve(_connectedKeys.size());
    for (const auto& key : _connectedKeys)
    {
        keys.push_back(key.first);
    }
    return keys;
}

vector<string>
DataElementI::getConnectedElements() const
{
    unique_lock<mutex> lock(_parent->_mutex);
    vector<string> elements;
    for (const auto& listener : _listeners)
    {
        for (const auto& subscriber : listener.second.subscribers)
        {
            elements.push_back(subscriber.second->name);
        }
    }
    return elements;
}

void
DataElementI::onConnectedKeys(
    function<void(vector<shared_ptr<Key>>)> init,
    function<void(DataStorm::CallbackReason, shared_ptr<Key>)> update)
{
    unique_lock<mutex> lock(_parent->_mutex);
    _onConnectedKeys = std::move(update);
    if (init)
    {
        vector<shared_ptr<Key>> keys;
        keys.reserve(_connectedKeys.size());
        for (const auto& [key, _] : _connectedKeys)
        {
            keys.push_back(key);
        }
        _executor->queue([init = std::move(init), keys = std::move(keys)]() mutable { init(std::move(keys)); }, true);
    }
}

void
DataElementI::onConnectedElements(
    function<void(vector<string>)> init,
    function<void(DataStorm::CallbackReason, string)> update)
{
    unique_lock<mutex> lock(_parent->_mutex);
    _onConnectedElements = std::move(update);
    if (init)
    {
        vector<string> elements;
        for (const auto& listener : _listeners)
        {
            for (const auto& [_, subscriber] : listener.second.subscribers)
            {
                elements.push_back(subscriber->name);
            }
        }
        _executor->queue(
            [init = std::move(init), elements = std::move(elements)]() mutable { init(std::move(elements)); },
            true);
    }
}

void
DataElementI::initSamples(
    const vector<shared_ptr<Sample>>&,
    int64_t,
    int64_t,
    int,
    const chrono::time_point<chrono::system_clock>&,
    bool)
{
}

DataSamples
DataElementI::getSamples(
    const shared_ptr<Key>&,
    const shared_ptr<Filter>&,
    const shared_ptr<DataStormContract::ElementConfig>&,
    int64_t,
    const chrono::time_point<chrono::system_clock>&)
{
    return {};
}

void
DataElementI::queue(
    const shared_ptr<Sample>&,
    int,
    const shared_ptr<SessionI>&,
    const string&,
    const chrono::time_point<chrono::system_clock>&,
    bool)
{
    assert(false);
}

shared_ptr<DataStormContract::ElementConfig>
DataElementI::getConfig() const
{
    return _config;
}

void
DataElementI::waitForListeners(int count) const
{
    unique_lock<mutex> lock(_parent->_mutex);
    while (true)
    {
        _parent->instance()->checkShutdown();
        if (count < 0 && _listenerCount == 0)
        {
            return;
        }
        else if (count >= 0 && _listenerCount >= static_cast<size_t>(count))
        {
            return;
        }
        _parent->_cond.wait(lock);
    }
}

bool
DataElementI::hasListeners() const
{
    unique_lock<mutex> lock(_parent->_mutex);
    return _listenerCount > 0;
}

CommunicatorPtr
DataElementI::getCommunicator() const
{
    return _parent->instance()->getCommunicator();
}

bool
DataElementI::addConnectedKey(const shared_ptr<Key>& key, const shared_ptr<Subscriber>& subscriber)
{
    auto& subscribers = _connectedKeys[key];
    if (find(subscribers.begin(), subscribers.end(), subscriber) == subscribers.end())
    {
        if (key && subscribers.empty() && _onConnectedKeys)
        {
            _executor->queue([callback = _onConnectedKeys, key] { callback(DataStorm::CallbackReason::Connect, key); });
        }
        subscribers.push_back(subscriber);
        return true;
    }
    return false;
}

bool
DataElementI::removeConnectedKey(const shared_ptr<Key>& key, const shared_ptr<Subscriber>& subscriber)
{
    auto& subscribers = _connectedKeys[key];
    auto p = find(subscribers.begin(), subscribers.end(), subscriber);
    if (p != subscribers.end())
    {
        subscribers.erase(p);
        if (subscribers.empty())
        {
            if (key && _onConnectedKeys)
            {
                _executor->queue([callback = _onConnectedKeys, key]
                                 { callback(DataStorm::CallbackReason::Disconnect, key); });
            }
            _connectedKeys.erase(key);
        }
        return true;
    }
    return false;
}

void
DataElementI::notifyListenerWaiters() const
{
    _parent->_cond.notify_all();
}

void
DataElementI::disconnect()
{
    map<ListenerKey, Listener> listeners;
    {
        unique_lock<mutex> lock(_parent->_mutex);
        listeners.swap(_listeners);
        _parent->decListenerCount(_listenerCount);
        _listenerCount = 0;
        notifyListenerWaiters();
    }
    for (const auto& listener : listeners)
    {
        for (const auto& ks : listener.second.subscribers)
        {
            const auto& k = ks.first;
            if (k.second < 0)
            {
                listener.first.session->disconnectFromFilter(k.first, k.second, shared_from_this());
            }
            else
            {
                listener.first.session->disconnectFromKey(k.first, k.second, shared_from_this(), ks.second->id);
            }
        }
    }
}

void
DataElementI::forward(const ByteSeq& inParams, const Current& current) const
{
    for (const auto& [_, listener] : _listeners)
    {
        // If we are forwarding a sample check if at least once of the listeners is interested in the sample.
        if (!_sample || listener.matchOne(_sample, false))
        {
            // Forward the call using the listener's session proxy don't need to wait for the result.
            listener.proxy
                ->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, nullptr, nullptr, current.ctx);
        }
    }
}

DataReaderI::DataReaderI(
    TopicReaderI* topic,
    string name,
    int64_t id,
    string sampleFilterName,
    ByteSeq sampleFilterCriteria,
    const DataStorm::ReaderConfig& config)
    : DataElementI(topic, std::move(name), id, config),
      _parent(topic),
      _discardPolicy(config.discardPolicy ? *config.discardPolicy : DataStorm::DiscardPolicy::None)
{
    if (!sampleFilterName.empty())
    {
        _config->sampleFilter =
            FilterInfo{.name = std::move(sampleFilterName), .criteria = std::move(sampleFilterCriteria)};
    }
}

int
DataReaderI::getInstanceCount() const
{
    lock_guard<mutex> lock(_parent->_mutex);
    return _instanceCount;
}

vector<shared_ptr<Sample>>
DataReaderI::getAllUnread()
{
    lock_guard<mutex> lock(_parent->_mutex);
    vector<shared_ptr<Sample>> unread(_samples.begin(), _samples.end());
    _samples.clear();
    return unread;
}

void
DataReaderI::waitForUnread(unsigned int count) const
{
    unique_lock<mutex> lock(_parent->_mutex);
    _parent->_cond.wait(
        lock,
        [&]()
        {
            _parent->instance()->checkShutdown();
            return _samples.size() >= count;
        });
}

bool
DataReaderI::hasUnread() const
{
    unique_lock<mutex> lock(_parent->_mutex);
    return !_samples.empty();
}

shared_ptr<Sample>
DataReaderI::getNextUnread()
{
    unique_lock<mutex> lock(_parent->_mutex);
    _parent->_cond.wait(
        lock,
        [&]()
        {
            _parent->instance()->checkShutdown();
            return !_samples.empty();
        });
    shared_ptr<Sample> sample = _samples.front();
    _samples.pop_front();
    return sample;
}

void
DataReaderI::initSamples(
    const vector<shared_ptr<Sample>>& samples,
    int64_t topic,
    int64_t element,
    int priority,
    const chrono::time_point<chrono::system_clock>& now,
    bool checkKey)
{
    if (_traceLevels->data > 1)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": initialized " << samples.size() << " samples from '" << element << '@' << topic << "'";
    }

    vector<shared_ptr<Sample>> valid;
    // Resolve partial updates against the previous sample of the SAME key.
    map<shared_ptr<Key>, shared_ptr<Sample>> previousByKey = _lastByKey;
    for (const auto& sample : samples)
    {
        if (checkKey && !matchKey(sample->key))
        {
            continue;
        }

        // Apply discard policies:
        // - SendTime: discard samples older than the last received sample.
        // - Priority: discard samples from publisher with lower priority than the highest priority among the connected
        //   publishers for the same key. The subscriber list is sorted by priority in addConnectedKey.
        if ((_discardPolicy == DataStorm::DiscardPolicy::SendTime && sample->timestamp <= _lastSendTime) ||
            (_discardPolicy == DataStorm::DiscardPolicy::Priority &&
             hasLowerPriorityThanConnected(priority, sample->key)))
        {
            // The discard only affects the application-visible delivery: when the key has no base yet, still record
            // the discarded sample as the base partial updates resolve against, otherwise a later partial update on
            // the key would have no base. A remove carries no value and cannot serve as a base.
            if (sample->event != DataStorm::SampleEvent::PartialUpdate &&
                sample->event != DataStorm::SampleEvent::Remove &&
                previousByKey.find(sample->key) == previousByKey.end())
            {
                try
                {
                    if (!sample->hasValue())
                    {
                        sample->decode(_parent->instance()->getCommunicator());
                    }
                    previousByKey[sample->key] = sample;
                }
                catch (const std::exception&)
                {
                    // Ignore, the sample was discarded anyway.
                }
            }
            continue;
        }

        assert(sample->key);

        if (!sample->hasValue())
        {
            if (sample->event == DataStorm::SampleEvent::PartialUpdate)
            {
                auto p = previousByKey.find(sample->key);
                if (p == previousByKey.end() || !p->second->hasValue())
                {
                    // No usable base to resolve the partial update against (the key was never set, was removed, or
                    // the base sample carries no value): drop the sample rather than apply the update to a
                    // default-constructed value.
                    if (_traceLevels->data > 0)
                    {
                        Trace out(_traceLevels->logger, _traceLevels->dataCat);
                        out << this << ": discarded partial update sample " << sample->id
                            << ": no base value for the key";
                    }
                    continue;
                }

                try
                {
                    _parent->getUpdater(sample->tag)(p->second, sample, _parent->instance()->getCommunicator());
                }
                catch (const std::exception& ex)
                {
                    // An updater or decoder that throws (a user updater error, or a writer/reader update type
                    // mismatch) drops the sample; the base is left unchanged so the next full sample resynchronizes
                    // the key.
                    Warning out(_traceLevels->logger);
                    out << "dropped sample " << sample->id << ": the partial update could not be applied:\n"
                        << ex.what();
                    continue;
                }
            }
            else if (sample->event != DataStorm::SampleEvent::Remove)
            {
                // A remove's value is irrelevant and is left default-constructed: skipping the decoding ensures a
                // decoding failure cannot drop the remove and leave the key's stale base in place below.
                try
                {
                    sample->decode(_parent->instance()->getCommunicator());
                }
                catch (const std::exception& ex)
                {
                    Warning out(_traceLevels->logger);
                    out << "dropped sample " << sample->id << ": the value could not be decoded:\n" << ex.what();
                    continue;
                }
            }
        }
        valid.push_back(sample);

        // A remove clears the per-key base: the key has no value anymore, so a later partial update for the key
        // has no base to resolve against and is discarded.
        if (sample->event == DataStorm::SampleEvent::Remove)
        {
            previousByKey.erase(sample->key);
        }
        else
        {
            previousByKey[sample->key] = sample;
        }
    }

    if (_traceLevels->data > 2 && valid.size() < samples.size())
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": discarded " << samples.size() - valid.size() << " samples from '" << element << '@' << topic
            << "'";
    }

    if (_onSamples)
    {
        _executor->queue(
            [callback = _onSamples, valid]
            {
                for (const auto& s : valid)
                {
                    callback(s);
                }
            });
    }

    if (valid.empty())
    {
        // Even when every sample was discarded or dropped, keep the bases recorded above so a later partial update
        // on their keys can resolve.
        _lastByKey = std::move(previousByKey);
        return;
    }
    _lastSendTime = valid.back()->timestamp;

    // The per-key bases for partial updates are maintained even when the element keeps no history (sampleCount == 0).
    _lastByKey = std::move(previousByKey);

    if (_config->sampleLifetime && *_config->sampleLifetime > 0)
    {
        cleanOldSamples(_samples, now, *_config->sampleLifetime);
    }

    if (_config->sampleCount && *_config->sampleCount == 0)
    {
        return; // Don't keep history
    }

    if (_config->clearHistory && *_config->clearHistory == ClearHistoryPolicy::OnAll)
    {
        _samples.clear();
        _samples.push_back(valid.back());
    }
    else
    {
        for (const auto& s : valid)
        {
            if (_config->clearHistory &&
                ((s->event == DataStorm::SampleEvent::Add && *_config->clearHistory == ClearHistoryPolicy::OnAdd) ||
                 (s->event == DataStorm::SampleEvent::Remove &&
                  *_config->clearHistory == ClearHistoryPolicy::OnRemove) ||
                 (s->event != DataStorm::SampleEvent::PartialUpdate &&
                  *_config->clearHistory == ClearHistoryPolicy::OnAllExceptPartialUpdate)))
            {
                _samples.clear();
            }
            _samples.push_back(s);
        }
    }

    // Keep at most sampleCount samples. The init batch can exceed sampleCount when the writer delivers one base per
    // key (an any-key reader joining a writer with more distinct keys than sampleCount); those per-key bases were
    // already recorded in _lastByKey above, so trimming the surplus from the visible history here is safe.
    if (_config->sampleCount && *_config->sampleCount > 0)
    {
        while (_samples.size() > static_cast<size_t>(*_config->sampleCount))
        {
            _samples.pop_front();
        }
    }

    assert(!_samples.empty());
    _parent->_cond.notify_all();
}

void
DataReaderI::queue(
    const shared_ptr<Sample>& sample,
    int priority,
    const shared_ptr<SessionI>&,
    const string& facet,
    const chrono::time_point<chrono::system_clock>& now,
    bool checkKey)
{
    if (_config->facet && *_config->facet != facet)
    {
        if (_traceLevels->data > 2)
        {
            Trace out(_traceLevels->logger, _traceLevels->dataCat);
            out << this << ": skipped sample " << sample->id << " (facet doesn't match)";
        }
        return;
    }
    else if (checkKey && !matchKey(sample->key))
    {
        if (_traceLevels->data > 2)
        {
            Trace out(_traceLevels->logger, _traceLevels->dataCat);
            out << this << ": skipped sample " << sample->id << " (key doesn't match)";
        }
        return;
    }

    if (_traceLevels->data > 2)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": queued sample " << sample->id << " listeners=" << _listenerCount;
    }

    // Apply discard policies:
    // - SendTime: discard samples older than the last received sample.
    // - Priority: discard samples from publisher with lower priority than the highest priority among the connected
    //   publishers for the same key. The subscriber list is sorted by priority in addConnectedKey.
    if ((_discardPolicy == DataStorm::DiscardPolicy::SendTime && sample->timestamp <= _lastSendTime) ||
        (_discardPolicy == DataStorm::DiscardPolicy::Priority && hasLowerPriorityThanConnected(priority, sample->key)))
    {
        if (_traceLevels->data > 2)
        {
            Trace out(_traceLevels->logger, _traceLevels->dataCat);
            out << this << ": discarded sample" << sample->id;
        }

        // The discard only affects the application-visible delivery: when the key has no base yet, still record the
        // discarded sample as the base partial updates resolve against, otherwise a later partial update on the key
        // would have no base. A remove carries no value and cannot serve as a base.
        if (sample->event != DataStorm::SampleEvent::PartialUpdate && sample->event != DataStorm::SampleEvent::Remove &&
            _lastByKey.find(sample->key) == _lastByKey.end())
        {
            try
            {
                if (!sample->hasValue())
                {
                    sample->decode(_parent->instance()->getCommunicator());
                }
                _lastByKey[sample->key] = sample;
            }
            catch (const std::exception&)
            {
                // Ignore, the sample was discarded anyway.
            }
        }
        return;
    }

    if (!sample->hasValue())
    {
        if (sample->event == DataStorm::SampleEvent::PartialUpdate)
        {
            auto p = _lastByKey.find(sample->key);
            if (p == _lastByKey.end() || !p->second->hasValue())
            {
                // No usable base to resolve the partial update against (the key was never set, was removed, or the
                // base sample carries no value): drop the sample rather than apply the update to a
                // default-constructed value.
                if (_traceLevels->data > 0)
                {
                    Trace out(_traceLevels->logger, _traceLevels->dataCat);
                    out << this << ": discarded partial update sample " << sample->id << ": no base value for the key";
                }
                return;
            }

            try
            {
                _parent->getUpdater(sample->tag)(p->second, sample, _parent->instance()->getCommunicator());
            }
            catch (const std::exception& ex)
            {
                // An updater or decoder that throws (a user updater error, or a writer/reader update type mismatch)
                // drops the sample; the base is left unchanged so the next full sample resynchronizes the key.
                Warning out(_traceLevels->logger);
                out << "dropped sample " << sample->id << ": the partial update could not be applied:\n" << ex.what();
                return;
            }
        }
        else if (sample->event != DataStorm::SampleEvent::Remove)
        {
            // A remove's value is irrelevant and is left default-constructed: skipping the decoding ensures a
            // decoding failure cannot drop the remove and leave the key's stale base in place below.
            try
            {
                sample->decode(_parent->instance()->getCommunicator());
            }
            catch (const std::exception& ex)
            {
                Warning out(_traceLevels->logger);
                out << "dropped sample " << sample->id << ": the value could not be decoded:\n" << ex.what();
                return;
            }
        }
    }
    _lastSendTime = sample->timestamp;

    // The per-key base for partial updates is maintained even when the element keeps no history (sampleCount == 0).
    // A remove clears the base: the key has no value anymore, so a later partial update for the key has no base to
    // resolve against and is discarded.
    if (sample->event == DataStorm::SampleEvent::Remove)
    {
        _lastByKey.erase(sample->key);
    }
    else
    {
        _lastByKey[sample->key] = sample;
    }

    if (_onSamples)
    {
        _executor->queue([callback = _onSamples, sample] { callback(sample); });
    }

    if (_config->sampleLifetime && *_config->sampleLifetime > 0)
    {
        cleanOldSamples(_samples, now, *_config->sampleLifetime);
    }

    if (_config->sampleCount)
    {
        if (*_config->sampleCount > 0)
        {
            size_t count = _samples.size();
            auto maxCount = static_cast<size_t>(*_config->sampleCount);
            if (count + 1 > maxCount)
            {
                if (!_samples.empty())
                {
                    _samples.pop_front();
                }
                assert(_samples.size() + 1 == maxCount);
            }
        }
        else if (*_config->sampleCount == 0)
        {
            return; // Don't keep history
        }
    }

    if (_config->clearHistory &&
        (*_config->clearHistory == ClearHistoryPolicy::OnAll ||
         (sample->event == DataStorm::SampleEvent::Add && *_config->clearHistory == ClearHistoryPolicy::OnAdd) ||
         (sample->event == DataStorm::SampleEvent::Remove && *_config->clearHistory == ClearHistoryPolicy::OnRemove) ||
         (sample->event != DataStorm::SampleEvent::PartialUpdate &&
          *_config->clearHistory == ClearHistoryPolicy::OnAllExceptPartialUpdate)))
    {
        _samples.clear();
    }
    _samples.push_back(sample);
    _parent->_cond.notify_all();
}

void
DataReaderI::onSamples(
    function<void(const vector<shared_ptr<Sample>>&)> init,
    function<void(const shared_ptr<Sample>&)> update)
{
    unique_lock<mutex> lock(_parent->_mutex);
    _onSamples = std::move(update);
    if (init && !_samples.empty())
    {
        vector<shared_ptr<Sample>> samples(_samples.begin(), _samples.end());
        _executor->queue([init, samples] { init(samples); }, true);
    }
}

bool
DataReaderI::addConnectedKey(const shared_ptr<Key>& key, const shared_ptr<Subscriber>& subscriber)
{
    if (DataElementI::addConnectedKey(key, subscriber))
    {
        if (_discardPolicy == DataStorm::DiscardPolicy::Priority)
        {
            auto& subscribers = _connectedKeys[key];
            sort(
                subscribers.begin(),
                subscribers.end(),
                [](const shared_ptr<Subscriber>& lhs, const shared_ptr<Subscriber>& rhs)
                { return lhs->priority < rhs->priority; });
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool
DataReaderI::hasLowerPriorityThanConnected(int priority, const shared_ptr<Key>& key) const
{
    // The connected publishers that can deliver this key are registered under the key itself (a keyed peer) and under
    // the null key (a filter or any-key peer, which delivers every key). The threshold is the highest priority across
    // both; each list is sorted ascending in addConnectedKey, so back() is the highest. With no connected publishers
    // the threshold stays at the sample's own priority, so the function returns false.
    int threshold = priority;
    for (const auto& k : {key, shared_ptr<Key>{}})
    {
        auto p = _connectedKeys.find(k);
        if (p != _connectedKeys.end() && !p->second.empty())
        {
            threshold = std::max(threshold, p->second.back()->priority);
        }
    }
    return priority < threshold;
}

DataWriterI::DataWriterI(TopicWriterI* topic, string name, int64_t id, const DataStorm::WriterConfig& config)
    : DataElementI(topic, std::move(name), id, config),
      _parent(topic),
      _subscribers{uncheckedCast<DataStormContract::SubscriberSessionPrx>(_forwarder)}
{
    _config->priority = config.priority;
}

void
DataWriterI::publish(const shared_ptr<Key>& key, const shared_ptr<Sample>& sample)
{
    lock_guard<mutex> lock(_parent->_mutex);
    if (sample->event == DataStorm::SampleEvent::PartialUpdate)
    {
        assert(!sample->hasValue());
        auto p = _lastByKey.find(key);
        if (p == _lastByKey.end())
        {
            // No base to resolve the partial update against (the key was never set, or was removed): discard the
            // update rather than apply it to a default-constructed value. The reader side discards such updates
            // the same way.
            Warning out(_traceLevels->logger);
            out << this << ": discarded partial update: no base value for the key";
            return;
        }
        _parent->getUpdater(sample->tag)(p->second, sample, _parent->instance()->getCommunicator());
    }

    sample->id = ++_parent->_nextSampleId;
    sample->timestamp = chrono::system_clock::now();

    if (_traceLevels->data > 2)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": publishing sample " << sample->id << " listeners=" << _listenerCount;
    }
    send(key, sample);

    // The per-key base for partial updates is maintained even when the element keeps no history (sampleCount == 0).
    // A remove clears the base: the key has no value anymore, so a later partial update for the key has no base to
    // resolve against and is discarded.
    if (sample->event == DataStorm::SampleEvent::Remove)
    {
        _lastByKey.erase(key);
    }
    else
    {
        _lastByKey[key] = sample;
    }

    if (_config->sampleLifetime && *_config->sampleLifetime > 0)
    {
        cleanOldSamples(_samples, sample->timestamp, *_config->sampleLifetime);
    }

    if (_config->sampleCount)
    {
        if (*_config->sampleCount > 0)
        {
            if (_samples.size() + 1 > static_cast<size_t>(*_config->sampleCount))
            {
                _samples.pop_front();
            }
        }
        else if (*_config->sampleCount == 0)
        {
            return; // Don't keep history
        }
    }

    if (_config->clearHistory &&
        (*_config->clearHistory == ClearHistoryPolicy::OnAll ||
         (sample->event == DataStorm::SampleEvent::Add && *_config->clearHistory == ClearHistoryPolicy::OnAdd) ||
         (sample->event == DataStorm::SampleEvent::Remove && *_config->clearHistory == ClearHistoryPolicy::OnRemove) ||
         (sample->event != DataStorm::SampleEvent::PartialUpdate &&
          *_config->clearHistory == ClearHistoryPolicy::OnAllExceptPartialUpdate)))
    {
        _samples.clear();
    }
    assert(sample->key);
    _samples.push_back(sample);
}

KeyDataReaderI::KeyDataReaderI(
    TopicReaderI* topic,
    string name,
    int64_t id,
    const vector<shared_ptr<Key>>& keys,
    string sampleFilterName,
    ByteSeq sampleFilterCriteria,
    const DataStorm::ReaderConfig& config)
    : DataReaderI(topic, std::move(name), id, std::move(sampleFilterName), std::move(sampleFilterCriteria), config),
      _keys(keys)
{
    if (_traceLevels->data > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": created key reader";
    }

    // If sample filtering is enabled, ensure the updates are received using a session facet specific to this reader.
    if (_config->sampleFilter)
    {
        ostringstream os;
        os << "fa" << _id;
        _config->facet = os.str();
    }
}

void
KeyDataReaderI::destroyImpl()
{
    if (_traceLevels->data > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": destroyed key reader";
    }
    try
    {
        _forwarder->detachElements(_parent->getId(), {_keys.empty() ? -_id : _id});
    }
    catch (const std::exception&)
    {
        _parent->forwarderException();
    }
    _parent->remove(shared_from_this(), _keys);
}

void
KeyDataReaderI::waitForWriters(int count)
{
    waitForListeners(count);
}

bool
KeyDataReaderI::hasWriters()
{
    return hasListeners();
}

string
KeyDataReaderI::toString() const
{
    ostringstream os;
    os << 'e' << _id << ":";
    if (_config->name)
    {
        os << *_config->name << ":";
    }
    os << "[";
    for (auto q = _keys.begin(); q != _keys.end(); ++q)
    {
        if (q != _keys.begin())
        {
            os << ",";
        }
        os << (*q)->toString();
    }
    os << "]@" << _parent;
    return os.str();
}

bool
KeyDataReaderI::matchKey(const shared_ptr<Key>& key) const
{
    return _keys.empty() || find(_keys.begin(), _keys.end(), key) != _keys.end();
}

KeyDataWriterI::KeyDataWriterI(
    TopicWriterI* topic,
    string name,
    int64_t id,
    const vector<shared_ptr<Key>>& keys,
    const DataStorm::WriterConfig& config)
    : DataWriterI(topic, std::move(name), id, config),
      _keys(keys)
{
    if (_traceLevels->data > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": created key writer";
    }
}

void
KeyDataWriterI::destroyImpl()
{
    if (_traceLevels->data > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": destroyed key writer";
    }
    try
    {
        _forwarder->detachElements(_parent->getId(), {_keys.empty() ? -_id : _id});
    }
    catch (const std::exception&)
    {
        _parent->forwarderException();
    }
    _parent->remove(shared_from_this(), _keys);
}

void
KeyDataWriterI::waitForReaders(int count) const
{
    waitForListeners(count);
}

bool
KeyDataWriterI::hasReaders() const
{
    return hasListeners();
}

shared_ptr<Sample>
KeyDataWriterI::getLast() const
{
    unique_lock<mutex> lock(_parent->_mutex);
    return _samples.empty() ? nullptr : _samples.back();
}

vector<shared_ptr<Sample>>
KeyDataWriterI::getAll() const
{
    unique_lock<mutex> lock(_parent->_mutex);
    vector<shared_ptr<Sample>> all(_samples.begin(), _samples.end());
    return all;
}

string
KeyDataWriterI::toString() const
{
    ostringstream os;
    os << 'e' << _id << ":";
    if (_config->name)
    {
        os << *_config->name << ":";
    }
    os << "[";
    for (auto q = _keys.begin(); q != _keys.end(); ++q)
    {
        if (q != _keys.begin())
        {
            os << ",";
        }
        os << (*q)->toString();
    }
    os << "]@" << _parent;
    return os.str();
}

DataSamples
KeyDataWriterI::getSamples(
    const shared_ptr<Key>& key,
    const shared_ptr<Filter>& sampleFilter,
    const shared_ptr<DataStormContract::ElementConfig>& config,
    int64_t lastId,
    const chrono::time_point<chrono::system_clock>& now)
{
    // Collect all queued samples that match the key and sample filter, are newer than the lastId, and are not stale.
    DataSamples samples;
    samples.id = _keys.empty() ? -_id : _id;

    // Orders the source samples to deliver by id, caps them to the reader's history depth, and delivers them. A
    // partial base is sent as a full Update, and the earliest delivered sample of each key is likewise resolved to a
    // full value so the reader always has a base to merge later partial updates for that key onto.
    auto finalize = [&](vector<shared_ptr<Sample>> sources)
    {
        sort(
            sources.begin(),
            sources.end(),
            [](const shared_ptr<Sample>& lhs, const shared_ptr<Sample>& rhs) { return lhs->id < rhs->id; });

        // Cap the batch to the reader's sampleCount, but keep the newest sample of every key even when the number of
        // distinct keys exceeds sampleCount: each key the reader may later receive a partial update for needs a
        // resolvable base. The reader records these bases in _lastByKey before capping its own visible history, so
        // delivering one per key never overflows it. sampleCount == 0 keeps no history and is handled by the caller
        // (the reader still records the bases). First reserve the newest sample of each key, then fill any remaining
        // budget with the newest of the other samples.
        if (config->sampleCount && *config->sampleCount > 0 &&
            sources.size() > static_cast<size_t>(*config->sampleCount))
        {
            auto maxCount = static_cast<size_t>(*config->sampleCount);
            vector<bool> keep(sources.size(), false);
            size_t kept = 0;
            set<shared_ptr<Key>> keptKeys;
            // First pass (newest to oldest): reserve the newest sample of every key, even past maxCount.
            for (size_t i = sources.size(); i-- > 0;)
            {
                if (keptKeys.insert(sources[i]->key).second)
                {
                    keep[i] = true;
                    ++kept;
                }
            }
            // Second pass (newest to oldest): fill the remaining budget with the newest not-yet-kept samples.
            for (size_t i = sources.size(); i-- > 0 && kept < maxCount;)
            {
                if (!keep[i])
                {
                    keep[i] = true;
                    ++kept;
                }
            }
            vector<shared_ptr<Sample>> capped;
            capped.reserve(kept);
            for (size_t i = 0; i < sources.size(); ++i)
            {
                if (keep[i])
                {
                    capped.push_back(sources[i]);
                }
            }
            sources = std::move(capped);
        }

        set<shared_ptr<Key>> seen;
        for (const auto& sample : sources)
        {
            DataSample ds = toSample(sample, getCommunicator(), _keys.empty());
            // The earliest delivered sample of each key must carry a full value; a partial is sent as a full Update
            // built from the sample's own resolved value.
            if (seen.insert(sample->key).second && sample->event == DataStorm::SampleEvent::PartialUpdate)
            {
                ds.tag = 0;
                ds.event = DataStorm::SampleEvent::Update;
                ds.value = sample->encodeValue(getCommunicator());
            }
            samples.samples.push_back(std::move(ds));
        }
    };

    // Collects the current per-key base for every key that passes the caller's key/sample filter and is newer than
    // lastId; `covered` excludes keys already represented by the writer's history. A removed key has no value and is
    // skipped. A single-key writer stores its base under a null publish key, so the key is matched on the sample.
    auto collectBases = [&](const set<shared_ptr<Key>>& covered)
    {
        vector<shared_ptr<Sample>> bases;
        for (const auto& [baseKey, baseSample] : _lastByKey)
        {
            if (baseSample->id <= lastId || !baseSample->hasValue() || (key && key != baseSample->key) ||
                (sampleFilter && !sampleFilter->match(baseSample)) || covered.count(baseSample->key))
            {
                continue;
            }
            bases.push_back(baseSample);
        }
        return bases;
    };

    // A reader that keeps no history still needs the current per-key base to resolve partial updates. Send just the
    // base, not the history.
    if (config->sampleCount && *config->sampleCount == 0)
    {
        finalize(collectBases({}));
        return samples;
    }

    // Reap stale samples before collecting any samples.
    if (_config->sampleLifetime && *_config->sampleLifetime > 0)
    {
        cleanOldSamples(_samples, now, *_config->sampleLifetime);
    }

    // Compute the stale time, according to the callers sample lifetime configuration.
    chrono::time_point<chrono::system_clock> staleTime = chrono::time_point<chrono::system_clock>::min();
    if (config->sampleLifetime && *config->sampleLifetime > 0)
    {
        staleTime = now - chrono::milliseconds(*config->sampleLifetime);
    }

    // Iterate through samples in reverse chronological order, starting with the newest.
    // Stop iterating if any of the following conditions are met:
    // - A sample's timestamp is older than the specified stale time.
    // - A sample's ID is less than or equal to the specified last ID.
    // - The requested number of samples has been collected.
    // - A sample event triggers history clearing based on the caller's clear history policy.
    // For each matching sample, add it to the source set and record the key it covers.
    vector<shared_ptr<Sample>> sources;
    set<shared_ptr<Key>> covered;
    for (auto p = _samples.rbegin(); p != _samples.rend(); ++p)
    {
        if ((*p)->timestamp < staleTime)
        {
            break;
        }
        if ((*p)->id <= lastId)
        {
            break;
        }

        if ((!key || key == (*p)->key) && (!sampleFilter || sampleFilter->match(*p)))
        {
            sources.push_back(*p);
            covered.insert((*p)->key);
            if (config->sampleCount && *config->sampleCount > 0 &&
                static_cast<size_t>(*config->sampleCount) == sources.size())
            {
                break;
            }

            if (config->clearHistory &&
                (*config->clearHistory == ClearHistoryPolicy::OnAll ||
                 ((*p)->event == DataStorm::SampleEvent::Add && *config->clearHistory == ClearHistoryPolicy::OnAdd) ||
                 ((*p)->event == DataStorm::SampleEvent::Remove &&
                  *config->clearHistory == ClearHistoryPolicy::OnRemove) ||
                 ((*p)->event != DataStorm::SampleEvent::PartialUpdate &&
                  *config->clearHistory == ClearHistoryPolicy::OnAllExceptPartialUpdate)))
            {
                break;
            }
        }
    }

    // Bootstrap the base for every key the history does not already cover (trimmed by ClearHistory, aged out, or a
    // no-history writer) so the reader can resolve later partial updates for those keys too.
    auto bases = collectBases(covered);
    sources.insert(sources.end(), bases.begin(), bases.end());
    finalize(std::move(sources));
    return samples;
}

void
KeyDataWriterI::send(const shared_ptr<Key>& key, const shared_ptr<Sample>& sample) const
{
    assert(key || _keys.size() == 1);
    _sample = sample;
    _sample->key = key ? key : _keys[0];
    _subscribers->s(_parent->getId(), _keys.empty() ? -_id : _id, toSample(sample, getCommunicator(), _keys.empty()));
    _sample = nullptr;
}

void
KeyDataWriterI::forward(const ByteSeq& inParams, const Current& current) const
{
    for (const auto& [_, listener] : _listeners)
    {
        // Forward the sample if the listener has at least one subscriber interested in the update. The key is
        // always matched: a multi-key writer's sessions don't necessarily subscribe to every key of the writer,
        // and an unmatched key's sample would be wasted bandwidth at best (the receiver never subscribed its id).
        if (!_sample || listener.matchOne(_sample, true))
        {
            // Forward the call using the listener's session proxy, don't need to wait for the result.
            listener.proxy
                ->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, nullptr, nullptr, current.ctx);
        }
    }
}

FilteredDataReaderI::FilteredDataReaderI(
    TopicReaderI* topic,
    string name,
    int64_t id,
    shared_ptr<Filter> filter,
    string sampleFilterName,
    ByteSeq sampleFilterCriteria,
    const DataStorm::ReaderConfig& config)
    : DataReaderI(topic, std::move(name), id, std::move(sampleFilterName), std::move(sampleFilterCriteria), config),
      _filter(std::move(filter))
{
    if (_traceLevels->data > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": created filtered reader";
    }

    // If sample filtering is enabled, ensure the updates are received using a session facet specific to this reader.
    if (_config->sampleFilter)
    {
        ostringstream os;
        os << "fa" << _id;
        _config->facet = os.str();
    }
}

void
FilteredDataReaderI::destroyImpl()
{
    if (_traceLevels->data > 0)
    {
        Trace out(_traceLevels->logger, _traceLevels->dataCat);
        out << this << ": destroyed filter reader";
    }
    try
    {
        _forwarder->detachElements(_parent->getId(), {-_id});
    }
    catch (const std::exception&)
    {
        _parent->forwarderException();
    }
    _parent->removeFiltered(shared_from_this(), _filter);
}

void
FilteredDataReaderI::waitForWriters(int count)
{
    waitForListeners(count);
}

bool
FilteredDataReaderI::hasWriters()
{
    return hasListeners();
}

string
FilteredDataReaderI::toString() const
{
    ostringstream os;
    os << 'e' << _id << ':';
    if (_config->name)
    {
        os << *_config->name << ":";
    }
    os << "[" << _filter->toString() << "]@" << _parent;
    return os.str();
}

bool
FilteredDataReaderI::matchKey(const shared_ptr<Key>& key) const
{
    return _filter->match(key);
}
