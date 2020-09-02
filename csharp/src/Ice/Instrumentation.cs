//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace ZeroC.Ice.Instrumentation
{
    public interface IChildInvocationObserver : IObserver
    {
        /// <summary>Reply notification.</summary>
        /// <param name="size">The size of the reply in bytes.</param>
        void Reply(int size);
    }

    public partial interface ICollocatedObserver : IChildInvocationObserver
    {
    }

    public interface ICommunicatorObserver
    {
        /// <summary>This method should return an observer for the given endpoint information and connector.
        /// The Ice run-time calls this method for each connection establishment attempt.</summary>
        /// <param name="endpoint">The endpoint.</param>
        /// <param name="connector">The description of the connector. For IP transports, this is typically the IP
        /// address to connect to.</param>
        /// <returns>The observer to instrument the connection establishment.</returns>
        IObserver? GetConnectionEstablishmentObserver(Endpoint endpoint, string connector);

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
        /// this method to resolve an endpoint and obtain the list of connectors. For IP endpoints, this typically
        /// involves doing a DNS lookup to obtain the IP addresses associated with the DNS name.</summary>
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

    public interface IConnectionObserver : IObserver
    {
        /// <summary>Notification of received bytes over the connection.</summary>
        /// <param name="num">The number of bytes received.</param>
        void ReceivedBytes(int num);

        /// <summary>Notification of sent bytes over the connection.</summary>
        /// <param name="num">The number of bytes sent.</param>
        void SentBytes(int num);
    }

    public interface IDispatchObserver : IObserver
    {
        /// <summary>Remote exception notification.</summary>
        void RemoteException();

        /// <summary>Reply notification.</summary>
        /// <param name="size">The size of the reply in bytes.</param>
        void Reply(int size);
    }

    public interface IInvocationObserver : IObserver
    {
        /// <summary>Get a collocated observer for this invocation.</summary>
        /// <param name="adapter">The object adapter hosting the collocated Ice object.</param>
        /// <param name="requestId">The ID of the invocation.</param>
        /// <param name="size">The size of the invocation in bytes.</param>
        /// <returns>The observer to instrument the collocated invocation.</returns>
        ICollocatedObserver? GetCollocatedObserver(ObjectAdapter adapter, long requestId, int size);

        /// <summary>Get a remote observer for this invocation.</summary>
        /// <param name="connection">The connection information.</param>
        /// <param name="requestId">The invocation request ID.</param>
        /// <param name="size">The size of the invocation in bytes.</param>
        /// <returns>The observer to instrument the remote invocation.</returns>
        IRemoteObserver? GetRemoteObserver(Connection connection, long requestId, int size);

        /// <summary>Remote exception notification.</summary>
        void RemoteException();

        /// <summary>Retry notification.</summary>
        void Retried();
    }

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

    public interface IObserverUpdater
    {
        /// <summary>Update the connection observers associated with each Ice connection from the communicator and its
        /// object adapters. When called, this method goes through all the connections and for each connection
        /// CommunicatorObserver.GetConnectionObserver is called. The implementation of GetConnectionObserver has the
        /// possibility to return an updated observer if necessary.</summary>
        void UpdateConnectionObservers();
    }

    public interface IRemoteObserver : IChildInvocationObserver
    {
    }
}
