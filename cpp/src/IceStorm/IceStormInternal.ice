// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_INTERNAL_ICE
#define ICE_STORM_INTERNAL_ICE

#include <IceStorm/IceStorm.ice>
#include <Ice/Current.ice>
#include <Ice/BuiltinSequences.ice>

module IceStorm
{

/**
 *
 * The event data.
 *
 **/
["cpp:class"] struct EventData
{
    /** The operation name. */
    string op;
    /** The operation mode. */
    Ice::OperationMode mode;
     /** The encoded data for the operation's input parameters. */
    Ice::ByteSeq data;
    /** The Ice::Current::Context data from the originating request. */
    Ice::Context context;
};

/** A sequence of EventData. */
sequence<EventData> EventDataSeq;

/**
 *
 * The TopicLink interface. This is used to forward events between
 * federated Topic instances.
 *
 * @see TopicInternal
 *
 **/
interface TopicLink
{
    /**
     *
     * Forward a sequence of events.
     *
     * @param events The events to forward.
     *
     **/
    ["ami"] void forward(EventDataSeq events);
};

/**
 *
 * Internal operations for a topic.
 *
 * @see Topic
 *
 **/
interface TopicInternal extends Topic
{
    /**
     *
     * Retrieve a proxy to the TopicLink interface.
     *
     * @return The TopicLink for the Topic.
     *
     **/
    idempotent TopicLink* getLinkProxy();
};

}; // End module IceStorm

#endif
