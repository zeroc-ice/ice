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

    using ObserverPtr = std::shared_ptr<Observer>;
    using ThreadObserverPtr = std::shared_ptr<ThreadObserver>;
    using ConnectionObserverPtr = std::shared_ptr<ConnectionObserver>;
    using DispatchObserverPtr = std::shared_ptr<DispatchObserver>;
    using ChildInvocationObserverPtr = std::shared_ptr<ChildInvocationObserver>;
    using RemoteObserverPtr = std::shared_ptr<RemoteObserver>;
    using CollocatedObserverPtr = std::shared_ptr<CollocatedObserver>;
    using InvocationObserverPtr = std::shared_ptr<InvocationObserver>;
    using ObserverUpdaterPtr = std::shared_ptr<ObserverUpdater>;
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

    /// The object observer interface used by instrumented objects to notify the observer of their existence.
    /// @headerfile Ice/Ice.h
    class Observer
    {
    public:
        virtual ~Observer() = default;

        /// This method is called when the instrumented object is created or when the observer is attached to an
        /// existing object.
        virtual void attach() = 0;

        /// This method is called when the instrumented object is destroyed and as a result the observer detached from
        /// the object.
        virtual void detach() = 0;

        /// Notification of a failure.
        /// @param exceptionName The name of the exception.
        virtual void failed(const std::string& exceptionName) = 0;
    };

    /// The thread observer interface to instrument Ice threads. This can be threads from the Ice thread pool or utility
    /// threads used by the Ice core.
    /// @headerfile Ice/Ice.h
    class ThreadObserver : public virtual Observer
    {
    public:
        /// Notification of thread state change.
        /// @param oldState The previous thread state.
        /// @param newState The new thread state.
        virtual void stateChanged(ThreadState oldState, ThreadState newState) = 0;
    };

    /// The connection observer interface to instrument Ice connections.
    /// @headerfile Ice/Ice.h
    class ConnectionObserver : public virtual Observer
    {
    public:
        /// Notification of sent bytes over the connection.
        /// @param num The number of bytes sent.
        virtual void sentBytes(int num) = 0;

        /// Notification of received bytes over the connection.
        /// @param num The number of bytes received.
        virtual void receivedBytes(int num) = 0;
    };

    /// The dispatch observer to instrument servant dispatch.
    /// @headerfile Ice/Ice.h
    class DispatchObserver : public virtual Observer
    {
    public:
        /// Notification of a user exception.
        virtual void userException() = 0;

        /// Reply notification.
        /// @param size The size of the reply.
        virtual void reply(int size) = 0;
    };

    /// The child invocation observer to instrument remote or collocated invocations.
    /// @headerfile Ice/Ice.h
    class ChildInvocationObserver : public virtual Observer
    {
    public:
        /// Reply notification.
        /// @param size The size of the reply.
        virtual void reply(int size) = 0;
    };

    /// The remote observer to instrument invocations that are sent over the wire.
    /// @headerfile Ice/Ice.h
    class RemoteObserver : public virtual ChildInvocationObserver
    {
    };

    /// The collocated observer to instrument invocations that are collocated.
    /// @headerfile Ice/Ice.h
    class CollocatedObserver : public virtual ChildInvocationObserver
    {
    };

    /// The invocation observer to instrument invocations on proxies. A proxy invocation can either result in a
    /// collocated or remote invocation. If it results in a remote invocation, a sub-observer is requested for the
    /// remote invocation.
    /// @headerfile Ice/Ice.h
    class InvocationObserver : public virtual Observer
    {
    public:
        /// Notification of the invocation being retried.
        virtual void retried() = 0;

        /// Notification of a user exception.
        virtual void userException() = 0;

        /// Get a remote observer for this invocation.
        /// @param con The connection information.
        /// @param endpt The connection endpoint.
        /// @param requestId The ID of the invocation.
        /// @param size The size of the invocation.
        /// @return The observer to instrument the remote invocation.
        virtual RemoteObserverPtr
        getRemoteObserver(const ConnectionInfoPtr& con, const EndpointPtr& endpt, int requestId, int size) = 0;

        /// Get a collocated observer for this invocation.
        /// @param adapter The object adapter hosting the collocated Ice object.
        /// @param requestId The ID of the invocation.
        /// @param size The size of the invocation.
        /// @return The observer to instrument the collocated invocation.
        virtual CollocatedObserverPtr
        getCollocatedObserver(const ObjectAdapterPtr& adapter, int requestId, int size) = 0;
    };

    /// The observer updater interface. This interface is implemented by the Ice run-time and an instance of this
    /// interface is provided by the Ice communicator on initialization to the {@link CommunicatorObserver} object set
    /// with the communicator initialization data. The Ice communicator calls {@link
    /// CommunicatorObserver#setObserverUpdater} to provide the observer updater. This interface can be used by add-ins
    /// implementing the {@link CommunicatorObserver} interface to update the observers of connections and threads.
    /// @headerfile Ice/Ice.h
    class ObserverUpdater
    {
    public:
        virtual ~ObserverUpdater() = default;

        /// Update connection observers associated with each of the Ice connection from the communicator and its object
        /// adapters.
        /// When called, this method goes through all the connections and for each connection
        /// {@link CommunicatorObserver#getConnectionObserver} is called. The implementation of getConnectionObserver
        /// has the possibility to return an updated observer if necessary.
        virtual void updateConnectionObservers() = 0;

        /// Update thread observers associated with each of the Ice thread from the communicator and its object
        /// adapters. When called, this method goes through all the threads and for each thread
        /// {@link CommunicatorObserver#getThreadObserver} is called. The implementation of getThreadObserver has the
        /// possibility to return an updated observer if necessary.
        virtual void updateThreadObservers() = 0;
    };

    /// The communicator observer interface used by the Ice run-time to obtain and update observers for its observable
    /// objects. This interface should be implemented by add-ins that wish to observe Ice objects in order to collect
    /// statistics. An instance of this interface can be provided to the Ice run-time through the Ice communicator
    /// initialization data.
    /// @headerfile Ice/Ice.h
    class CommunicatorObserver
    {
    public:
        virtual ~CommunicatorObserver() = default;

        /// This method should return an observer for the given endpoint information and connector. The Ice run-time
        /// calls this method for each connection establishment attempt.
        /// @param endpt The endpoint.
        /// @param connector The description of the connector. For IP transports, this is typically the IP address to
        /// connect to.
        /// @return The observer to instrument the connection establishment.
        virtual ObserverPtr
        getConnectionEstablishmentObserver(const EndpointPtr& endpt, const std::string& connector) = 0;

        /// This method should return an observer for the given endpoint information. The Ice run-time calls this method
        /// to resolve an endpoint and obtain the list of connectors. For IP endpoints, this typically involves doing a
        /// DNS lookup to obtain the IP addresses associated with the DNS name.
        /// @param endpt The endpoint.
        /// @return The observer to instrument the endpoint lookup.
        virtual ObserverPtr getEndpointLookupObserver(const EndpointPtr& endpt) = 0;

        /// This method should return a connection observer for the given connection. The Ice run-time calls this method
        /// for each new connection and for all the Ice communicator connections when
        /// {@link ObserverUpdater#updateConnectionObservers} is called.
        /// @param c The connection information.
        /// @param e The connection endpoint.
        /// @param s The state of the connection.
        /// @param o The old connection observer if one is already set or a null reference otherwise.
        /// @return The connection observer to instrument the connection.
        virtual ConnectionObserverPtr getConnectionObserver(
            const ConnectionInfoPtr& c,
            const EndpointPtr& e,
            ConnectionState s,
            const ConnectionObserverPtr& o) = 0;

        /// This method should return a thread observer for the given thread. The Ice run-time calls this method for
        /// each new thread and for all the Ice communicator threads when {@link ObserverUpdater#updateThreadObservers}
        /// is called.
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

        /// This method should return an invocation observer for the given invocation. The Ice run-time calls this
        /// method for each new invocation on a proxy.
        /// @param prx The proxy used for the invocation.
        /// @param operation The name of the operation.
        /// @param ctx The context specified by the user.
        /// @return The invocation observer to instrument the invocation.
        virtual InvocationObserverPtr
        getInvocationObserver(const std::optional<ObjectPrx>& prx, std::string_view operation, const Context& ctx) = 0;

        /// This method should return a dispatch observer for the given dispatch. The Ice run-time calls this method
        /// each time it receives an incoming invocation to be dispatched for an Ice object.
        /// @param c The current object as provided to the Ice servant dispatching the invocation.
        /// @param size The size of the dispatch.
        /// @return The dispatch observer to instrument the dispatch.
        virtual DispatchObserverPtr getDispatchObserver(const Current& c, int size) = 0;

        /// The Ice run-time calls this method when the communicator is initialized. The add-in implementing this
        /// interface can use this object to get the Ice run-time to re-obtain observers for observed objects.
        /// @param updater The observer updater object.
        virtual void setObserverUpdater(const ObserverUpdaterPtr& updater) = 0;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

#endif
