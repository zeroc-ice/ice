// Copyright (c) ZeroC, Inc.

#include "TopicI.h"
#include "NodeI.h"
#include "SessionI.h"
#include "TopicFactoryI.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;
using namespace Ice;

namespace
{
    static Topic::Updater noOpUpdater = // NOLINT(cert-err58-cpp)
        [](const shared_ptr<Sample>& previous, const shared_ptr<Sample>& next, const CommunicatorPtr&)
    { next->setValue(previous); };

    // The always match filter always matches the value, it's used by the any key reader/writer.
    class AlwaysMatchFilter final : public Filter
    {
    public:
        [[nodiscard]] string toString() const final { return "f1:alwaysmatch"; }

        [[nodiscard]] const string& getName() const final
        {
            static string alwaysmatch("alwaysmatch");
            return alwaysmatch;
        }

        [[nodiscard]] ByteSeq encode(const CommunicatorPtr&) const final { return {}; }

        [[nodiscard]] int64_t getId() const final
        {
            return 1; // 1 is reserved for the match all filter.
        }

        [[nodiscard]] bool match(const shared_ptr<Filterable>&) const final { return true; }
    };
    const auto alwaysMatchFilter = make_shared<AlwaysMatchFilter>(); // NOLINT(cert-err58-cpp)

    DataStorm::ClearHistoryPolicy parseClearHistory(const std::string& value)
    {
        if (value == "OnAdd")
        {
            return DataStorm::ClearHistoryPolicy::OnAdd;
        }
        else if (value == "OnRemove")
        {
            return DataStorm::ClearHistoryPolicy::OnRemove;
        }
        else if (value == "OnAll")
        {
            return DataStorm::ClearHistoryPolicy::OnAll;
        }
        else if (value == "OnAllExceptPartialUpdate")
        {
            return DataStorm::ClearHistoryPolicy::OnAllExceptPartialUpdate;
        }
        else if (value == "Never")
        {
            return DataStorm::ClearHistoryPolicy::Never;
        }
        else
        {
            throw ParseException(__FILE__, __LINE__, "Invalid clear history policy: " + value);
        }
    }

    DataStorm::DiscardPolicy parseDiscardPolicy(const std::string& value)
    {
        if (value == "Never")
        {
            return DataStorm::DiscardPolicy::None;
        }
        else if (value == "SendTime")
        {
            return DataStorm::DiscardPolicy::SendTime;
        }
        else if (value == "Priority")
        {
            return DataStorm::DiscardPolicy::Priority;
        }
        else
        {
            throw ParseException(__FILE__, __LINE__, "Invalid discard policy: " + value);
        }
    }
}

TopicI::TopicI(
    shared_ptr<Instance> instance,
    const shared_ptr<TopicFactoryI>& factory,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories,
    string name,
    int64_t id)
    : _factory(factory),
      _keyFactory(std::move(keyFactory)),
      _tagFactory(std::move(tagFactory)),
      _sampleFactory(std::move(sampleFactory)),
      _keyFilterFactories(std::move(keyFilterFactories)),
      _sampleFilterFactories(std::move(sampleFilterFactories)),
      _name(std::move(name)),
      _instance(std::move(instance)),
      _traceLevels(_instance->getTraceLevels()),
      _id(id),
      // The collocated forwarder is initalized here to avoid using a nullable proxy. The forwarder is only used by
      // the instance that owns it and is removed in destroy implementation.
      _forwarder{_instance->getCollocatedForwarder()->add<SessionPrx>(
          [this](const ByteSeq& inParams, const Current& current) { forward(inParams, current); })}
{
}

TopicI::~TopicI() { assert(_destroyed); }

string
TopicI::getName() const
{
    return _name;
}

void
TopicI::destroy()
{
    std::map<std::shared_ptr<Key>, std::set<std::shared_ptr<DataElementI>>> keyElements;
    std::map<std::shared_ptr<Filter>, std::set<std::shared_ptr<DataElementI>>> filteredElements;
    {
        lock_guard<mutex> lock(_mutex);
        assert(!_destroyed);
        _destroyed = true;
        try
        {
            _forwarder->detachTopic(_id); // Must be called before disconnect()
        }
        catch (const std::exception&)
        {
            forwarderException();
        }
        _keyElements.swap(keyElements);
        _filteredElements.swap(filteredElements);
        _instance->getCollocatedForwarder()->remove(_forwarder->ice_getIdentity());
    }
    disconnect();
}

void
TopicI::shutdown()
{
    lock_guard<mutex> lock(_mutex);
    _cond.notify_all();
}

TopicSpec
TopicI::getTopicSpec() const
{
    TopicSpec spec;
    spec.id = _id;
    spec.name = _name;
    spec.elements.reserve(_keyElements.size() + _filteredElements.size());

    // Add a key element to the spec for each topic key. Key elements have positive IDs.
    for (const auto& [key, _] : _keyElements)
    {
        spec.elements.push_back(
            ElementInfo{.id = key->getId(), .name = "", .value = key->encode(_instance->getCommunicator())});
    }

    // Add a filtered element to the spec for each topic filter. Filtered elements have negative IDs.
    for (const auto& [filter, _] : _filteredElements)
    {
        spec.elements.push_back(ElementInfo{
            .id = -filter->getId(),
            .name = filter->getName(),
            .value = filter->encode(_instance->getCommunicator())});
    }

    spec.tags = getTags();
    return spec;
}

ElementInfoSeq
TopicI::getTags() const
{
    ElementInfoSeq tags;
    tags.reserve(_updaters.size());
    for (const auto& [tag, _] : _updaters)
    {
        tags.push_back(ElementInfo{.id = tag->getId(), .name = "", .value = tag->encode(_instance->getCommunicator())});
    }
    return tags;
}

ElementSpecSeq
TopicI::getElementSpecs(int64_t topicId, const ElementInfoSeq& infos, const shared_ptr<SessionI>& session)
{
    ElementSpecSeq specs;
    // Iterate over the element infos representing the remote keys, and filters and compute the element spec for local
    // keys and filters that match. Positive IDs represent keys and negative IDs represent filters.
    for (const auto& info : infos)
    {
        if (info.id > 0)
        {
            auto key = _keyFactory->decode(_instance->getCommunicator(), info.value);
            auto p = _keyElements.find(key);
            if (p != _keyElements.end())
            {
                // If we have a matching key add it to the spec, with all the key data readers or writers.
                ElementDataSeq elements;
                for (const auto& dataElement : p->second)
                {
                    elements.push_back(ElementData{
                        .id = dataElement->getId(),
                        .config = dataElement->getConfig(),
                        .lastIds = session->getLastIds(topicId, info.id, dataElement)});
                }
                specs.push_back(ElementSpec{
                    .elements = std::move(elements),
                    .id = key->getId(),
                    .name = "",
                    .value = {},
                    .peerId = info.id,
                    .peerName = ""});
            }

            // Add filtered elements matching the key.
            for (const auto& [filter, filteredDataElements] : _filteredElements)
            {
                if (filter->match(key))
                {
                    ElementDataSeq elements;
                    for (const auto& dataElement : filteredDataElements)
                    {
                        elements.push_back(ElementData{
                            .id = dataElement->getId(),
                            .config = dataElement->getConfig(),
                            .lastIds = session->getLastIds(topicId, info.id, dataElement)});
                    }

                    specs.push_back(ElementSpec{
                        .elements = std::move(elements),
                        .id = -filter->getId(),
                        .name = filter->getName(),
                        .value = filter->encode(_instance->getCommunicator()),
                        .peerId = info.id,
                        .peerName = ""});
                }
            }
        }
        else
        {
            // An empty filter value represents a match all filter. Otherwise we decode the filter using the key filter
            // factory.
            shared_ptr<Filter> peerFilter;
            if (info.value.empty())
            {
                peerFilter = alwaysMatchFilter;
            }
            else
            {
                peerFilter = _keyFilterFactories->decode(_instance->getCommunicator(), info.name, info.value);
            }

            // Add key elements matching the filter.
            for (const auto& [key, keyDataElements] : _keyElements)
            {
                if (peerFilter->match(key))
                {
                    ElementDataSeq elements;
                    for (const auto& dataElement : keyDataElements)
                    {
                        elements.push_back(ElementData{
                            .id = dataElement->getId(),
                            .config = dataElement->getConfig(),
                            .lastIds = session->getLastIds(topicId, info.id, dataElement)});
                    }
                    specs.push_back(ElementSpec{
                        .elements = std::move(elements),
                        .id = key->getId(),
                        .name = "",
                        .value = key->encode(_instance->getCommunicator()),
                        .peerId = info.id,
                        .peerName = info.name});
                }
            }

            if (peerFilter == alwaysMatchFilter)
            {
                for (const auto& [filter, filteredDataElements] : _filteredElements)
                {
                    ElementDataSeq elements;
                    for (const auto& dataElement : filteredDataElements)
                    {
                        elements.push_back(ElementData{
                            .id = dataElement->getId(),
                            .config = dataElement->getConfig(),
                            .lastIds = session->getLastIds(topicId, info.id, dataElement)});
                    }
                    specs.push_back(ElementSpec{
                        .elements = std::move(elements),
                        .id = -filter->getId(),
                        .name = filter->getName(),
                        .value = filter->encode(_instance->getCommunicator()),
                        .peerId = info.id,
                        .peerName = info.name});
                }
            }
            else
            {
                auto p = _filteredElements.find(alwaysMatchFilter);
                if (p != _filteredElements.end())
                {
                    ElementDataSeq elements;
                    for (const auto& dataElement : p->second)
                    {
                        elements.push_back(ElementData{
                            .id = dataElement->getId(),
                            .config = dataElement->getConfig(),
                            .lastIds = session->getLastIds(topicId, info.id, dataElement)});
                    }
                    specs.push_back(ElementSpec{
                        .elements = std::move(elements),
                        .id = -alwaysMatchFilter->getId(),
                        .name = alwaysMatchFilter->getName(),
                        .value = alwaysMatchFilter->encode(_instance->getCommunicator()),
                        .peerId = info.id,
                        .peerName = info.name});
                }
            }
        }
    }
    return specs;
}

void
TopicI::attach(int64_t topicId, shared_ptr<SessionI> session, SessionPrx peerSession)
{
    auto p = _listeners.find(session);
    if (p == _listeners.end())
    {
        p = _listeners.emplace(std::move(session), Listener(std::move(peerSession))).first;
    }

    // Add the topic ID to the list of remote topics for the listener if it is not already present.
    // If the topic ID is successfully added, instruct the session to subscribe to the topic.
    if (p->second.topics.insert(topicId).second)
    {
        p->first->subscribe(topicId, this);
    }
}

void
TopicI::detach(int64_t topicId, const shared_ptr<SessionI>& session)
{
    auto p = _listeners.find(session);
    if (p != _listeners.end() && p->second.topics.erase(topicId))
    {
        // If the topic ID is removed, instruct the session to unsubscribe from the topic.
        session->unsubscribe(topicId, this);

        // If the session has no remaining subscribed topics, remove its listener from the list.
        if (p->second.topics.empty())
        {
            _listeners.erase(p);
        }
    }
}

ElementSpecAckSeq
TopicI::attachElements(
    int64_t topicId,
    const ElementSpecSeq& elements,
    const shared_ptr<SessionI>& session,
    const SessionPrx& prx,
    const chrono::time_point<chrono::system_clock>& now)
{
    // Called by the session holding the session and topic locks.

    ElementSpecAckSeq specs;
    for (const auto& spec : elements)
    {
        // The peer ID is computed by the remote caller and represents our local ID.
        // Positive IDs represent keys and negative IDs represent filters.
        if (spec.peerId > 0)
        {
            auto key = _keyFactory->get(spec.peerId);
            auto p = _keyElements.find(key);
            if (p != _keyElements.end())
            {
                shared_ptr<Filter> filter;
                if (spec.id < 0) // Filter
                {
                    if (spec.value.empty())
                    {
                        filter = alwaysMatchFilter;
                    }
                    else
                    {
                        filter = _keyFilterFactories->decode(_instance->getCommunicator(), spec.name, spec.value);
                    }
                }

                // Iterate over the data elements for the matching key, attaching them to the data elements of the spec.
                if (spec.id > 0 || filter->match(key))
                {
                    for (const auto& dataElement : p->second)
                    {
                        ElementDataAckSeq acks;
                        for (const auto& data : spec.elements)
                        {
                            dataElement->attach(topicId, spec.id, key, filter, session, prx, data, now, acks);
                        }

                        if (!acks.empty())
                        {
                            specs.push_back(ElementSpecAck{
                                .elements = std::move(acks),
                                .id = key->getId(),
                                .name = "",
                                .value = spec.id < 0 ? key->encode(_instance->getCommunicator()) : ByteSeq{},
                                .peerId = spec.id,
                                .peerName = spec.name});
                        }
                    }
                }
            }
        }
        else
        {
            shared_ptr<Filter> filter;
            if (spec.peerId == -1)
            {
                filter = alwaysMatchFilter;
            }
            else
            {
                filter = _keyFilterFactories->get(spec.peerName, -spec.peerId);
            }

            auto p = _filteredElements.find(filter);
            if (p != _filteredElements.end())
            {
                shared_ptr<Key> key;
                if (spec.id > 0) // Key
                {
                    key = _keyFactory->decode(_instance->getCommunicator(), spec.value);
                }

                if (spec.id < 0 || filter->match(key))
                {
                    for (const auto& dataElement : p->second)
                    {
                        ElementDataAckSeq acks;
                        for (const auto& data : spec.elements)
                        {
                            dataElement->attach(topicId, spec.id, key, filter, session, prx, data, now, acks);
                        }

                        if (!acks.empty())
                        {
                            specs.push_back(ElementSpecAck{
                                .elements = std::move(acks),
                                .id = -filter->getId(),
                                .name = filter->getName(),
                                .value = spec.id > 0 ? filter->encode(_instance->getCommunicator()) : ByteSeq{},
                                .peerId = spec.id,
                                .peerName = spec.name});
                        }
                    }
                }
            }
        }
    }
    return specs;
}

DataSamplesSeq
TopicI::attachElementsAck(
    int64_t topicId,
    const ElementSpecAckSeq& elements,
    const shared_ptr<SessionI>& session,
    const SessionPrx& prx,
    const chrono::time_point<chrono::system_clock>& now,
    LongSeq& removedIds)
{
    DataSamplesSeq samples;
    vector<function<void()>> initCallbacks;
    for (const auto& spec : elements)
    {
        if (spec.peerId > 0) // Key
        {
            auto key = _keyFactory->get(spec.peerId);
            auto p = _keyElements.find(key);
            if (p != _keyElements.end())
            {
                shared_ptr<Filter> filter;
                if (spec.id < 0)
                {
                    if (spec.value.empty())
                    {
                        filter = alwaysMatchFilter;
                    }
                    else
                    {
                        filter = _keyFilterFactories->decode(_instance->getCommunicator(), spec.name, spec.value);
                    }
                }

                for (const auto& data : spec.elements)
                {
                    bool found = false;
                    for (const auto& dataElement : p->second)
                    {
                        if (data.peerId == dataElement->getId())
                        {
                            function<void()> initCb;
                            if (spec.id > 0) // Key
                            {
                                initCb = dataElement
                                             ->attach(topicId, spec.id, key, nullptr, session, prx, data, now, samples);
                            }
                            else if (filter->match(key)) // Filter
                            {
                                initCb = dataElement
                                             ->attach(topicId, spec.id, key, filter, session, prx, data, now, samples);
                            }

                            if (initCb)
                            {
                                initCallbacks.push_back(initCb);
                            }
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        removedIds.push_back(data.peerId);
                    }
                }
            }
            else
            {
                for (const auto& data : spec.elements)
                {
                    removedIds.push_back(data.peerId);
                }
            }
        }
        else // Filter
        {
            shared_ptr<Filter> filter;
            if (spec.peerId == -1)
            {
                filter = alwaysMatchFilter;
            }
            else
            {
                filter = _keyFilterFactories->get(spec.peerName, -spec.peerId);
            }

            auto p = _filteredElements.find(filter);
            if (p != _filteredElements.end())
            {
                shared_ptr<Key> key;
                if (spec.id > 0) // Key
                {
                    key = _keyFactory->decode(_instance->getCommunicator(), spec.value);
                }

                for (const auto& data : spec.elements)
                {
                    bool found = false;
                    for (const auto& dataElement : p->second)
                    {
                        if (data.peerId == dataElement->getId())
                        {
                            function<void()> initCb;
                            if (spec.id < 0) // Filter
                            {
                                initCb =
                                    dataElement
                                        ->attach(topicId, spec.id, nullptr, filter, session, prx, data, now, samples);
                            }
                            else if (filter->match(key))
                            {
                                initCb = dataElement
                                             ->attach(topicId, spec.id, key, nullptr, session, prx, data, now, samples);
                            }

                            if (initCb)
                            {
                                initCallbacks.push_back(initCb);
                            }
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        removedIds.push_back(-data.peerId);
                    }
                }
            }
            else
            {
                for (const auto& data : spec.elements)
                {
                    removedIds.push_back(-data.peerId);
                }
            }
        }
    }

    // Initialize samples on data elements once all the elements have been attached. This is important for the priority
    // configuration in case 2 writers with different priorities are attached from the same session.
    for (const auto& initCb : initCallbacks)
    {
        initCb();
    }
    return samples;
}

void
TopicI::setUpdater(const shared_ptr<Tag>& tag, Updater updater)
{
    unique_lock<mutex> lock(_mutex);
    if (updater)
    {
        _updaters[tag] = updater;
        try
        {
            _forwarder->attachTags(_id, {{tag->getId(), "", tag->encode(_instance->getCommunicator())}}, false);
        }
        catch (const std::exception&)
        {
            forwarderException();
        }
    }
    else
    {
        _updaters.erase(tag);
        try
        {
            _forwarder->detachTags(_id, {tag->getId()});
        }
        catch (const std::exception&)
        {
            forwarderException();
        }
    }
}

const Topic::Updater&
TopicI::getUpdater(const shared_ptr<Tag>& tag) const
{
    // Called with mutex locked
    auto p = _updaters.find(tag);
    if (p != _updaters.end())
    {
        return p->second;
    }
    return noOpUpdater;
}

void
TopicI::setUpdaters(map<shared_ptr<Tag>, Updater> updaters)
{
    unique_lock<mutex> lock(_mutex);
    _updaters = std::move(updaters);
}

map<shared_ptr<Tag>, Topic::Updater>
TopicI::getUpdaters() const
{
    unique_lock<mutex> lock(_mutex);
    return _updaters;
}

void
TopicI::incListenerCount(const shared_ptr<SessionI>& session)
{
    ++_listenerCount;
    notifyListenerWaiters(session->getTopicLock());
}

void
TopicI::decListenerCount(const shared_ptr<SessionI>& session)
{
    --_listenerCount;
    notifyListenerWaiters(session->getTopicLock());
}

void
TopicI::decListenerCount(size_t listenerCount)
{
    _listenerCount -= listenerCount;
}

void
TopicI::removeFiltered(const shared_ptr<DataElementI>& element, const shared_ptr<Filter>& filter)
{
    auto p = _filteredElements.find(filter);
    if (p != _filteredElements.end())
    {
        p->second.erase(element);
        if (p->second.empty())
        {
            _filteredElements.erase(p);
        }
    }
}

void
TopicI::remove(const shared_ptr<DataElementI>& element, const vector<shared_ptr<Key>>& keys)
{
    if (keys.empty())
    {
        removeFiltered(element, alwaysMatchFilter);
        return;
    }

    for (const auto& key : keys)
    {
        auto p = _keyElements.find(key);
        if (p != _keyElements.end())
        {
            p->second.erase(element);
            if (p->second.empty())
            {
                _keyElements.erase(p);
            }
        }
    }
}

void
TopicI::waitForListeners(int count) const
{
    unique_lock<mutex> lock(_mutex);
    ++_waiters;
    while (true)
    {
        _instance->checkShutdown();
        if (count < 0 && _listenerCount == 0)
        {
            --_waiters;
            return;
        }
        else if (count >= 0 && _listenerCount >= static_cast<size_t>(count))
        {
            --_waiters;
            return;
        }
        _cond.wait(lock);
        ++_notified;
        // Ensure that notifyListenerWaiters checks the wait condition after _notified is incremented.
        _cond.notify_all();
    }
}

bool
TopicI::hasListeners() const
{
    unique_lock<mutex> lock(_mutex);
    return _listenerCount > 0;
}

void
TopicI::notifyListenerWaiters(unique_lock<mutex>& lock) const
{
    if (_waiters > 0)
    {
        _notified = 0;
        _cond.notify_all();
        _cond.wait(lock, [&]() { return _notified < _waiters; }); // Wait until all the waiters are notified.
    }
}

void
TopicI::disconnect()
{
    map<shared_ptr<SessionI>, Listener> listeners;
    {
        unique_lock<mutex> lock(_mutex);
        listeners.swap(_listeners);
    }

    for (const auto& [session, listener] : listeners)
    {
        for (const auto& id : listener.topics)
        {
            session->disconnect(id, this);
        }
    }

#ifndef NDEBUG
    {
        unique_lock<mutex> lock(_mutex);
        assert(_listenerCount == 0);
    }
#endif
}

void
TopicI::forward(const ByteSeq& inParams, const Current& current) const
{
    // Forwarder proxy must be called with the mutex locked!
    for (const auto& [_, listener] : _listeners)
    {
        // Forward the call to all listeners using its session proxy, passing nullptr for the callbacks because we
        // don't need to check the result.
        listener.proxy
            ->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, nullptr, nullptr, current.ctx);
    }
}

void
TopicI::forwarderException() const
{
    try
    {
        rethrow_exception(current_exception());
    }
    catch (const CommunicatorDestroyedException&)
    {
        // Ignore
    }
    catch (const ObjectAdapterDestroyedException&)
    {
        // Ignore
    }
}

void
TopicI::add(const shared_ptr<DataElementI>& element, const vector<shared_ptr<Key>>& keys)
{
    assert(element);

    if (keys.empty())
    {
        addFiltered(element, alwaysMatchFilter);
        return;
    }

    ElementInfoSeq infos;
    for (const auto& key : keys)
    {
        auto p = _keyElements.find(key);
        if (p == _keyElements.end())
        {
            p = _keyElements.emplace(key, set<shared_ptr<DataElementI>>()).first;
        }
        infos.push_back({key->getId(), "", key->encode(_instance->getCommunicator())});
        p->second.insert(element);
    }

    if (!infos.empty())
    {
        try
        {
            _forwarder->announceElements(_id, infos);
        }
        catch (const std::exception&)
        {
            forwarderException();
        }
    }
}

void
TopicI::addFiltered(const shared_ptr<DataElementI>& element, const shared_ptr<Filter>& filter)
{
    auto p = _filteredElements.find(filter);
    if (p == _filteredElements.end())
    {
        p = _filteredElements.emplace(filter, set<shared_ptr<DataElementI>>()).first;
    }
    assert(element);
    p->second.insert(element);
    try
    {
        _forwarder->announceElements(
            _id,
            {{-filter->getId(), filter->getName(), filter->encode(_instance->getCommunicator())}});
    }
    catch (const std::exception&)
    {
        forwarderException();
    }
}

TopicReaderI::TopicReaderI(
    shared_ptr<Instance> instance,
    const shared_ptr<TopicFactoryI>& factory,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories,
    string name,
    int64_t id)
    : TopicI(
          std::move(instance),
          factory,
          std::move(keyFactory),
          std::move(tagFactory),
          std::move(sampleFactory),
          std::move(keyFilterFactories),
          std::move(sampleFilterFactories),
          std::move(name),
          id)
{
    _defaultConfig = parseConfig();
}

shared_ptr<DataReader>
TopicReaderI::createFiltered(
    const shared_ptr<Filter>& filter,
    string name,
    DataStorm::ReaderConfig config,
    string sampleFilterName,
    ByteSeq sampleFilterCriteria)
{
    lock_guard<mutex> lock(_mutex);
    auto element = make_shared<FilteredDataReaderI>(
        this,
        std::move(name),
        ++_nextFilteredId,
        filter,
        std::move(sampleFilterName),
        std::move(sampleFilterCriteria),
        mergeConfigs(config));
    addFiltered(element, filter);
    return element;
}

shared_ptr<DataReader>
TopicReaderI::create(
    const vector<shared_ptr<Key>>& keys,
    string name,
    DataStorm::ReaderConfig config,
    string sampleFilterName,
    ByteSeq sampleFilterCriteria)
{
    lock_guard<mutex> lock(_mutex);
    auto element = make_shared<KeyDataReaderI>(
        this,
        std::move(name),
        ++_nextId,
        keys,
        std::move(sampleFilterName),
        std::move(sampleFilterCriteria),
        mergeConfigs(config));
    add(element, keys);
    return element;
}

void
TopicReaderI::setDefaultConfig(DataStorm::ReaderConfig config)
{
    lock_guard<mutex> lock(_mutex);
    _defaultConfig = mergeConfigs(config);
}

void
TopicReaderI::waitForWriters(int count) const
{
    waitForListeners(count);
}

bool
TopicReaderI::hasWriters() const
{
    return hasListeners();
}

void
TopicReaderI::destroy()
{
    TopicI::destroy();

    if (auto factory = _factory.lock())
    {
        factory->removeTopicReader(_name, shared_from_this());
    }
}

DataStorm::ReaderConfig
TopicReaderI::parseConfig() const
{
    auto properties = _instance->getCommunicator()->getProperties();
    DataStorm::ReaderConfig config;
    config.clearHistory = parseClearHistory(properties->getIceProperty("DataStorm.Topic.ClearHistory"));
    config.sampleCount = properties->getIcePropertyAsInt("DataStorm.Topic.SampleCount");
    config.sampleLifetime = properties->getIcePropertyAsInt("DataStorm.Topic.SampleLifetime");
    config.discardPolicy = parseDiscardPolicy(properties->getIceProperty("DataStorm.Topic.DiscardPolicy"));
    return config;
}

DataStorm::ReaderConfig
TopicReaderI::mergeConfigs(DataStorm::ReaderConfig config) const
{
    if (!config.sampleCount.has_value())
    {
        assert(_defaultConfig.sampleCount.has_value());
        config.sampleCount = _defaultConfig.sampleCount;
    }

    if (!config.sampleLifetime.has_value())
    {
        assert(_defaultConfig.sampleLifetime.has_value());
        config.sampleLifetime = _defaultConfig.sampleLifetime;
    }

    if (!config.clearHistory.has_value())
    {
        assert(_defaultConfig.clearHistory.has_value());
        config.clearHistory = _defaultConfig.clearHistory;
    }

    if (!config.discardPolicy.has_value())
    {
        assert(_defaultConfig.discardPolicy.has_value());
        config.discardPolicy = _defaultConfig.discardPolicy;
    }
    return config;
}

TopicWriterI::TopicWriterI(
    shared_ptr<Instance> instance,
    const shared_ptr<TopicFactoryI>& factory,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories,
    string name,
    int64_t id)
    : TopicI(
          std::move(instance),
          factory,
          std::move(keyFactory),
          std::move(tagFactory),
          std::move(sampleFactory),
          std::move(keyFilterFactories),
          std::move(sampleFilterFactories),
          std::move(name),
          id)
{
    _defaultConfig = parseConfig();
}

shared_ptr<DataWriter>
TopicWriterI::create(const vector<shared_ptr<Key>>& keys, string name, DataStorm::WriterConfig config)
{
    lock_guard<mutex> lock(_mutex);
    auto element = make_shared<KeyDataWriterI>(this, std::move(name), ++_nextId, keys, mergeConfigs(config));
    add(element, keys);
    return element;
}

void
TopicWriterI::setDefaultConfig(DataStorm::WriterConfig config)
{
    lock_guard<mutex> lock(_mutex);
    _defaultConfig = mergeConfigs(config);
}

void
TopicWriterI::waitForReaders(int count) const
{
    waitForListeners(count);
}

bool
TopicWriterI::hasReaders() const
{
    return hasListeners();
}

void
TopicWriterI::destroy()
{
    TopicI::destroy();

    if (auto factory = _factory.lock())
    {
        factory->removeTopicWriter(_name, shared_from_this());
    }
}

DataStorm::WriterConfig
TopicWriterI::parseConfig() const
{
    auto properties = _instance->getCommunicator()->getProperties();
    DataStorm::WriterConfig config;
    config.clearHistory = parseClearHistory(properties->getIceProperty("DataStorm.Topic.ClearHistory"));
    config.sampleCount = properties->getIcePropertyAsInt("DataStorm.Topic.SampleCount");
    config.sampleLifetime = properties->getIcePropertyAsInt("DataStorm.Topic.SampleLifetime");
    config.priority = properties->getIcePropertyAsInt("DataStorm.Topic.Priority");
    return config;
}

DataStorm::WriterConfig
TopicWriterI::mergeConfigs(DataStorm::WriterConfig config) const
{
    if (!config.sampleCount.has_value())
    {
        assert(_defaultConfig.sampleCount.has_value());
        config.sampleCount = _defaultConfig.sampleCount;
    }

    if (!config.sampleLifetime.has_value())
    {
        assert(_defaultConfig.sampleLifetime.has_value());
        config.sampleLifetime = _defaultConfig.sampleLifetime;
    }

    if (!config.clearHistory.has_value())
    {
        assert(_defaultConfig.clearHistory.has_value());
        config.clearHistory = _defaultConfig.clearHistory;
    }

    if (!config.priority.has_value())
    {
        assert(_defaultConfig.priority.has_value());
        config.priority = _defaultConfig.priority;
    }
    return config;
}
