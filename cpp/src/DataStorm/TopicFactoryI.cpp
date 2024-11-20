//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TopicFactoryI.h"
#include "Instance.h"
#include "NodeI.h"
#include "NodeSessionManager.h"
#include "TopicI.h"
#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;

TopicFactoryI::TopicFactoryI(const shared_ptr<Instance>& instance)
    : _instance(instance),
      _traceLevels(instance->getTraceLevels()),
      _nextReaderId(0),
      _nextWriterId(0)
{
}

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
    bool hasWriters;
    {
        lock_guard<mutex> lock(_mutex);
        reader = make_shared<TopicReaderI>(
            shared_from_this(),
            std::move(keyFactory),
            std::move(tagFactory),
            std::move(sampleFactory),
            std::move(keyFilterFactories),
            std::move(sampleFilterFactories),
            name, // we keep using name below
            _nextReaderId++);
        _readers[name].push_back(reader);
        if (_traceLevels->topic > 0)
        {
            Trace out(_traceLevels, _traceLevels->topicCat);
            out << name << ": created topic reader";
        }

        hasWriters = _writers.find(name) != _writers.end();
    }

    try
    {
        auto instance = getInstance();
        auto node = instance->getNode();
        auto nodePrx = node->getProxy();
        if (hasWriters)
        {
            node->createSubscriberSession(nodePrx, nullptr, nullptr);
        }
        node->getSubscriberForwarder()->announceTopics({{name, {reader->getId()}}}, false);
        instance->getNodeSessionManager()->announceTopicReader(name, nodePrx);
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
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
    bool hasReaders;
    {
        lock_guard<mutex> lock(_mutex);
        writer = make_shared<TopicWriterI>(
            shared_from_this(),
            std::move(keyFactory),
            std::move(tagFactory),
            std::move(sampleFactory),
            std::move(keyFilterFactories),
            std::move(sampleFilterFactories),
            name, // we keep using name below
            _nextWriterId++);
        _writers[name].push_back(writer);
        if (_traceLevels->topic > 0)
        {
            Trace out(_traceLevels, _traceLevels->topicCat);
            out << name << ": created topic writer";
        }

        hasReaders = _readers.find(name) != _readers.end();
    }

    try
    {
        auto instance = getInstance();
        auto node = instance->getNode();
        auto nodePrx = node->getProxy();
        if (hasReaders)
        {
            node->createPublisherSession(nodePrx, nullptr, nullptr);
        }
        node->getPublisherForwarder()->announceTopics({{name, {writer->getId()}}}, false);
        instance->getNodeSessionManager()->announceTopicWriter(name, nodePrx);
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
    }

    return writer;
}

void
TopicFactoryI::removeTopicReader(const string& name, const shared_ptr<TopicI>& reader)
{
    lock_guard<mutex> lock(_mutex);
    if (_traceLevels->topic > 0)
    {
        Trace out(_traceLevels, _traceLevels->topicCat);
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
    if (_traceLevels->topic > 0)
    {
        Trace out(_traceLevels, _traceLevels->topicCat);
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
        return vector<shared_ptr<TopicI>>();
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
        return vector<shared_ptr<TopicI>>();
    }
    return p->second;
}

void
TopicFactoryI::createPublisherSession(
    const string& topic,
    DataStormContract::NodePrx publisher,
    const Ice::ConnectionPtr& connection)
{
    auto readers = getTopicReaders(topic);
    if (!readers.empty())
    {
        getInstance()->getNode()->createPublisherSession(publisher, connection, nullptr);
    }
}

void
TopicFactoryI::createSubscriberSession(
    const string& topic,
    DataStormContract::NodePrx subscriber,
    const Ice::ConnectionPtr& connection)
{
    auto writers = getTopicWriters(topic);
    if (!writers.empty())
    {
        getInstance()->getNode()->createSubscriberSession(subscriber, connection, nullptr);
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

Ice::StringSeq
TopicFactoryI::getTopicReaderNames() const
{
    lock_guard<mutex> lock(_mutex);
    Ice::StringSeq readers;
    readers.reserve(_readers.size());
    for (const auto& p : _readers)
    {
        readers.push_back(p.first);
    }
    return readers;
}

Ice::StringSeq
TopicFactoryI::getTopicWriterNames() const
{
    lock_guard<mutex> lock(_mutex);
    Ice::StringSeq writers;
    writers.reserve(_writers.size());
    for (const auto& p : _writers)
    {
        writers.push_back(p.first);
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

Ice::CommunicatorPtr
TopicFactoryI::getCommunicator() const
{
    return getInstance()->getCommunicator();
}
