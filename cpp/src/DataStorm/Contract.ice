//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
#pragma once

#include <Ice/Identity.ice>
#include <DataStorm/Sample.ice>

[["cpp:include:deque"]]

module DataStormContract
{

enum ClearHistoryPolicy
{
    OnAdd,
    OnRemove,
    OnAll,
    OnAllExceptPartialUpdate,
    Never
};

/** A sequence of bytes use to hold the encoded key or value */
sequence<byte> ByteSeq;

/** A sequence of long */
sequence<long> LongSeq;

/** A sequence of strings */
sequence<string> StringSeq;

/** A dictionary of <long, long> */
dictionary<long, long> LongLongDict;

struct DataSample
{
    /** The sample id. */
    long id;

    /** The key id. */
    long keyId;

    /** The key value if the key ID <= 0. */
    ByteSeq keyValue;

    /** The timestamp of the sample (write time). */
    long timestamp;

    /** The update tag if the sample event is PartialUpdate. */
    long tag;

    /** The sample event. */
    DataStorm::SampleEvent event;

    /** The value of the sample. */
    ByteSeq value;
}
["cpp:type:std::deque<DataSample>"] sequence<DataSample> DataSampleSeq;

struct DataSamples
{
    /** The id of the writer or reader. */
    long id;

    /** The samples. */
    DataSampleSeq samples;
}
sequence<DataSamples> DataSamplesSeq;

struct ElementInfo
{
    /** The key or filter id. */
    long id;

    /** The filter name. */
    string name;

    /** The key or filter value. */
    ByteSeq value;
}
sequence<ElementInfo> ElementInfoSeq;

struct TopicInfo
{
    /** The topic name. */
    string name;

    /** The id of topic writers or readers. */
    LongSeq ids;
}
sequence<TopicInfo> TopicInfoSeq;

struct TopicSpec
{
    /** The id of the topic. */
    long id;

    /* The name of the topic. */
    string name;

    /** The topic keys or filters. */
    ElementInfoSeq elements;

    /** The topic update tags. */
    ElementInfoSeq tags;
};

struct FilterInfo
{
    string name;
    ByteSeq criteria;
};

class ElementConfig(1)
{
    optional(1) string facet;
    optional(2) FilterInfo sampleFilter;
    optional(3) string name;
    optional(4) int priority;

    optional(10) int sampleCount;
    optional(11) int sampleLifetime;
    optional(12) ClearHistoryPolicy clearHistory;
};

struct ElementData
{
    /** The id of the writer or reader */
    long id;

    /** The config of the writer or reader */
    ElementConfig config;

    /** The lastIds received by the reader. */
    LongLongDict lastIds;
}
sequence<ElementData> ElementDataSeq;

struct ElementSpec
{
    /** The readers and writers associated with the key or filter. */
    ElementDataSeq elements;

    /** The id of the key or filter */
    long id;

    /** The name of the filter. */
    string name;

    /** The value of the key or filter. */
    ByteSeq value;

    /** The id of the key or filter from the peer. */
    long peerId;

    /** The name of the filter from the peer. */
    string peerName;
}
sequence<ElementSpec> ElementSpecSeq;

struct ElementDataAck
{
    /** The id of the writer or filter. */
    long id;

    /** The config of the writer or reader */
    ElementConfig config;

    /** The lastIds received by the reader. */
    LongLongDict lastIds;

    /** The samples of the writer or reader */
    DataSampleSeq samples;

    /** The id of the writer or reader on the peer. */
    long peerId;
}
sequence<ElementDataAck> ElementDataAckSeq;

struct ElementSpecAck
{
    /** The readers or writers associated with the key or filter. */
    ElementDataAckSeq elements;

    /** The id of the key or filter. */
    long id;

    /** The name of the filter. */
    string name;

    /** The key or filter value. */
    ByteSeq value;

    /** The id of the key or filter on the peer. */
    long peerId;

    /** The name of the filter on the peer. */
    string peerName;
}
sequence<ElementSpecAck> ElementSpecAckSeq;

interface Session
{
    void announceTopics(TopicInfoSeq topics, bool initialize);
    void attachTopic(TopicSpec topic);
    void detachTopic(long topic);

    void attachTags(long topic, ElementInfoSeq tags, bool initialize);
    void detachTags(long topic, LongSeq tags);

    void announceElements(long topic, ElementInfoSeq keys);
    void attachElements(long topic, ElementSpecSeq elements, bool initialize);
    void attachElementsAck(long topic, ElementSpecAckSeq elements);
    void detachElements(long topic, LongSeq keys);

    void initSamples(long topic, DataSamplesSeq samples);

    void disconnected();
}

interface PublisherSession extends Session
{
}

interface SubscriberSession extends Session
{
    void s(long topicId, long elementId, DataSample sample);
}

interface Node
{
    void initiateCreateSession(Node* publisher);
    void createSession(Node* subscriber, SubscriberSession* session, bool fromRelay);
    void confirmCreateSession(Node* publisher, PublisherSession* session);
}

interface Lookup
{
    idempotent void announceTopicReader(string topic, Node* node);
    idempotent void announceTopicWriter(string topic, Node* node);

    idempotent void announceTopics(StringSeq readers, StringSeq writers, Node* node);

    Node* createSession(Node* node);
}

}
