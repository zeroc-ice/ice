//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
#pragma once

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <DataStorm/Sample.ice>

[["cpp:include:deque"]]

module DataStormContract
{
    /// The ClearHistoryPolicy enumeration defines the policy that determines when a reader clears its
    /// DataSample history in response to various events.
    enum ClearHistoryPolicy
    {
        /// The reader clears its history when a new DataSample is added.
        OnAdd,
        /// The reader clears its history when a DataSample is removed.
        OnRemove,
        /// The reader clears its history when any DataSample event occurs.
        OnAll,
        /// The reader clears its history when any DataSample event occurs, except for PartialUpdate events.
        OnAllExceptPartialUpdate,
        /// The reader never clears its history.
        Never
    }

    dictionary<long, long> LongLongDict;

    struct DataSample
    {
        /// The sample id.
        long id;

        /// The key id.
        long keyId;

        /// The key value if the key ID <= 0.
        Ice::ByteSeq keyValue;

        /// The timestamp of the sample (write time).
        long timestamp;

        /// The update tag if the sample event is PartialUpdate.
        long tag;

        /// The sample event.
        DataStorm::SampleEvent event;

        /// The value of the sample.
        Ice::ByteSeq value;
    }

    // A queue of DataSample
    ["cpp:type:std::deque<DataSample>"] sequence<DataSample> DataSampleSeq;

    struct DataSamples
    {
        /// The id of the writer or reader.
        long id;

        /// The samples.
        DataSampleSeq samples;
    }
    sequence<DataSamples> DataSamplesSeq;

    /// Provides information about an element, which can be a key, a filter, or a tag. Includes the element's ID, name,
    /// and encoded value.
    struct ElementInfo
    {
        /// The unique identifier of the element. Filter IDs are negative.
        long id;

        /// The name of the filter. This field is empty for key and tag elements.
        string name;

        /// The encoded value of the element.
        Ice::ByteSeq value;
    }
    sequence<ElementInfo> ElementInfoSeq;

    /// Provides information about a topic, including its name and the list of active topic reader or topic writer IDs.
    ///
    /// There is a unique `TopicInfo` for all topic instances with the same name, representing a single logical topic.
    /// Each instance has its own topic reader and topic writer, which are lazily initialized and have a unique ID.
    ///
    /// @see Session#announceTopics
    struct TopicInfo
    {
        /// The name of the topic.
        string name;

        /// The list of active topic reader or topic writer IDs for the topic.
        ///
        /// - In a publisher session announcing topics to a subscriber session, this contains the active topic writer
        /// IDs.
        /// - In a subscriber session announcing topics to a publisher session, this contains the active topic reader
        /// IDs.
        Ice::LongSeq ids;
    }

    /// Represents a sequence of active topics used for transmitting topic information during session establishment.
    ///
    /// @see Session#announceTopics
    sequence<TopicInfo> TopicInfoSeq;

    /// Provides detailed information about topic readers and topic writers, including its ID, name, keys, filters,
    /// and tags.
    ///
    /// @see Session#attachTopic
    struct TopicSpec
    {
        /// The ID of the topic.
        long id;

        /// The name of the topic.
        string name;

        /// The topic's keys and filters.
        ElementInfoSeq elements;

        /// The topic update tags.
        ElementInfoSeq tags;
    }

    struct FilterInfo
    {
        string name;
        Ice::ByteSeq criteria;
    }

    class ElementConfig(1)
    {
        optional(1) string facet;
        optional(2) FilterInfo sampleFilter;
        optional(3) string name;
        optional(4) int priority;

        optional(10) int sampleCount;
        optional(11) int sampleLifetime;
        optional(12) ClearHistoryPolicy clearHistory;
    }

    struct ElementData
    {
        /// The id of the writer or reader.
        long id;

        /// The config of the writer or reader.
        ElementConfig config;

        /// The lastIds received by the reader.
        LongLongDict lastIds;
    }
    sequence<ElementData> ElementDataSeq;

    /// Provides detailed information about elements that can be either a key or a filter.
    struct ElementSpec
    {
        /// The readers and writers associated with the key or filter.
        ElementDataSeq elements;

        /// The id of the key or filter.
        long id;

        /// The name of the filter. This field is empty for key elements.
        string name;

        /// The value of the key or filter.
        Ice::ByteSeq value;

        /// The id of the key or filter from the peer.
        long peerId;

        /// The name of the filter from the peer.
        string peerName;
    }
    sequence<ElementSpec> ElementSpecSeq;

    struct ElementDataAck
    {
        /// The id of the writer or filter.
        long id;

        /// The config of the writer or reader.
        ElementConfig config;

        /// The lastIds received by the reader.
        LongLongDict lastIds;

        /// The samples of the writer or reader.
        DataSampleSeq samples;

        /// The id of the writer or reader on the peer.
        long peerId;
    }
    sequence<ElementDataAck> ElementDataAckSeq;

    struct ElementSpecAck
    {
        /// The readers or writers associated with the key or filter.
        ElementDataAckSeq elements;

        /// The id of the key or filter.
        long id;

        /// The name of the filter.
        string name;

        /// The key or filter value.
        Ice::ByteSeq value;

        /// The id of the key or filter on the peer.
        long peerId;

        /// The name of the filter on the peer.
        string peerName;
    }
    sequence<ElementSpecAck> ElementSpecAckSeq;

    interface Session
    {
        /// Announces existing topics to the peer during session establishment.
        /// A publisher session announces the topics it writes, while a subscriber session announces the topics it reads.
        ///
        /// The peer receiving the announcement will invoke `attachTopic` for the topics it is interested in.
        ///
        /// @param topics The sequence of topics to announce.
        /// @param initialize Currently unused.
        /// @see attachTopic
        void announceTopics(TopicInfoSeq topics, bool initialize);

        /// Attaches a local topic to a remote topic when a session receives a topic announcement from a peer.
        ///
        /// This method is called if the session is interested in the announced topic, which occurs when:
        /// - The session has a reader for a topic that the peer has a writer for, or
        /// - The session has a writer for a topic that the peer has a reader for.
        ///
        /// @param topic The TopicSpec object describing the topic being attached to the remote topic.
        void attachTopic(TopicSpec topic);

        void detachTopic(long topic);

        void attachTags(long topic, ElementInfoSeq tags, bool initialize);
        void detachTags(long topic, Ice::LongSeq tags);

        /// Announces new elements to the peer.
        /// The peer will invoke `attachElements` for the elements it is interested in. The announced elements include
        /// key readers, key writers, and filter readers associated with the specified topic.
        ///
        /// @param topic The ID of the topic associated with the elements.
        /// @param elements The sequence of elements to announce.
        /// @see attachElements
        void announceElements(long topic, ElementInfoSeq elements);

        void attachElements(long topic, ElementSpecSeq elements, bool initialize);
        void attachElementsAck(long topic, ElementSpecAckSeq elements);
        void detachElements(long topic, Ice::LongSeq keys);

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

    /// The Node interface allows DataStorm nodes to create publisher and subscriber sessions with each other.
    ///
    /// When a node has a writer for a topic that another node is reading, the node initiates the creation of a
    /// publisher session. Likewise, when a node has a reader for a topic that another node is writing, the node
    /// initiates the creation of a subscriber session.
    ///
    /// The publisher node hosts the publisher session servant, which is accessed by the subscriber node through a
    /// PublisherSession proxy. The subscriber node hosts the subscriber session servant, which is accessed by the
    /// publisher node through a SubscriberSession proxy.
    interface Node
    {
        /// Initiate the creation of a publisher session with a node, after the target node has announced a topic
        /// reader for which this node has a corresponding topic writer.
        ///
        /// @param publisher The publisher node initiating the session. The proxy is never null.
        /// @see Lookup::announceTopicReader
        void initiateCreateSession(Node* publisher);

        /// Initiate the creation of a subscriber session with a node, after the target node has announced a topic
        /// writer for which this node has a corresponding topic reader, or after the node has called
        /// Node::initiateCreateSession.
        ///
        /// @param subscriber The subscriber node initiating the session. The proxy is never null.
        /// @param session The subscriber session being created. The proxy is never null.
        /// @param fromRelay Indicates if the session is being created from a relay node.
        void createSession(Node* subscriber, SubscriberSession* session, bool fromRelay);

        /// Confirm the creation of a publisher session with a node.
        ///
        /// @param publisher The publisher node confirming the session. The proxy is never null.
        /// @param session The publisher session being confirmed. The proxy is never null.
        void confirmCreateSession(Node* publisher, PublisherSession* session);
    }

    /// The lookup interface is used by DataStorm nodes to announce their topic readers and writers to other connected
    /// nodes. When multicast is enabled, the lookup interface also broadcasts these announcements.
    /// Each DataStorm node hosts a lookup servant with the identity "DataStorm/Lookup".
    interface Lookup
    {
        /// Announce a topic reader.
        ///
        /// @param topic The name of the topic.
        /// @param subscriber The node reading the topic. The subscriber proxy is never null.
        idempotent void announceTopicReader(string topic, Node* subscriber);

        /// Announce a topic writer.
        ///
        /// @param topic The name of the topic.
        /// @param node The node writing the topic. The proxy is never null.
        idempotent void announceTopicWriter(string topic, Node* node);

        /// Announce a set of topic readers and writers.
        ///
        /// @param readers A sequence of topic names for readers.
        /// @param writers A sequence of topic names for writers.
        /// @param node The node reading or writing the topics. The proxy is never null.
        idempotent void announceTopics(Ice::StringSeq readers, Ice::StringSeq writers, Node* node);

        /// Establish a connection between this node and another node.
        ///
        /// @param node The node initiating the connection. The proxy is never null.
        /// @return A proxy to this node. The proxy is never null.
        Node* createSession(Node* node);
    }
}
