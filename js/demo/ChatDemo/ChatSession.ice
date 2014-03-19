// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Chat Demo is licensed to you under the terms described
// in the CHAT_DEMO_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CHAT_SESSION_ICE
#define CHAT_SESSION_ICE

#include <Ice/BuiltinSequences.ice>
#include <Glacier2/Session.ice>
#include <Chat.ice>

module Chat
{

/**
 *
 * The ChatRoomCallback interface is the interface that clients implement
 * as their callback object.
 *
 * The server calls operations of this interface to communicate
 * with connected clients.
 *
 **/
interface ChatRoomCallback
{
    /**
     *
     * The server invokes this operation when the client sets the callback
     * for a session. This provides the client with the initial list of users
     * currently in the chat room.
     *
     * @param users The names of users currently in the chat room.
     *
     **/
    void init(Ice::StringSeq users);

    /**
     *
     * The server invokes this operation to deliver a message
     * that was sent to the chat room.
     *
     * @param name The name of the user that send the message.
     *
     * @param message The contents of the message.
     *
     * @param timestamp The time at which the message was sent.
     *
     **/
    void send(long timestamp, string name, string message);

    /**
     *
     * The server invokes this operation when a user joins
     * the chat room.
     *
     * @param name The name of the user that joined the chat room.
     *
     * @param timestamp The time at which the user joined the chat room.
     *
     **/
    void join(long timestamp, string name);

    /**
     *
     * The servers invokes this operation when a user leaves
     * the chat room.
     *
     * @param name The name of the user that left the chat room.
     *
     * @param timestamp The time at which the user left the chat room.
     *
     **/
    void leave(long timestamp, string name);
};

/**
 *
 * A ChatSession is a custom Glacier2::Session for clients that use
 * Glacier2 and support callbacks (C++, Java, and .NET clients).
 *
 * @see Glacier2::Session
 *
 **/
interface ChatSession extends Glacier2::Session
{
    /**
     *
     * The setCallback operation is called by clients to set the
     * callback used to receive notification of activity in the
     * room. Clients receive notifications as soon as they call this
     * operation (before setCallback returns).
     *
     * The first callback made by the server is a call to
     * ChatRoomCallback::init, which delivers the current list of
     * users to the client.
     *
     * @param cb The callback the server uses to deliver notifications.
     *
     * @see ChatRoomCallback
     *
     **/
    void setCallback(ChatRoomCallback* cb);

    /**
     *
     * Send a message to the chat room.
     *
     * @param message The message to be sent.
     *
     * @return The time at which the message is sent.
     *
     * @throws InvalidMessageException should the message be invalid.
     *
     **/
    long send(string message) throws InvalidMessageException;
};

};

#endif
