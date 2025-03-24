// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_INSTRUMENTATION_H_
#define GLACIER2_INSTRUMENTATION_H_

#include "Ice/Instrumentation.h"

#include <memory>

namespace Glacier2::Instrumentation
{
    class SessionObserver : public virtual Ice::Instrumentation::Observer
    {
    public:
        /// Notification of a forwarded request. This also implies removing the event from the queue.
        /// @param client True if client request, false if server request.
        virtual void forwarded(bool client) = 0;

        /// Notification of a routing table size change.
        /// @param delta The size adjustment.
        virtual void routingTableSize(int delta) = 0;
    };

    /// The ObserverUpdater interface is implemented by Glacier2 and an instance of this interface is provided on
    /// initialization to the RouterObserver object.
    /// This interface can be used by add-ins implementing the RouterObserver interface to update the observers of
    /// observed objects.
    class ObserverUpdater
    {
    public:
        virtual ~ObserverUpdater() = default;

        /// Update the router sessions.
        /// When called, this method goes through all the sessions and for each session
        /// RouterObserver::getSessionObserver is called. The implementation of getSessionObserver has the
        /// possibility to return an updated observer if necessary.
        virtual void updateSessionObservers() = 0;
    };

    /// The router observer interface used by Glacier2 to obtain and update observers for its observable objects.
    /// This interface should be implemented by add-ins that wish to observe Glacier2 objects in order to collect
    /// statistics.
    class RouterObserver
    {
    public:
        virtual ~RouterObserver() = default;

        /// This method should return an observer for the given session.
        /// @param id The id of the session (the user id or the SSL DN).
        /// @param con The connection associated to the session.
        /// @param routingTableSize The size of the routing table for this session.
        /// @param old The previous observer, only set when updating an existing observer.
        virtual std::shared_ptr<::Glacier2::Instrumentation::SessionObserver> getSessionObserver(
            const std::string& id,
            const Ice::ConnectionPtr& con,
            int routingTableSize,
            const std::shared_ptr<SessionObserver>& old) = 0;

        /// Glacier2 calls this method on initialization. The add-in implementing this interface can use this object
        /// to get Glacier2 to re-obtain observers for topics and subscribers.
        /// @param updater The observer updater object.
        virtual void setObserverUpdater(const std::shared_ptr<ObserverUpdater>& updater) = 0;
    };

    using SessionObserverPtr = std::shared_ptr<SessionObserver>;
    using ObserverUpdaterPtr = std::shared_ptr<ObserverUpdater>;
    using RouterObserverPtr = std::shared_ptr<RouterObserver>;
}

#endif
