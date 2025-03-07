// Copyright (c) ZeroC, Inc.
    
#pragma once
    
[["cpp:header-ext:h"]]
    
#include "IceStorm/IceStorm.ice"
#include "Election.ice"
#include "Ice/Context.ice"
#include "Ice/OperationMode.ice"
#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"
    
[["cpp:include:deque"]]
    
module IceStorm
{
    /// The event data.
    struct EventData
    {
        /// The operation name.
        string op;
        /// The operation mode.
        Ice::OperationMode mode;
        /// The encoded data for the operation's input parameters.
        Ice::ByteSeq data;
        /// The Ice::Current::Context data from the originating request.
        Ice::Context context;
    }
<<<<<<< Updated upstream

    /// A sequence of EventData.
    ["cpp:type:std::deque<IceStorm::EventData>"] sequence<EventData> EventDataSeq;

=======
        
    /// A sequence of EventData.
    ["cpp:type:std::deque<IceStorm::EventData>"] sequence<EventData> EventDataSeq;
        
>>>>>>> Stashed changes
    /// The TopicLink interface. This is used to forward events between federated Topic instances.
    /// @see TopicInternal
    interface TopicLink
    {
        /// Forward a sequence of events.
        /// @param events The events to forward.
        void forward(EventDataSeq events);
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    /// Thrown if the reap call would block.
    exception ReapWouldBlock
    {
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    /// Internal operations for a topic.
    /// @see Topic
    interface TopicInternal extends Topic
    {
        /// Retrieve a proxy to the TopicLink interface.
        /// @return The TopicLink for the Topic.
        idempotent TopicLink* getLinkProxy();
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        /// Reap the given identities.
        /// @param id The sequence of identities.
        /// @throws ReapWouldBlock Raised if the reap call would block.
        void reap(Ice::IdentitySeq id) throws ReapWouldBlock;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    /// Internal operations for a topic manager.
    /// @see TopicManager
    interface TopicManagerInternal extends TopicManager
    {
        /// Return the replica node proxy for this topic manager.
        /// @return The replica proxy, or null if this instance is not replicated.
        ["cpp:const"] idempotent IceStormElection::Node* getReplicaNode();
    }
} // End module IceStorm
