//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TopicI.h"
#include "NodeI.h"
#include "SessionI.h"
#include "TopicFactoryI.h"

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;

namespace
{
    static Topic::Updater noOpUpdater =
        [](const shared_ptr<Sample>& previous, const shared_ptr<Sample>& next, const Ice::CommunicatorPtr&)
    { next->setValue(previous); };

    // The always match filter always matches the value, it's used by the any key reader/writer.
    class AlwaysMatchFilter final : public Filter
    {
    public:
        string toString() const final { return "f1:alwaysmatch"; }

        const string& getName() const final
        {
            static string alwaysmatch("alwaysmatch");
            return alwaysmatch;
        }

        Ice::ByteSeq encode(const Ice::CommunicatorPtr&) const final { return {}; }

        int64_t getId() const final
        {
            return 1; // 1 is reserved for the match all filter.
        }

        bool match(const shared_ptr<Filterable>&) const final { return true; }
    };
    const auto alwaysMatchFilter = make_shared<AlwaysMatchFilter>();

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
            throw Ice::ParseException(__FILE__, __LINE__, "Invalid clear history policy: " + value);
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
            throw Ice::ParseException(__FILE__, __LINE__, "Invalid discard policy: " + value);
        }
    }
}

TopicI::TopicI(
    weak_ptr<TopicFactoryI> factory,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories,
    string name,
    int64_t id)
    : _factory(std::move(factory)),
      _keyFactory(std::move(keyFactory)),
      _tagFactory(std::move(tagFactory)),
      _sampleFactory(std::move(sampleFactory)),
      _keyFilterFactories(std::move(keyFilterFactories)),
      _sampleFilterFactories(std::move(sampleFilterFactories)),
      _name(std::move(name)),
      _instance(_factory.lock()->getInstance()),
      _traceLevels(_instance->getTraceLevels()),
      _id(id),
      _forwarder{_instance->getCollocatedForwarder()->add<SessionPrx>(
          [this](Ice::ByteSeq inParams, const Ice::Current& current) { forward(inParams, current); })},
      _destroyed(false),
      _listenerCount(0),
      _waiters(0),
      _notified(0),
      _nextId(0),
      _nextFilteredId(0),
      _nextSampleId(0)
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
    for (auto k : _keyElements)
    {
        spec.elements.push_back({k.first->getId(), "", k.first->encode(_instance->getCommunicator())});
    }
    for (auto f : _filteredElements)
    {
        spec.elements.push_back({-f.first->getId(), f.first->getName(), f.first->encode(_instance->getCommunicator())});
    }
    spec.tags = getTags();
    return spec;
}

ElementInfoSeq
TopicI::getTags() const
{
    ElementInfoSeq tags;
    tags.reserve(_updaters.size());
    for (auto u : _updaters)
    {
        tags.push_back({u.first->getId(), "", u.first->encode(_instance->getCommunicator())});
    }
    return tags;
}

ElementSpecSeq
TopicI::getElementSpecs(int64_t topicId, const ElementInfoSeq& infos, const shared_ptr<SessionI>& session)
{
    ElementSpecSeq specs;
    for (const auto& info : infos)
    {
        if (info.id > 0) // Key
        {
            auto key = _keyFactory->decode(_instance->getCommunicator(), info.value);
            auto p = _keyElements.find(key);
            if (p != _keyElements.end())
            {
                ElementDataSeq elements;
                for (auto k : p->second)
                {
                    elements.push_back({k->getId(), k->getConfig(), session->getLastIds(topicId, info.id, k)});
                }
                specs.push_back({std::move(elements), key->getId(), "", {}, info.id, ""});
            }
            for (auto e : _filteredElements)
            {
                if (e.first->match(key))
                {
                    ElementDataSeq elements;
                    for (auto f : e.second)
                    {
                        elements.push_back({f->getId(), f->getConfig(), session->getLastIds(topicId, info.id, f)});
                    }
                    specs.push_back(
                        {std::move(elements),
                         -e.first->getId(),
                         e.first->getName(),
                         e.first->encode(_instance->getCommunicator()),
                         info.id,
                         ""});
                }
            }
        }
        else
        {
            shared_ptr<Filter> filter;
            if (info.value.empty())
            {
                filter = alwaysMatchFilter;
            }
            else
            {
                filter = _keyFilterFactories->decode(_instance->getCommunicator(), info.name, info.value);
            }

            for (auto e : _keyElements)
            {
                if (filter->match(e.first))
                {
                    ElementDataSeq elements;
                    for (auto k : e.second)
                    {
                        elements.push_back({k->getId(), k->getConfig(), session->getLastIds(topicId, info.id, k)});
                    }
                    specs.push_back(
                        {std::move(elements),
                         e.first->getId(),
                         "",
                         e.first->encode(_instance->getCommunicator()),
                         info.id,
                         info.name});
                }
            }

            if (filter == alwaysMatchFilter)
            {
                for (auto e : _filteredElements)
                {
                    ElementDataSeq elements;
                    for (auto f : e.second)
                    {
                        elements.push_back({f->getId(), f->getConfig(), session->getLastIds(topicId, info.id, f)});
                    }
                    specs.push_back(
                        {std::move(elements),
                         -e.first->getId(),
                         e.first->getName(),
                         e.first->encode(_instance->getCommunicator()),
                         info.id,
                         info.name});
                }
            }
            else
            {
                auto p = _filteredElements.find(alwaysMatchFilter);
                if (p != _filteredElements.end())
                {
                    ElementDataSeq elements;
                    for (auto f : p->second)
                    {
                        elements.push_back({f->getId(), f->getConfig(), session->getLastIds(topicId, info.id, f)});
                    }
                    specs.push_back(
                        {std::move(elements),
                         -alwaysMatchFilter->getId(),
                         alwaysMatchFilter->getName(),
                         alwaysMatchFilter->encode(_instance->getCommunicator()),
                         info.id,
                         info.name});
                }
            }
        }
    }
    return specs;
}

void
TopicI::attach(int64_t id, shared_ptr<SessionI> session, SessionPrx peerSession)
{
    auto p = _listeners.find(session);
    if (p == _listeners.end())
    {
        p = _listeners.emplace(std::move(session), Listener(std::move(peerSession))).first;
    }

    if (p->second.topics.insert(id).second)
    {
        p->first->subscribe(id, this);
    }
}

void
TopicI::detach(int64_t id, const shared_ptr<SessionI>& session)
{
    auto p = _listeners.find(session);
    if (p != _listeners.end() && p->second.topics.erase(id))
    {
        session->unsubscribe(id, this);
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
    SessionPrx prx,
    const chrono::time_point<chrono::system_clock>& now)
{
    ElementSpecAckSeq specs;
    for (const auto& spec : elements)
    {
        if (spec.peerId > 0) // Key
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

                for (auto e : p->second)
                {
                    ElementDataAckSeq acks;
                    for (const auto& data : spec.elements)
                    {
                        if (spec.id > 0) // Key
                        {
                            e->attach(topicId, spec.id, key, nullptr, session, prx, data, now, acks);
                        }
                        else if (filter->match(key))
                        {
                            e->attach(topicId, spec.id, key, filter, session, prx, data, now, acks);
                        }
                    }

                    if (!acks.empty())
                    {
                        specs.push_back(
                            {std::move(acks),
                             key->getId(),
                             "",
                             spec.id < 0 ? key->encode(_instance->getCommunicator()) : Ice::ByteSeq{},
                             spec.id,
                             spec.name});
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

                for (auto e : p->second)
                {
                    ElementDataAckSeq acks;
                    for (const auto& data : spec.elements)
                    {
                        if (spec.id < 0) // Filter
                        {
                            e->attach(topicId, spec.id, nullptr, filter, session, prx, data, now, acks);
                        }
                        else if (filter->match(key))
                        {
                            e->attach(topicId, spec.id, key, filter, session, prx, data, now, acks);
                        }
                    }
                    if (!acks.empty())
                    {
                        specs.push_back(
                            {std::move(acks),
                             -filter->getId(),
                             filter->getName(),
                             spec.id > 0 ? filter->encode(_instance->getCommunicator()) : Ice::ByteSeq{},
                             spec.id,
                             spec.name});
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
    SessionPrx prx,
    const chrono::time_point<chrono::system_clock>& now,
    Ice::LongSeq& removedIds)
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

                vector<shared_ptr<Sample>> samplesI;
                for (const auto& data : spec.elements)
                {
                    bool found = false;
                    for (auto e : p->second)
                    {
                        if (data.peerId == e->getId())
                        {
                            function<void()> initCb;
                            if (spec.id > 0) // Key
                            {
                                initCb = e->attach(topicId, spec.id, key, nullptr, session, prx, data, now, samples);
                            }
                            else if (filter->match(key)) // Filter
                            {
                                initCb = e->attach(topicId, spec.id, key, filter, session, prx, data, now, samples);
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
                    for (auto e : p->second)
                    {
                        if (data.peerId == e->getId())
                        {
                            function<void()> initCb;
                            if (spec.id < 0) // Filter
                            {
                                initCb = e->attach(topicId, spec.id, nullptr, filter, session, prx, data, now, samples);
                            }
                            else if (filter->match(key))
                            {
                                initCb = e->attach(topicId, spec.id, key, nullptr, session, prx, data, now, samples);
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
    for (auto initCb : initCallbacks)
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

    for (auto key : keys)
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

    for (auto s : listeners)
    {
        for (auto t : s.second.topics)
        {
            s.first->disconnect(t, this);
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
TopicI::forward(const Ice::ByteSeq& inParams, const Ice::Current& current) const
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
    catch (const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
        // Ignore
    }
}

void
TopicI::add(const shared_ptr<DataElementI>& element, const vector<shared_ptr<Key>>& keys)
{
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
        assert(element);
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
    shared_ptr<TopicFactoryI> factory,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories,
    string name,
    int64_t id)
    : TopicI(
          std::move(factory),
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
    Ice::ByteSeq sampleFilterCriteria)
{
    lock_guard<mutex> lock(_mutex);
    auto element = make_shared<FilteredDataReaderI>(
        this,
        std::move(name),
        ++_nextFilteredId,
        filter,
        std::move(sampleFilterName),
        std::move(sampleFilterCriteria),
        mergeConfigs(std::move(config)));
    addFiltered(element, filter);
    return element;
}

shared_ptr<DataReader>
TopicReaderI::create(
    const vector<shared_ptr<Key>>& keys,
    string name,
    DataStorm::ReaderConfig config,
    string sampleFilterName,
    Ice::ByteSeq sampleFilterCriteria)
{
    lock_guard<mutex> lock(_mutex);
    auto element = make_shared<KeyDataReaderI>(
        this,
        std::move(name),
        ++_nextId,
        keys,
        std::move(sampleFilterName),
        std::move(sampleFilterCriteria),
        mergeConfigs(std::move(config)));
    add(element, keys);
    return element;
}

void
TopicReaderI::setDefaultConfig(DataStorm::ReaderConfig config)
{
    lock_guard<mutex> lock(_mutex);
    _defaultConfig = mergeConfigs(std::move(config));
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

    auto factory = _factory.lock();
    if (factory)
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
    shared_ptr<TopicFactoryI> factory,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories,
    string name,
    int64_t id)
    : TopicI(
          std::move(factory),
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
    auto element = make_shared<KeyDataWriterI>(this, std::move(name), ++_nextId, keys, mergeConfigs(std::move(config)));
    add(element, keys);
    return element;
}

void
TopicWriterI::setDefaultConfig(DataStorm::WriterConfig config)
{
    lock_guard<mutex> lock(_mutex);
    _defaultConfig = mergeConfigs(std::move(config));
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

    auto factory = _factory.lock();
    if (factory)
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
