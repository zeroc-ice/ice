// Copyright (c) ZeroC, Inc.

#include "TopicFactoryI.h"
#include "Instance.h"
#include "NodeI.h"
#include "NodeSessionManager.h"
#include "TopicI.h"
#include "TraceUtil.h"

#include <algorithm>

using namespace std;
using namespace DataStormI;
using namespace DataStormContract;
using namespace Ice;

TopicFactoryI::TopicFactoryI(const shared_ptr<Instance>& instance) : _instance{instance} {}

shared_ptr<TopicReader>
TopicFactoryI::createTopicReader(
    string name,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories)
{
    shared_ptr<TopicReaderI> reader;
    auto instance = _instance.lock();
    assert(instance);
    bool hasWriters;
    {
        lock_guard<mutex> lock(_mutex);
        reader = make_shared<TopicReaderI>(
            instance,
            shared_from_this(),
            std::move(keyFactory),
            std::move(tagFactory),
            std::move(sampleFactory),
            std::move(keyFilterFactories),
            std::move(sampleFilterFactories),
            name, // we keep using name below
            _nextReaderId++);
        _readers[name].push_back(reader);
        if (instance->getTraceLevels()->topic > 0)
        {
            Trace out(instance->getTraceLevels()->logger, instance->getTraceLevels()->topicCat);
            out << name << ": created topic reader";
        }

        hasWriters = _writers.find(name) != _writers.end();
    }

    try
    {
        auto node = instance->getNode();
        auto nodePrx = node->getProxy();
        if (hasWriters)
        {
            node->createSubscriberSession(nodePrx, nullptr, nullptr);
        }
        node->getSubscriberForwarder()->announceTopics({TopicInfo{.name = name, .ids = {reader->getId()}}}, false);
        instance->getNodeSessionManager()->announceTopicReader(name, nodePrx);
    }
    catch (const CommunicatorDestroyedException&)
    {
    }
    catch (const ObjectAdapterDestroyedException&)
    {
    }
    return reader;
}

shared_ptr<TopicWriter>
TopicFactoryI::createTopicWriter(
    string name,
    shared_ptr<KeyFactory> keyFactory,
    shared_ptr<TagFactory> tagFactory,
    shared_ptr<SampleFactory> sampleFactory,
    shared_ptr<FilterManager> keyFilterFactories,
    shared_ptr<FilterManager> sampleFilterFactories)
{
    shared_ptr<TopicWriterI> writer;
    auto instance = _instance.lock();
    assert(instance);
    bool hasReaders;
    {
        lock_guard<mutex> lock(_mutex);
        writer = make_shared<TopicWriterI>(
            instance,
            shared_from_this(),
            std::move(keyFactory),
            std::move(tagFactory),
            std::move(sampleFactory),
            std::move(keyFilterFactories),
            std::move(sampleFilterFactories),
            name, // we keep using name below
            _nextWriterId++);
        _writers[name].push_back(writer);

        if (instance->getTraceLevels()->topic > 0)
        {
            Trace out(instance->getTraceLevels()->logger, instance->getTraceLevels()->topicCat);
            out << name << ": created topic writer";
        }

        hasReaders = _readers.find(name) != _readers.end();
    }

    try
    {
        auto node = instance->getNode();
        auto nodePrx = node->getProxy();
        if (hasReaders)
        {
            try
            {
                node->createPublisherSession(nodePrx, nullptr, nullptr);
            }
            catch (const SessionCreationException&)
            {
                // Session creation failed upon receiving a writer announcement. This can happen if:
                //
                // - The session is already connected.
                // - The node that sent the announcement is shutting down.
                // - This node is shutting down.
                //
                // In all cases, no further action is required, and the exception can safely be ignored.
            }
        }
        node->getPublisherForwarder()->announceTopics({TopicInfo{.name = name, .ids = {writer->getId()}}}, false);
        instance->getNodeSessionManager()->announceTopicWriter(name, nodePrx);
    }
    catch (const CommunicatorDestroyedException&)
    {
    }
    catch (const ObjectAdapterDestroyedException&)
    {
    }

    return writer;
}

void
TopicFactoryI::removeTopicReader(const string& name, const shared_ptr<TopicI>& reader)
{
    lock_guard<mutex> lock(_mutex);
    auto instance = _instance.lock();
    assert(instance);
    if (instance->getTraceLevels()->topic > 0)
    {
        Trace out(instance->getTraceLevels()->logger, instance->getTraceLevels()->topicCat);
        out << name << ": destroyed topic reader";
    }
    auto& readers = _readers[name];
    readers.erase(find(readers.begin(), readers.end(), reader));
    if (readers.empty())
    {
        _readers.erase(name);
    }
}

void
TopicFactoryI::removeTopicWriter(const string& name, const shared_ptr<TopicI>& writer)
{
    lock_guard<mutex> lock(_mutex);
    auto instance = _instance.lock();
    assert(instance);
    if (instance->getTraceLevels()->topic > 0)
    {
        Trace out(instance->getTraceLevels()->logger, instance->getTraceLevels()->topicCat);
        out << name << ": destroyed topic writer";
    }
    auto& writers = _writers[name];
    writers.erase(find(writers.begin(), writers.end(), writer));
    if (writers.empty())
    {
        _writers.erase(name);
    }
}

vector<shared_ptr<TopicI>>
TopicFactoryI::getTopicReaders(const string& name) const
{
    lock_guard<mutex> lock(_mutex);
    auto p = _readers.find(name);
    if (p == _readers.end())
    {
        return {};
    }
    return p->second;
}

vector<shared_ptr<TopicI>>
TopicFactoryI::getTopicWriters(const string& name) const
{
    lock_guard<mutex> lock(_mutex);
    auto p = _writers.find(name);
    if (p == _writers.end())
    {
        return {};
    }
    return p->second;
}

void
TopicFactoryI::createPublisherSession(
    const string& topic,
    const DataStormContract::NodePrx& publisher,
    const ConnectionPtr& connection)
{
    auto readers = getTopicReaders(topic);
    if (!readers.empty())
    {
        auto instance = _instance.lock();
        assert(instance);
        try
        {
            instance->getNode()->createPublisherSession(publisher, connection, nullptr);
        }
        catch (const SessionCreationException&)
        {
            // Session creation failed upon receiving a writer announcement. This can happen if:
            //
            // - The session is already connected.
            // - The node that sent the announcement is shutting down.
            // - This node is shutting down.
            //
            // In all cases, no further action is required, and the exception can safely be ignored.
        }
        catch (const CommunicatorDestroyedException&)
        {
            // The node is shutting down.
        }
        catch (const ObjectAdapterDestroyedException&)
        {
            // The node is shutting down.
        }
    }
}

void
TopicFactoryI::createSubscriberSession(
    const string& topic,
    const DataStormContract::NodePrx& subscriber,
    const ConnectionPtr& connection)
{
    auto writers = getTopicWriters(topic);
    if (!writers.empty())
    {
        auto instance = _instance.lock();
        assert(instance);
        instance->getNode()->createSubscriberSession(subscriber, connection, nullptr);
    }
}

DataStormContract::TopicInfoSeq
TopicFactoryI::getTopicReaders() const
{
    lock_guard<mutex> lock(_mutex);
    DataStormContract::TopicInfoSeq readers;
    readers.reserve(_readers.size());
    for (const auto& p : _readers)
    {
        DataStormContract::TopicInfo info;
        info.name = p.first;
        info.ids.reserve(p.second.size());
        for (const auto& q : p.second)
        {
            info.ids.push_back(q->getId());
        }
        readers.push_back(std::move(info));
    }
    return readers;
}

DataStormContract::TopicInfoSeq
TopicFactoryI::getTopicWriters() const
{
    lock_guard<mutex> lock(_mutex);
    DataStormContract::TopicInfoSeq writers;
    writers.reserve(_writers.size());
    for (const auto& p : _writers)
    {
        DataStormContract::TopicInfo info;
        info.name = p.first;
        info.ids.reserve(p.second.size());
        for (const auto& q : p.second)
        {
            info.ids.push_back(q->getId());
        }
        writers.push_back(std::move(info));
    }
    return writers;
}

StringSeq
TopicFactoryI::getTopicReaderNames() const
{
    lock_guard<mutex> lock(_mutex);
    StringSeq readers;
    readers.reserve(_readers.size());
    for (const auto& [name, _] : _readers)
    {
        readers.push_back(name);
    }
    return readers;
}

StringSeq
TopicFactoryI::getTopicWriterNames() const
{
    lock_guard<mutex> lock(_mutex);
    StringSeq writers;
    writers.reserve(_writers.size());
    for (const auto& [name, _] : _writers)
    {
        writers.push_back(name);
    }
    return writers;
}

void
TopicFactoryI::shutdown() const
{
    lock_guard<mutex> lock(_mutex);
    for (const auto& p : _writers)
    {
        for (const auto& w : p.second)
        {
            w->shutdown();
        }
    }

    for (const auto& p : _readers)
    {
        for (const auto& r : p.second)
        {
            r->shutdown();
        }
    }
}

CommunicatorPtr
TopicFactoryI::getCommunicator() const
{
    auto instance = _instance.lock();
    assert(instance);
    return instance->getCommunicator();
}
