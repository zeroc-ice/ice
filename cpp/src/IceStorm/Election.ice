// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h"]]

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>
#include <IceStorm/SubscriberRecord.ice>
#include <IceStorm/LLURecord.ice>

module IceStormElection
{

/** The contents of topic. */
struct TopicContent
{
    /** The topic identity. */
    Ice::Identity id;
    /** The topic subscribers. */
    IceStorm::SubscriberRecordSeq records;
};

/** A sequence of topic content. */
sequence<TopicContent> TopicContentSeq;

/** Thrown if an observer detects an inconsistency. */
exception ObserverInconsistencyException
{
    /** The reason for the inconsistency. */
    string reason;
};

/** The replica observer. */
interface ReplicaObserver
{
    /**
     *
     * Initialize the observer.
     *
     * @param llu The last log update seen by the master.
     *
     * @param content The topic content.
     *
     * @throws ObserverInconsistencyException Raised if an
     * inconsisency was detected.
     *
     **/
    void init(LogUpdate llu, TopicContentSeq content)
        throws ObserverInconsistencyException;

    /**
     *
     * Create the topic with the given name.
     *
     * @param llu The log update token.
     *
     * @param name The topic name.
     *
     * @throws ObserverInconsistencyException Raised if an
     * inconsisency was detected.
     *
     **/
    void createTopic(LogUpdate llu, string name)
        throws ObserverInconsistencyException;

    /**
     *
     * Destroy the topic with the given name.
     *
     * @param llu The log update token.
     *
     * @param name The topic name.
     *
     * @throws ObserverInconsistencyException Raised if an
     * inconsisency was detected.
     *
     **/
    void destroyTopic(LogUpdate llu, string name)
        throws ObserverInconsistencyException;

    /**
     *
     * Add a subscriber to a topic.
     *
     * @param llu The log update token.
     *
     * @param topic The topic name to which to add the subscriber.
     *
     * @param record The subscriber information.
     *
     * @throws ObserverInconsistencyException Raised if an
     * inconsisency was detected.
     *
     **/
    void addSubscriber(LogUpdate llu, string topic, IceStorm::SubscriberRecord record)
        throws ObserverInconsistencyException;

    /**
     *
     * Remove a subscriber from a topic.
     *
     * @param llu The log update token.
     *
     * @param name The topic name.
     *
     * @param subscribers The identities of the subscribers to remove.
     *
     * @throws ObserverInconsistencyException Raised if an
     * inconsisency was detected.
     *
     **/
    void removeSubscriber(LogUpdate llu, string topic, Ice::IdentitySeq subscribers)
        throws ObserverInconsistencyException;
};

/** Interface used to sync topics. */
interface TopicManagerSync
{
    /**
     * Retrieve the topic content.
     *
     * @param llu The last log update token.
     *
     * @param content The topic content.
     *
     **/
    void getContent(out LogUpdate llu, out TopicContentSeq content);
};

/** The node state. */
enum NodeState
{
    /** The node is inactive and awaiting an election. */
    NodeStateInactive,
    /** The node is electing a leader. */
    NodeStateElection,
    /** The replica group is reorganizing. */
    NodeStateReorganization,
    /** The replica group is active & replicating. */
    NodeStateNormal
};

/** Forward declaration. */
interface Node;

/** All nodes in the replication group. */
struct NodeInfo
{
    /** The identity of the node. */
    int id;
    /** The node proxy. */
    Node* n;
};

/** A sequence of node info. */
sequence<NodeInfo> NodeInfoSeq;

/** The group info. */
struct GroupInfo
{
    /** The identity of the node. */
    int id;
    /** The last known log update for this node. */
    LogUpdate llu;
};
/** A sequence of group info. */
sequence<GroupInfo> GroupInfoSeq;

struct QueryInfo
{
    /** The node id. */
    int id;

    /** The nodes coordinator. */
    int coord;

    /** The nodes group name. */
    string group;

    /** The replica the node is managing. */
    Object* replica;

    /** The node state. */
    NodeState state;

    /** The sequence of nodes in this nodes group. */
    GroupInfoSeq up;

    /** The highest priority node that this node has seen. */
    int max;
};

/** A replica node. */
interface Node
{
    /**
     *
     * Invite the node into a group with the given coordinator and
     * group name.
     *
     * @param gn The group name.
     *
     * @param j The group coordinator.
     *
     **/
    void invitation(int j, string gn);

    /**
     *
     * Call from the group coordinator to a node to inform the node
     * that the replica group is active.
     *
     * @param j The group coordinator.
     *
     * @param gn The group name.
     *
     * @param coordinator The proxy to the coordinator.
     *
     * @param max The highest priority node seen by this replica
     * group.
     *
     * @param generation The current generation count.
     */
    void ready(int j, string gn, Object* coordinator, int max, long generation);

    /**
     *
     * Called to accept an invitation into
     * the given group.
     *
     * @param j The id of the node accepting the invitation.
     *
     * @param observer The observer.
     *
     * @param gn The group name.
     *
     * @param forwardedInvites The ids of the nodes to which
     * invitations were forwarded.
     *
     * @param llu The last log update for the given node.
     *
     * @param max The highest priority node seen by this replica
     * group.
     *
     **/
    void accept(int j, string gn, Ice::IntSeq forwardedInvites, Object* observer, LogUpdate llu, int max);

    /**
     *
     * Determine if this node is a coordinator.
     *
     * @return True if the node is a coordinator, false otherwise.
     **/
    ["cpp:const"] idempotent bool areYouCoordinator();

    /**
     *
     * Determine if the node is a member of the given group with the
     * given coordinator.
     *
     * @param gn The group name.
     *
     * @param j The group coordinator.
     *
     * @return True if the node is a member, false otherwise.
     *
     **/
    ["cpp:const"] idempotent bool areYouThere(string gn, int j);

    /**
     *
     * Get the sync object for the replica hosted by this node.
     *
     * @return The sync object.
     *
     **/
    ["cpp:const"] idempotent Object* sync();

    /**
     *
     * Get the replication group information.
     *
     * @return The set of configured nodes and the associated
     * priority.
     *
     **/
    ["cpp:const"] idempotent NodeInfoSeq nodes();

    /**
     *
     * Get the query information for the given node.
     *
     * @return The query information.
     *
     **/
    ["cpp:const"] idempotent QueryInfo query();
};

};
