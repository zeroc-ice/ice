// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STORM_INTERNAL_ICE
#define ICE_STORM_INTERNAL_ICE

#include <IceStorm/IceStorm.ice>
#include <Ice/Current.ice>

module IceStorm
{

/**
 *
 * A sequence of bytes.
 *
 **/
sequence<byte> ByteSeq;

/**
 *
 * Mirror datastructure of Ice::Current::Context.
 *
 **/
dictionary<string, string> ContextData;

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
     * Forward an event
     *
     * @param op The operation name.
     *
     * @param idempotent Flag indicating whether the operation is
     * idempotent.
     *
     * @param data The encoded for the operation in parameters.
     *
     * @param context The Ice::Current::Context data from the
     * originating request.
     *
     **/
    void forward(string op, Ice::OperationMode mode, ByteSeq data, ContextData context);
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
