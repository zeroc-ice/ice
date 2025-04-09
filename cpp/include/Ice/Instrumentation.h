// Copyright (c) ZeroC, Inc.

#ifndef ICE_INSTRUMENTATION_H
#define ICE_INSTRUMENTATION_H

#include "ConnectionF.h"
#include "EndpointF.h"
#include "Ice/Context.h"
#include "ObjectAdapterF.h"

#include <cstdint>
#include <memory>
#include <optional>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace Ice
{
    struct Current;
    class ObjectPrx;
}

/// Observers for objects created by the Ice runtime.
namespace Ice::Instrumentation
{
    class Observer;
    class ThreadObserver;
    class ConnectionObserver;
    class DispatchObserver;
    class ChildInvocationObserver;
    class RemoteObserver;
    class CollocatedObserver;
    class InvocationObserver;
    class ObserverUpdater;
    class CommunicatorObserver;

    /// A shared point to an Observer.
    using ObserverPtr = std::shared_ptr<Observer>;

    /// A shared pointer to a ThreadObserver.
    using ThreadObserverPtr = std::shared_ptr<ThreadObserver>;

    /// A shared pointer to a ConnectionObserver.
    using ConnectionObserverPtr = std::shared_ptr<ConnectionObserver>;

    /// A shared pointer to a DispatchObserver.
    using DispatchObserverPtr = std::shared_ptr<DispatchObserver>;

    /// A shared pointer to a ChildInvocationObserver.
    using ChildInvocationObserverPtr = std::shared_ptr<ChildInvocationObserver>;

    /// A shared pointer to a RemoteObserver.
    using RemoteObserverPtr = std::shared_ptr<RemoteObserver>;

    /// A shared pointer to a CollocatedObserver.
    using CollocatedObserverPtr = std::shared_ptr<CollocatedObserver>;

    /// A shared pointer to an InvocationObserver.
    using InvocationObserverPtr = std::shared_ptr<InvocationObserver>;

    /// A shared pointer to an ObserverUpdater.
    using ObserverUpdaterPtr = std::shared_ptr<ObserverUpdater>;

    /// A shared pointer to a CommunicatorObserver.
    using CommunicatorObserverPtr = std::shared_ptr<CommunicatorObserver>;

    /// The thread state enumeration keeps track of the different possible states of Ice threads.
    enum class ThreadState : std::uint8_t
    {
        /// The thread is idle.
        ThreadStateIdle,
        /// The thread is in use performing reads or writes for Ice connections. This state is only for threads from an
        /// Ice thread pool.
        ThreadStateInUseForIO,
        /// The thread is calling user code (servant implementation, AMI callbacks). This state is only for threads from
        /// an Ice thread pool.
        ThreadStateInUseForUser,
        /// The thread is performing other internal activities (DNS lookups, timer callbacks, etc).
        ThreadStateInUseForOther
    };

    /// The state of an Ice connection.
    enum class ConnectionState : std::uint8_t
    {
        /// The connection is being validated.
        ConnectionStateValidating,

        /// The connection is holding the reception of new messages.
        ConnectionStateHolding,

        /// The connection is active and can send and receive messages.
        ConnectionStateActive,

        /// The connection is being gracefully shutdown and waits for the peer to close its end of the connection.
        ConnectionStateClosing,

        /// The connection is closed and waits for potential dispatch to be finished before being destroyed and detached
        /// from the observer.
        ConnectionStateClosed
    };

    /// Represents the base class for Ice observers.
    /// @headerfile Ice/Ice.h
    class Observer
    {
    public:
        virtual ~Observer() = default;

        /// Notifies the observer that an instrumented object was created.
        virtual void attach() = 0;

        /// Notifies the observer that an instrumented object was destroyed.
        virtual void detach() = 0;

        /// Notifies the observer of a failure.
        /// @param exceptionName The name of the exception.
        virtual void failed(const std::string& exceptionName) = 0;
    };

    /// Represents an observer for Ice threads. This can be threads from the Ice thread pool or utility threads used by
    /// the Ice core.
    /// @headerfile Ice/Ice.h
    class ThreadObserver : public virtual Observer
    {
    public:
        /// Notifies the observer of a thread state change.
        /// @param oldState The previous thread state.
        /// @param newState The new thread state.
        virtual void stateChanged(ThreadState oldState, ThreadState newState) = 0;
    };

    /// Represents an observer for Ice connections.
    /// @headerfile Ice/Ice.h
    class ConnectionObserver : public virtual Observer
    {
    public:
        /// Notifies the observer of the number of bytes sent over the connection.
        /// @param num The number of bytes sent.
        virtual void sentBytes(int num) = 0;

        /// Notifies the observer of the number of bytes received over the connection.
        /// @param num The number of bytes received.
        virtual void receivedBytes(int num) = 0;
    };

    /// Represents an observer for dispatches.
    /// @headerfile Ice/Ice.h
    class DispatchObserver : public virtual Observer
    {
    public:
        /// Notifies the observer that the dispatch completed with a user exception.
        virtual void userException() = 0;

        /// Notifies the observer that a reply was sent.
        /// @param size The size of the reply.
        virtual void reply(int size) = 0;
    };

    /// Represents an observer for remote or collocated invocations.
    /// @headerfile Ice/Ice.h
    class ChildInvocationObserver : public virtual Observer
    {
    public:
        /// Notifies the observer that a reply was received.
        /// @param size The size of the reply.
        virtual void reply(int size) = 0;
    };

    /// Represents an observer for remote invocations.
    /// @headerfile Ice/Ice.h
    class RemoteObserver : public virtual ChildInvocationObserver
    {
    };

    /// Represents an observer for collocated invocations.
    /// @headerfile Ice/Ice.h
    class CollocatedObserver : public virtual ChildInvocationObserver
    {
    };

    /// Represents an observer for invocations on proxies. A proxy invocation can either result in a collocated or
    /// remote invocation. If it results in a remote invocation, a sub-observer is requested for the remote invocation.
    /// @headerfile Ice/Ice.h
    class InvocationObserver : public virtual Observer
    {
    public:
        /// Notifies the observer that an invocation was retried.
        virtual void retried() = 0;

        /// Notifies the observer that a user exception was received.
        virtual void userException() = 0;

        /// Gets a remote observer for this invocation.
        /// @param con The connection information.
        /// @param endpt The connection endpoint.
        /// @param requestId The request ID.
        /// @param size The size of the invocation.
        /// @return The observer to instrument the remote invocation.
        virtual RemoteObserverPtr
        getRemoteObserver(const ConnectionInfoPtr& con, const EndpointPtr& endpt, int requestId, int size) = 0;

        /// Gets a collocated observer for this invocation.
        /// @param adapter The object adapter hosting the collocated Ice object.
        /// @param requestId The request ID.
        /// @param size The size of the invocation.
        /// @return The observer to instrument the collocated invocation.
        virtual CollocatedObserverPtr
        getCollocatedObserver(const ObjectAdapterPtr& adapter, int requestId, int size) = 0;
    };

    /// The observer updater interface. This interface is implemented by the Ice runtime and an instance of this
    /// interface is provided by the Ice communicator on initialization to the CommunicatorObserver object set
    /// with the communicator initialization data. The Ice communicator calls CommunicatorObserver::setObserverUpdater
    /// to provide the observer updater. This interface can be used by add-ins implementing the CommunicatorObserver
    /// interface to update the observers of connections and threads.
    /// @headerfile Ice/Ice.h
    class ObserverUpdater
    {
    public:
        virtual ~ObserverUpdater() = default;

        /// Updates connection observers associated with each of the Ice connection from the communicator and its object
        /// adapters.
        /// When called, this method goes through all the connections and for each connection
        /// CommunicatorObserver::getConnectionObserver is called. The implementation of getConnectionObserver
        /// has the possibility to return an updated observer if necessary.
        virtual void updateConnectionObservers() = 0;

        /// Updates thread observers associated with each of the Ice thread from the communicator and its object
        /// adapters. When called, this method goes through all the threads and for each thread
        /// CommunicatorObserver::getThreadObserver is called. The implementation of getThreadObserver has the
        /// possibility to return an updated observer if necessary.
        virtual void updateThreadObservers() = 0;
    };

    /// The communicator observer interface used by the Ice runtime to obtain and update observers for its observable
    /// objects. This interface should be implemented by add-ins that wish to observe Ice objects in order to collect
    /// statistics. An instance of this interface can be provided to the Ice run-time through the Ice communicator
    /// initialization data.
    /// @headerfile Ice/Ice.h
    class CommunicatorObserver
    {
    public:
        virtual ~CommunicatorObserver() = default;

        /// Gets an observer for the given endpoint information and connector. The Ice runtime calls this method for
        /// each connection establishment attempt.
        /// @param endpt The endpoint.
        /// @param connector The description of the connector. For IP transports, this is typically the IP address to
        /// connect to.
        /// @return The observer to instrument the connection establishment.
        virtual ObserverPtr
        getConnectionEstablishmentObserver(const EndpointPtr& endpt, const std::string& connector) = 0;

        /// Gets an observer for the given endpoint information. The Ice runtime calls this method to resolve an
        /// endpoint and obtain the list of connectors. For IP endpoints, this typically involves doing a DNS lookup to
        /// obtain the IP addresses associated with the DNS name.
        /// @param endpt The endpoint.
        /// @return The observer to instrument the endpoint lookup.
        virtual ObserverPtr getEndpointLookupObserver(const EndpointPtr& endpt) = 0;

        /// Gets an observer for the given connection. The Ice run-time calls this method for each new connection and
        /// for all the Ice communicator connections when ObserverUpdater::updateConnectionObservers is called.
        /// @param c The connection information.
        /// @param e The connection endpoint.
        /// @param s The state of the connection.
        /// @param o The old connection observer if one is already set or a nullptr reference otherwise.
        /// @return The connection observer to instrument the connection.
        virtual ConnectionObserverPtr getConnectionObserver(
            const ConnectionInfoPtr& c,
            const EndpointPtr& e,
            ConnectionState s,
            const ConnectionObserverPtr& o) = 0;

        /// Gets a thread observer for the given thread. The Ice runtime calls this method for each new thread and for
        /// all the Ice communicator threads when ObserverUpdater::updateThreadObservers is called.
        /// @param parent The parent of the thread.
        /// @param id The ID of the thread to observe.
        /// @param s The state of the thread.
        /// @param o The old thread observer if one is already set or a null reference otherwise.
        /// @return The thread observer to instrument the thread.
        virtual ThreadObserverPtr getThreadObserver(
            const std::string& parent,
            const std::string& id,
            ThreadState s,
            const ThreadObserverPtr& o) = 0;

        /// Gets an invocation observer for the given invocation. The Ice runtime calls this method for each new
        /// invocation on a proxy.
        /// @param prx The proxy used for the invocation.
        /// @param operation The name of the operation.
        /// @param ctx The context specified by the user.
        /// @return The invocation observer to instrument the invocation.
        virtual InvocationObserverPtr
        getInvocationObserver(const std::optional<ObjectPrx>& prx, std::string_view operation, const Context& ctx) = 0;

        /// Gets a dispatch observer for the given dispatch. The Ice runtime calls this method each time it receives an
        /// incoming invocation to be dispatched for an Ice object.
        /// @param c The current object as provided to the Ice servant dispatching the invocation.
        /// @param size The size of the dispatch.
        /// @return The dispatch observer to instrument the dispatch.
        virtual DispatchObserverPtr getDispatchObserver(const Current& c, int size) = 0;

        /// Sets the observer updater. The Ice runtime calls this method when the communicator is initialized. The
        /// add-in implementing this interface can use this object to get the Ice run-time to re-obtain observers for
        /// observed objects.
        /// @param updater The observer updater object.
        virtual void setObserverUpdater(const ObserverUpdaterPtr& updater) = 0;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

#endif
