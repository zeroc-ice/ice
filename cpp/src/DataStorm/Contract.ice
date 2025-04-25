// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Identity.ice"
#include "Ice/BuiltinSequences.ice"
#include "DataStorm/SampleEvent.ice"

[["cpp:include:deque"]]

module DataStormContract
{
    /// Defines policies for clearing the data sample history of a reader in response to sample events.
    enum ClearHistoryPolicy
    {
        /// The reader clears its history when a new data sample is added.
        OnAdd,

        /// The reader clears its history when a data sample is removed.
        OnRemove,

        /// The reader clears its history when any data sample event occurs.
        OnAll,

        /// The reader clears its history for all data sample events except for partial update events.
        OnAllExceptPartialUpdate,

        /// The reader never clears its history.
        Never
    }

    dictionary<long, long> LongLongDict;

    /// Represents a data sample, the fundamental unit of data exchanged between DataStorm readers and writers.
    struct DataSample
    {
        /// The unique identifier for the sample.
        long id;

        /// The unique identifier for the associated key.
        /// A negative value (< 0) indicates a key filter.
        long keyId;

        /// The encoded key value, used when keyId < 0 (key filter).
        Ice::ByteSeq keyValue;

        /// The timestamp when the sample was written, in milliseconds since the epoch.
        long timestamp;

        /// An update tag, used for PartialUpdate sample events.
        long tag;

        /// The event type associated with this sample (e.g., Add, Update, PartialUpdate, Remove).
        DataStorm::SampleEvent event;

        /// The payload data of the sample.
        Ice::ByteSeq value;
    }

    // A queue of DataSample
    ["cpp:type:std::deque<DataSample>"] sequence<DataSample> DataSampleSeq;

    /// Represents a collection of data samples produced by a specific writer.
    ["cpp:custom-print"]
    struct DataSamples
    {
        /// The unique identifier for the writer.
        long id;

        /// The sequence of samples produced by the writer.
        DataSampleSeq samples;
    }
    sequence<DataSamples> DataSamplesSeq;

    /// Provides metadata about an element, such as a key, filter, or tag.
    ["cpp:custom-print"]
    struct ElementInfo
    {
        /// The unique identifier for the element.
        /// Negative values indicate filter IDs; positive values indicate key or tag IDs.
        long id;

        /// The name of the element. Empty for key and tag elements.
        string name;

        /// The encoded value of the element.
        Ice::ByteSeq value;
    }
    sequence<ElementInfo> ElementInfoSeq;

    /// Contains metadata about a topic, including its name and associated reader/writer IDs.
    ///
    /// @see Session#announceTopics
    ["cpp:custom-print"]
    struct TopicInfo
    {
        /// The name of the topic.
        string name;

        /// The list of active topic reader or writer IDs.
        ///
        /// - In a publisher session,  the `ids` field contains the active topic writer IDs.
        /// - In a subscriber session,  the `ids` field contains the active topic reader IDs.
        Ice::LongSeq ids;
    }

    /// Represents a sequence of active topics used for transmitting topic information between publisher and subscriber
    /// sessions.
    ///
    /// @see Session#announceTopics
    sequence<TopicInfo> TopicInfoSeq;

    /// Provides detailed information about topic readers and topic writers, including its ID, name, keys, filters,
    /// and tags.
    ///
    /// @see Session#attachTopic
    ["cpp:custom-print"]
    struct TopicSpec
    {
        /// The unique identifier for the topic.
        /// The ID uniquely identifies a topic reader or topic writer within a node.
        long id;

        /// The name of the topic.
        string name;

        /// The topic's keys and filters.
        ElementInfoSeq elements;

        /// The topic update tags.
        ElementInfoSeq tags;
    }

    /// Represents a sample filter that specifies which samples should be sent to a data reader.
    struct FilterInfo
    {
        /// The unique name of the filter, used for identification.
        string name;

        /// The encoded criteria for instantiating the filter.
        Ice::ByteSeq criteria;
    }

    /// Represents the configuration of a data reader or data writer, including optional filters and priorities.
    class ElementConfig(1)
    {
        /// A facet that is used to process the samples when sample filtering is enabled.
        optional(1) string facet;

        /// An optional sample filter associated with the reader. Sample filters are specified on the reader side.
        optional(2) FilterInfo sampleFilter;

        /// An optional name for the reader or writer.
        optional(3) string name;

        /// An optional priority for the writer.
        /// See also the `DataStorm.Topic.Priority` property.
        optional(4) int priority;

        /// An optional sample count, specifying the number of samples queued in the writer or reader sample queue.
        /// See also the `DataStorm.Topic.SampleCount` property.
        optional(10) int sampleCount;

        /// An optional lifetime, specified in milliseconds, representing the maximum time samples are kept in the
        /// writer or reader sample queue. See also the `DataStorm.Topic.SampleLifetime` property.
        optional(11) int sampleLifetime;

        /// An optional clear history policy that determines when the reader or writer sample history is cleared.
        /// See also the `DataStorm.Topic.ClearHistory` property.
        optional(12) ClearHistoryPolicy clearHistory;
    }

    /// Encapsulates the state and configuration data for a data reader or data writer.
    ["cpp:custom-print"]
    struct ElementData
    {
        /// The unique identifier for the data reader or data writer.
        long id;

        /// The configuration settings for the data reader or data writer.
        ElementConfig config;

        /// A mapping of data writer IDs to the last sample IDs received by the data reader.
        ///
        /// - The key represents the data writer ID.
        /// - The value represents the last sample ID received from the corresponding data writer.
        LongLongDict lastIds;
    }
    sequence<ElementData> ElementDataSeq;

    /// Represents detailed information about topic elements, which can be a key or a filter.
    ["cpp:custom-print"]
    struct ElementSpec
    {
        /// A sequence of data readers and writers associated with the key or filter.
        ElementDataSeq elements;

        /// The unique identifier for the key or filter.
        long id;

        /// The name of the filter.
        /// This field is empty if the element is a key.
        string name;

        /// The encoded value of the key or filter.
        Ice::ByteSeq value;

        /// The unique identifier for the key or filter on the peer.
        long peerId;

        /// The name of the filter on the peer.
        /// This field is empty if the element is a key.
        string peerName;
    }
    sequence<ElementSpec> ElementSpecSeq;

    /// Represents an acknowledgment of the attachment of data readers or data writers associated with a key or filter.
    ["cpp:custom-print"]
    struct ElementDataAck
    {
        /// The unique identifier for the data reader or data writer.
        long id;

        /// The configuration settings for the data reader or data writer.
        ElementConfig config;

        /// A mapping of data writer IDs to the last sample IDs received by the data reader.
        ///
        /// - The key represents the data writer ID.
        /// - The value represents the last sample ID received from the corresponding data writer.
        LongLongDict lastIds;

        /// A sequence of samples in the writer's queue, used to initialize the reader.
        ///
        /// - When this struct is sent from a subscriber to a publisher, this field is empty.
        /// - When sent from a publisher to a subscriber, this field contains the queued samples.
        DataSampleSeq samples;

        /// The unique identifier for the peer's data reader or data writer.
        long peerId;
    }
    sequence<ElementDataAck> ElementDataAckSeq;

    /// Represents an acknowledgment of the attachment of an element, which can be a key or a filter.
    ["cpp:custom-print"]
    struct ElementSpecAck
    {
        /// A sequence of acknowledgments for the readers or writers associated with the key or filter.
        ElementDataAckSeq elements;

        /// The unique identifier for the key or filter.
        long id;

        /// The name of the filter.
        /// This field is empty if the element is a key.
        string name;

        /// The encoded value of the key or filter.
        Ice::ByteSeq value;

        /// The unique identifier for the key or filter on the peer.
        long peerId;

        /// The name of the filter on the peer.
        /// This field is empty if the element is a key.
        string peerName;
    }
    sequence<ElementSpecAck> ElementSpecAckSeq;

    /// The base interface for publisher and subscriber sessions.
    ///
    /// This interface specifies the operations for communication between publisher and subscriber sessions.
    ///
    /// @see PublisherSession
    /// @see SubscriberSession
    interface Session
    {
        /// Announces topics to the peer during session establishment or when adding new topics.
        ///
        /// - During session establishment, announces existing topics.
        /// - For established sessions, announces newly added topics.
        ///
        /// A publisher session announces the topics it writes, and a subscriber session announces the topics it reads.
        ///
        /// The receiving peer invokes attachTopic for topics it is interested in.
        ///
        /// @param topics The sequence of topics to announce.
        /// @param initialize Currently unused.
        /// @see attachTopic
        void announceTopics(TopicInfoSeq topics, bool initialize);

        // Attaches a local topic to a remote topic after receiving a topic announcement from the peer.
        ///
        /// This operation is invoked if the session is interested in the announced topic. Which occurs when:
        ///
        /// - The session has a reader for a topic that the peer writes, or
        /// - The session has a writer for a topic that the peer reads.
        ///
        /// @param topic The TopicSpec describing the topic to attach.
        void attachTopic(TopicSpec topic);

        /// Detaches a topic from the session, typically called when the topic is destroyed.
        ///
        /// This operation is invoked by the topic on listener sessions during its destruction.
        ///
        /// @param topicId The unique identifier for the topic to detach.
        void detachTopic(long topicId);

        /// Attaches the specified tags to the subscriber of a topic.
        ///
        /// Tags are used to support partial update samples.
        ///
        /// @param topicId The unique identifier for the topic to which the tags will be attached.
        /// @param tags The sequence of tags to attach, representing the partial update associations.
        /// @param initialize Indicates whether the tags are being attached during session initialization.
        void attachTags(long topicId, ElementInfoSeq tags, bool initialize);

        /// Detaches tags from the session.
        ///
        /// @param topicId The unique identifier for the topic.
        /// @param tags The sequence of tag identifiers to detach.
        void detachTags(long topicId, Ice::LongSeq tags);

        /// Announces elements associated with a topic to the peer.
        ///
        /// This operation informs the peer about new data readers or data writers associated with the specified topic.
        /// The receiving peer will invoke `attachElements` for any elements it is interested in.
        ///
        /// - A publisher session announces its data writers.
        /// - A subscriber session announces its data readers.
        ///
        /// @param topicId The unique identifier for the topic to which the elements belong.
        /// @param elements The sequence of elements to announce, representing the data readers or data writers.
        /// @see attachElements
        void announceElements(long topicId, ElementInfoSeq elements);

        /// Attaches the specified elements to the subscribers of a topic.
        ///
        /// This operation associates the provided elements, such as keys or filters, with the subscribers of the given
        /// topic.
        ///
        /// @param topicId The unique identifier for the topic to which the elements belong.
        /// @param elements The sequence of `ElementSpec` objects representing the elements to attach.
        /// @param initialize Indicates whether the elements are being attached during session initialization.
        void attachElements(long topicId, ElementSpecSeq elements, bool initialize);

        /// Acknowledges the attachment of elements to the session in response to a previous attachElements request.
        ///
        /// This method confirms that the specified elements, such as keys or filters, have been successfully attached
        /// to the session.
        ///
        /// @param topicId The unique identifier for the topic to which the elements belong.
        /// @param elements A sequence of `ElementSpecAck` objects representing the confirmed attachments.
        void attachElementsAck(long topicId, ElementSpecAckSeq elements);

        /// Instructs the peer to detach specific elements associated with a topic.
        ///
        /// This operation is invoked when the specified elements, such as keys or filters, are no longer valid
        /// and should be removed from the peer's session.
        ///
        /// @param topicId The unique identifier for the topic to which the elements belong.
        /// @param elements A sequence of element identifiers representing the keys or filters to detach.
        void detachElements(long topicId, Ice::LongSeq elements);

        /// Initializes the subscriber with the publisher queued samples for a topic during session establishment.
        ///
        /// @param topicId The unique identifier for the topic.
        /// @param samples A sequence of `DataSamples` containing the queued samples to initialize the subscriber.
        void initSamples(long topicId, DataSamplesSeq samples);

        /// Notifies the peer that the session is being disconnected.
        ///
        /// For sessions established through a relay node, this operation is invoked by the relay node if the connection
        /// between the relay node and the target node is lost.
        void disconnected();
    }

    /// The PublisherSession servant is hosted by the publisher node and is accessed by the subscriber node.
    interface PublisherSession extends Session
    {
    }

    /// The SubscriberSession servant is hosted by the subscriber node and is accessed by the publisher node.
    interface SubscriberSession extends Session
    {
        /// Queue a sample with the subscribers of the topic element.
        ///
        /// @param topicId The unique identifier for the topic to which the sample belong.
        /// @param elementId The unique identifier for the element to which the sample belong.
        /// @param sample The sample to queue.
        void s(long topicId, long elementId, DataSample sample);
    }

    /// The reason for the session creation failure.
    enum SessionCreationError
    {
        /// The session is already connected.
        AlreadyConnected,

        /// Node is shutting down.
        NodeShutdown,

        /// A confirmation was received for a session that doesn't exist.
        SessionNotFound,

        /// The session creation failed due to an internal error.
        Internal,
    }

    /// Throws when the session cannot be created.
    exception SessionCreationException
    {
        SessionCreationError error;
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
        /// @throws SessionCreationException Thrown when the session cannot be created.
        /// @see Lookup::announceTopicReader
        ["amd"] void initiateCreateSession(Node* publisher) throws SessionCreationException;

        /// Initiates the creation of a subscriber session with a node. The subscriber node sends this request to a
        /// publisher node in one of the following scenarios:
        ///
        /// - The subscriber has received a topic writer announcement from the publisher and has a matching topic
        /// reader.
        /// - The publisher node has previously sent a initiateCreateSession request.
        ///
        /// The publisher node dispatching this request then sends a confirmCreateSession request to the subscriber node
        /// to continue session establishment. If an active session already exists with the subscriber node, the
        /// request is ignored.
        ///
        /// @param subscriber The subscriber node initiating the session. This proxy is never null.
        /// @param session The subscriber session being created. This proxy is never null.
        /// @param fromRelay Indicates whether the session is being created from a relay node.
        /// @throws SessionCreationException Thrown when the session cannot be created.
        ["amd"] void createSession(Node* subscriber, SubscriberSession* session, bool fromRelay)
            throws SessionCreationException;

        /// Confirm the creation of a publisher session with a node.
        ///
        /// @param publisher The publisher node confirming the session. The proxy is never null.
        /// @param session The publisher session being confirmed. The proxy is never null.
        /// @throws SessionCreationException Thrown when the session cannot be created.
        ["amd"] void confirmCreateSession(Node* publisher, PublisherSession* session) throws SessionCreationException;
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

        /// Establish a connection between this node and the caller node.
        ///
        /// @param node The node initiating the connection. The proxy is never null.
        /// @return A proxy to this node. The proxy is never null.
        Node* createSession(Node* node);
    }
}
