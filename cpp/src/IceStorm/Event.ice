// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_EVENT_ICE
#define ICE_STORM_EVENT_ICE

#include <Ice/Current.ice>
#include <Ice/BuiltinSequences.ice>

module IceStorm
{

/**
 *
 * The event data.
 *
 **/
class Event
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

/** A sequence of Event. */
sequence<Event> EventSeq;

};

#endif
