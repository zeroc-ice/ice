// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;

namespace ZeroC.Ice.Instrumentation
{
    /// <summary>The child invocation observer to instrument remote or collocated invocations.
    /// </summary>
    public interface IChildInvocationObserver : IObserver
    {
        /// <summary>Reply notification.</summary>
        /// <param name="size">The size of the reply in bytes.</param>
        void Reply(int size);
    }

    /// <summary>The communicator observer interface used by the Ice run-time to obtain and update observers for its
    /// observable objects. This interface should be implemented by plug-ins that wish to observe Ice objects in order
    /// to collect statistics. An instance of this interface can be provided to the Ice run-time through the Ice
    /// communicator constructor.</summary>
    public interface ICommunicatorObserver
    {
        /// <summary>This method should return an observer for the given endpoint information.
        /// The Ice run-time calls this method for each connection establishment attempt.</summary>
        /// <param name="endpoint">The endpoint.</param>
        /// <param name="description">The description of the connection target. For IP transports, this is typically
        /// the IP address to connect to.</param>
        /// <returns>The observer to instrument the connection establishment.</returns>
        IObserver? GetConnectionEstablishmentObserver(Endpoint endpoint, string description);

        /// <summary>This method should return a connection observer for the given connection. The Ice run-time calls
        /// this method for each new connection and for all the Ice communicator connections when
        /// ObserverUpdater.UpdateConnectionObservers is called.</summary>
        /// <param name="connection">The connection.</param>
        /// <param name="connectionState">The state of the connection.</param>
        /// <param name="oldObserver">The old connection observer if one is already set or a null reference otherwise.
        /// </param>
        /// <returns>The connection observer to instrument the connection.</returns>
        IConnectionObserver? GetConnectionObserver(
            Connection connection,
            ConnectionState connectionState,
            IConnectionObserver? oldObserver);

        /// <summary>This method should return a dispatch observer for the given dispatch. The Ice run-time calls this
        /// method each time it receives an incoming request to be dispatched for an Ice object.</summary>
        /// <param name="current">The current object as provided to the Ice servant dispatching the request.</param>
        /// <param name="requestId">The request ID of the request being dispatched.</param>
        /// <param name="size">The size of the dispatch.</param>
        /// <returns>The dispatch observer to instrument the dispatch.</returns>
        IDispatchObserver? GetDispatchObserver(Current current, long requestId, int size);

        /// <summary>This method should return an observer for the given endpoint information. The Ice run-time calls
        /// this method to resolve an endpoint. For IP endpoints, this typically involves doing a DNS lookup to obtain
        /// the IP addresses associated with the DNS name.</summary>
        /// <param name="endpoint">The endpoint.</param>
        /// <returns>The observer to instrument the endpoint lookup.</returns>
        IObserver? GetEndpointLookupObserver(Endpoint endpoint);

        /// <summary>This method should return an invocation observer for the given invocation. The Ice run-time calls
        /// this method for each invocation on a proxy.</summary>
        /// <param name="prx">The proxy used for the invocation.</param>
        /// <param name="operation">The name of the invocation.</param>
        /// <param name="context">The context specified by the user.</param>
        /// <returns>The invocation observer to instrument the invocation.</returns>
        IInvocationObserver? GetInvocationObserver(
            IObjectPrx prx,
            string operation,
            IReadOnlyDictionary<string, string> context);

        /// <summary>The Ice run-time calls this method when the communicator is initialized. The add-in implementing
        /// this interface can use this object to get the Ice run-time to re-obtain observers for observed objects.
        /// </summary>
        /// <param name="updater">The observer updater object.</param>
        void SetObserverUpdater(IObserverUpdater? updater);
    }

    /// <summary>The connection observer interface to instrument Ice connections.</summary>
    public interface IConnectionObserver : IObserver
    {
        /// <summary>Notification of received bytes over the connection.</summary>
        /// <param name="num">The number of bytes received.</param>
        void ReceivedBytes(int num);

        /// <summary>Notification of sent bytes over the connection.</summary>
        /// <param name="num">The number of bytes sent.</param>
        void SentBytes(int num);
    }

    /// <summary>The dispatch observer to instrument servant dispatch.</summary>
    public interface IDispatchObserver : IObserver
    {
        /// <summary>Remote exception notification.</summary>
        void RemoteException();

        /// <summary>Reply notification.</summary>
        /// <param name="size">The size of the reply in bytes.</param>
        void Reply(int size);
    }

    /// <summary>The invocation observer to instrument invocations on proxies. A proxy invocation can either result in
    /// a collocated or remote invocation. If it results in a remote invocation, a sub-observer is requested for the
    /// remote invocation.</summary>
    public interface IInvocationObserver : IObserver
    {
        /// <summary>Get a child invocation observer for this invocation.</summary>
        /// <param name="connection">The connection information.</param>
        /// <param name="size">The size of the invocation in bytes.</param>
        /// <returns>The observer to instrument the child invocation.</returns>
        IChildInvocationObserver? GetChildInvocationObserver(Connection connection, int size);

        /// <summary>Remote exception notification.</summary>
        void RemoteException();

        /// <summary>Retry notification.</summary>
        void Retried();
    }

    /// <summary>The object observer interface used by instrumented objects to notify the observer of their existence.
    /// </summary>
    public interface IObserver
    {
        /// <summary>This method is called when the instrumented object is created or when the observer is attached to
        /// an existing object.</summary>
        void Attach();

        /// <summary>This method is called when the instrumented object is destroyed and as a result the observer
        /// detached from the object.</summary>
        void Detach();

        /// <summary>Notification of a failure.</summary>
        /// <param name="exceptionName">The name of the exception.</param>
        void Failed(string exceptionName);
    }

    /// <summary>The observer updater interface. This interface is implemented by the Ice run-time and an instance of
    /// this interface is provided by the Ice communicator on initialization to the CommunicatorObserver object set
    /// with the communicator constructor.
    /// The Ice communicator calls CommunicatorObserver.SetObserverUpdater(ZeroC.Ice.Instrumentation.ObserverUpdater)
    /// to provide the observer updater. This interface can be used by add-ins implementing the CommunicatorObserver
    /// interface to update the connection observers.</summary>
    public interface IObserverUpdater
    {
        /// <summary>Update the connection observers associated with each Ice connection from the communicator and its
        /// object adapters. When called, this method goes through all the connections and for each connection
        /// CommunicatorObserver.GetConnectionObserver is called. The implementation of GetConnectionObserver has the
        /// possibility to return an updated observer if necessary.</summary>
        void UpdateConnectionObservers();
    }
}
