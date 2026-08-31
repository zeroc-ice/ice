// Copyright (c) ZeroC, Inc.

#pragma once

// The part of the DataStorm session protocol this test drives, restated here so the test peer can speak it
// without linking the DataStorm library - the library carries its own copy of the generated contract, and two
// copies in one process compete to supply the exception factories.
//
// Only what the test invokes or implements is declared. The names, the operation signatures and the order of the
// SessionCreationError enumerators have to match cpp/src/DataStorm/Contract.ice; a mismatch shows up as a
// marshaling failure, which is the intended signal.
module DataStormContract
{
    enum SessionCreationError
    {
        AlreadyConnected,
        NodeShutdown,
        SessionNotFound,
        Superseded,
        Internal,
    }

    exception SessionCreationException
    {
        SessionCreationError error;
    }

    interface Session
    {
        void disconnected(long handshakeId);
    }

    interface SubscriberSession extends Session {}

    interface PublisherSession extends Session {}

    interface Node
    {
        void initiateCreateSession(Node* publisher) throws SessionCreationException;

        void createSession(Node* subscriber, SubscriberSession* session, bool fromRelay, long handshakeId)
            throws SessionCreationException;

        void confirmCreateSession(Node* publisher, PublisherSession* session, long handshakeId)
            throws SessionCreationException;
    }

    interface Lookup
    {
        idempotent void announceTopicReader(string topic, Node* subscriber);
    }
}
