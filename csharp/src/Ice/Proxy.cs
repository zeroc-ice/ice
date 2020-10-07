// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    /// <summary>Factory function that creates a proxy from a reference.</summary>
    /// <typeparam name="T">The proxy type.</typeparam>
    /// <param name="reference">The underlying reference.</param>
    /// <returns>The new proxy.</returns>
    public delegate T ProxyFactory<T>(Reference reference) where T : IObjectPrx;

    /// <summary>Proxy provides extension methods for IObjectPrx.</summary>
    public static class Proxy
    {
        /// <summary>Tests whether this proxy points to a remote object derived from T. If so it returns a proxy of
        /// type T otherwise returns null.</summary>
        /// <param name="prx">The source proxy.</param>
        /// <param name="factory">The proxy factory used to manufacture the returned proxy.</param>
        /// <param name="context">The request context used for the remote
        /// <see cref="IObjectPrx.IceIsA(string, IReadOnlyDictionary{string, string}?, CancellationToken)"/>
        /// invocation.</param>
        /// <returns>A new proxy manufactured by the proxy factory (see factory parameter).</returns>
        public static T? CheckedCast<T>(
            this IObjectPrx prx,
            ProxyFactory<T> factory,
            IReadOnlyDictionary<string, string>? context = null) where T : class, IObjectPrx =>
            prx.IceIsA(typeof(T).GetIceTypeId()!, context) ? factory(prx.IceReference) : null;

        /// <summary>Creates a clone of this proxy, with a new identity and optionally other options. The clone
        /// is identical to this proxy except for its identity and other options set through parameters.</summary>
        /// <param name="prx">The source proxy.</param>
        /// <param name="factory">The proxy factory used to manufacture the clone.</param>
        /// <param name="cacheConnection">Determines whether or not the clone caches its connection (optional).</param>
        /// <param name="clearLocator">When set to true, the clone does not have an associated locator proxy (optional).
        /// </param>
        /// <param name="clearRouter">When set to true, the clone does not have an associated router proxy (optional).
        /// </param>
        /// <param name="connectionId">The connection ID of the clone (optional).</param>
        /// <param name="context">The context of the clone (optional).</param>
        /// <param name="encoding">The encoding of the clone (optional).</param>
        /// <param name="endpointSelection">The encoding selection policy of the clone (optional).</param>
        /// <param name="endpoints">The endpoints of the clone (optional).</param>
        /// <param name="facet">The facet of the clone (optional).</param>
        /// <param name="fixedConnection">The connection of the clone (optional). When specified, the clone is a fixed
        /// proxy. You can clone a non-fixed proxy into a fixed proxy but not vice-versa.</param>
        /// <param name="identity">The identity of the clone.</param>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <param name="invocationMode">The invocation mode of the clone (optional). Applies only to ice1 proxies.
        /// </param>
        /// <param name="location">The location of the clone (optional).</param>
        /// <param name="locator">The locator proxy of the clone (optional).</param>
        /// <param name="locatorCacheTimeout">The locator cache timeout of the clone (optional).</param>
        /// <param name="oneway">Determines whether the clone is oneway or twoway (optional).</param>
        /// <param name="preferNonSecure">Determines whether the clone prefers non-secure connections over secure
        /// connections (optional).</param>
        /// <param name="router">The router proxy of the clone (optional).</param>
        /// <returns>A new proxy manufactured by the proxy factory (see factory parameter).</returns>
        public static T Clone<T>(
            this IObjectPrx prx,
            ProxyFactory<T> factory,
            bool? cacheConnection = null,
            bool clearLocator = false,
            bool clearRouter = false,
            string? connectionId = null,
            IReadOnlyDictionary<string, string>? context = null,
            Encoding? encoding = null,
            EndpointSelectionType? endpointSelection = null,
            IEnumerable<Endpoint>? endpoints = null,
            string? facet = null,
            Connection? fixedConnection = null,
            Identity? identity = null,
            string? identityAndFacet = null,
            InvocationMode? invocationMode = null,
            IEnumerable<string>? location = null,
            ILocatorPrx? locator = null,
            TimeSpan? locatorCacheTimeout = null,
            bool? oneway = null,
            bool? preferNonSecure = null,
            IRouterPrx? router = null) where T : class, IObjectPrx =>
            factory(prx.IceReference.Clone(cacheConnection,
                                           clearLocator,
                                           clearRouter,
                                           connectionId,
                                           context,
                                           encoding,
                                           endpointSelection,
                                           endpoints,
                                           facet,
                                           fixedConnection,
                                           identity,
                                           identityAndFacet,
                                           invocationMode,
                                           location,
                                           locator,
                                           locatorCacheTimeout,
                                           oneway,
                                           preferNonSecure,
                                           router));

        /// <summary>Creates a clone of this proxy. The clone is identical to this proxy except for options set
        /// through parameters. This method returns this proxy instead of a new proxy in the event none of the options
        /// specified through the parameters change this proxy's options.</summary>
        /// <param name="prx">The source proxy.</param>
        /// <param name="cacheConnection">Determines whether or not the clone caches its connection (optional).</param>
        /// <param name="clearLocator">When set to true, the clone does not have an associated locator proxy (optional).
        /// </param>
        /// <param name="clearRouter">When set to true, the clone does not have an associated router proxy (optional).
        /// </param>
        /// <param name="connectionId">The connection ID of the clone (optional).</param>
        /// <param name="context">The context of the clone (optional).</param>
        /// <param name="encoding">The encoding of the clone (optional).</param>
        /// <param name="endpointSelection">The encoding selection policy of the clone (optional).</param>
        /// <param name="endpoints">The endpoints of the clone (optional).</param>
        /// <param name="fixedConnection">The connection of the clone (optional). When specified, the clone is a fixed
        /// proxy. You can clone a non-fixed proxy into a fixed proxy but not vice-versa.</param>
        /// <param name="invocationMode">The invocation mode of the clone (optional). Applies only to ice1 proxies.
        /// </param>
        /// <param name="location">The location of the clone (optional).</param>
        /// <param name="locator">The locator proxy of the clone (optional).</param>
        /// <param name="locatorCacheTimeout">The locator cache timeout of the clone (optional).</param>
        /// <param name="oneway">Determines whether the clone is oneway or twoway (optional).</param>
        /// <param name="preferNonSecure">Determines whether the clone prefers non-secure connections over secure
        /// connections (optional).</param>
        /// <param name="router">The router proxy of the clone (optional).</param>
        /// <returns>A new proxy with the same type as this proxy.</returns>
        public static T Clone<T>(
            this T prx,
            bool? cacheConnection = null,
            bool clearLocator = false,
            bool clearRouter = false,
            string? connectionId = null,
            IReadOnlyDictionary<string, string>? context = null,
            Encoding? encoding = null,
            EndpointSelectionType? endpointSelection = null,
            IEnumerable<Endpoint>? endpoints = null,
            Connection? fixedConnection = null,
            InvocationMode? invocationMode = null,
            IEnumerable<string>? location = null,
            ILocatorPrx? locator = null,
            TimeSpan? locatorCacheTimeout = null,
            bool? oneway = null,
            bool? preferNonSecure = null,
            IRouterPrx? router = null) where T : IObjectPrx
        {
            Reference clone = prx.IceReference.Clone(cacheConnection,
                                                     clearLocator,
                                                     clearRouter,
                                                     connectionId,
                                                     context,
                                                     encoding,
                                                     endpointSelection,
                                                     endpoints,
                                                     facet: null,
                                                     fixedConnection,
                                                     identity: null,
                                                     identityAndFacet: null,
                                                     invocationMode,
                                                     location,
                                                     locator,
                                                     locatorCacheTimeout,
                                                     oneway,
                                                     preferNonSecure,
                                                     router);

            // Reference.Clone never returns a new reference == to itself.
            return ReferenceEquals(clone, prx.IceReference) ? prx : (T)prx.Clone(clone);
        }

        /// <summary>Returns the cached Connection for this proxy. If the proxy does not yet have an established
        /// connection, it does not attempt to create a connection.</summary>
        /// <returns>The cached Connection for this proxy (null if the proxy does not have
        /// an established connection).</returns>
        public static Connection? GetCachedConnection(this IObjectPrx prx) => prx.IceReference.GetCachedConnection();

        /// <summary>Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.</summary>
        /// <returns>The Connection for this proxy or null if colocation optimization is used.</returns>
        public static Connection? GetConnection(this IObjectPrx prx)
        {
            try
            {
                ValueTask<Connection?> task = prx.GetConnectionAsync(cancel: default);
                return task.IsCompleted ? task.Result : task.AsTask().Result;
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.</summary>
        /// <returns>The Connection for this proxy or null if colocation optimization is used.</returns>
        public static async ValueTask<Connection?> GetConnectionAsync(
            this IObjectPrx prx,
            CancellationToken cancel = default)
        {
            IRequestHandler handler = await prx.IceReference.GetRequestHandlerAsync(
                ImmutableList<IConnector>.Empty,
                cancel).ConfigureAwait(false);
            return handler as Connection;
        }

        /// <summary>Sends a request synchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The <see cref="OutgoingRequestFrame"/> for this invocation. Usually this request
        /// frame should have been created using the same proxy, however some differences are acceptable, for example
        /// proxy can have different endpoints.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// twoway request.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The response frame.</returns>
        public static IncomingResponseFrame Invoke(
            this IObjectPrx proxy,
            OutgoingRequestFrame request,
            bool oneway = false,
            CancellationToken cancel = default)
        {
            try
            {
                return InvokeWithInterceptorsAsync(proxy, request, oneway, synchronous: true, cancel: cancel).Result;
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Sends a request asynchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The <see cref="OutgoingRequestFrame"/> for this invocation. Usually this request
        /// frame should have been created using the same proxy, however some differences are acceptable, for example
        /// proxy can have different endpoints.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// two-way request.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static Task<IncomingResponseFrame> InvokeAsync(
            this IObjectPrx proxy,
            OutgoingRequestFrame request,
            bool oneway = false,
            IProgress<bool>? progress = null,
            CancellationToken cancel = default) =>
            InvokeWithInterceptorsAsync(proxy, request, oneway, synchronous: false, progress, cancel);

        /// <summary>Forwards an incoming request to another Ice object represented by the <paramref name="proxy"/>
        /// parameter.</summary>
        /// <remarks>When the incoming request frame's protocol and proxy's protocol are different, this method
        /// automatically bridges between these two protocols. When proxy's protocol is ice1, the resulting outgoing
        /// request frame is never compressed.</remarks>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// two-way request.</param>
        /// <param name="request">The incoming request frame to forward to proxy's target.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static async ValueTask<OutgoingResponseFrame> ForwardAsync(
            this IObjectPrx proxy,
            bool oneway,
            IncomingRequestFrame request,
            IProgress<bool>? progress = null,
            CancellationToken cancel = default)
        {
            var forwardedRequest = new OutgoingRequestFrame(proxy, request);

            IncomingResponseFrame response;
            try
            {
                response = await proxy.InvokeAsync(forwardedRequest, oneway, progress, cancel).ConfigureAwait(false);
            }
            catch (DispatchException ex)
            {
                // It looks like InvokeAsync threw a 1.1 system exception (used for retries)
                // TODO: fix (remove) when we fix retries
                ex.ConvertToUnhandled = false;
                throw;
            }
            return new OutgoingResponseFrame(request, response);
        }

        private static Task<IncomingResponseFrame> InvokeAsync(
            this IObjectPrx proxy,
            OutgoingRequestFrame request,
            bool oneway,
            bool synchronous,
            IProgress<bool>? progress,
            CancellationToken cancel)
        {
            request.Finish();
            InvocationMode mode = proxy.IceReference.InvocationMode;
            switch (mode)
            {
                case InvocationMode.BatchOneway:
                case InvocationMode.BatchDatagram:
                    Debug.Assert(false); // not implemented
                    return default;
                case InvocationMode.Datagram when !oneway:
                    throw new InvalidOperationException("cannot make two-way call on a datagram proxy");
                default:
                    return InvokeAsync(proxy, request, oneway, synchronous, progress, cancel);
            }

            static async Task<IncomingResponseFrame> InvokeAsync(
                IObjectPrx proxy,
                OutgoingRequestFrame request,
                bool oneway,
                bool synchronous,
                IProgress<bool>? progress,
                CancellationToken cancel)
            {
                Reference reference = proxy.IceReference;

                IInvocationObserver? observer = ObserverHelper.GetInvocationObserver(proxy,
                                                                                     request.Operation,
                                                                                     request.Context);
                try
                {
                    IncomingResponseFrame? response = null;
                    Exception? lastException = null;
                    List<IConnector>? excludedConnectors = null;
                    for (int retryCount = 0; retryCount < reference.Communicator.RetryMaxAttempts;)
                    {
                        IRequestHandler? handler = null;
                        var progressWrapper = new ProgressWrapper(progress);
                        Retryable retryable = Retryable.No;
                        TimeSpan afterDelay = TimeSpan.Zero;
                        try
                        {
                            // Get the request handler, this will eventually establish a connection if needed.
                            handler = await reference.GetRequestHandlerAsync(
                                excludedConnectors ?? (IReadOnlyList<IConnector>)ImmutableList<IConnector>.Empty,
                                cancel).ConfigureAwait(false);

                            // Send the request and if it's a twoway request get the task to wait for the response
                            response = await handler.SendRequestAsync(request,
                                                                      oneway,
                                                                      synchronous,
                                                                      observer,
                                                                      progressWrapper,
                                                                      cancel).ConfigureAwait(false);
                            lastException = null;
                            if (response.ResultType != ResultType.Failure)
                            {
                                return response;
                            }
                            // retry below
                        }
                        catch (InvalidRequestHandlerException)
                        {
                            // TODO temporary, this is already fixed in the transport refactoring branch
                            // Not a retry, we try to send the request using a handler that was no longer valid
                            if (reference.IsConnectionCached)
                            {
                                reference.ClearRequestHandler(handler!);
                            }
                            continue;
                        }
                        catch (NoMoreReplicasException)
                        {
                            // The last exception asked to retry using a different replica, but we already tried all
                            // known replicas.
                            break;
                        }
                        catch (ConnectionClosedByPeerException ex)
                        {
                            // Always retry a gracefully close connection. Don't exclude the connector after a
                            // graceful close connection, in case it is the only connector available.
                            lastException = ex;
                            retryable = Retryable.AfterDelay;
                        }
                        catch (TransportException ex) when (request.IsIdempotent || !progressWrapper.IsSent)
                        {
                            // Retry transport exceptions if the request is idempotent or was not send
                            lastException = ex;
                            retryable = Retryable.AfterDelay;
                        }

                        if (lastException == null)
                        {
                            Debug.Assert(response != null && response.ResultType == ResultType.Failure);
                            observer?.RemoteException();
                            if (response.ReadSystemException(proxy.Communicator) is Exception systemException &&
                                systemException is ObjectNotExistException one)
                            {
                                // 1.1 System exceptions
                                lastException = systemException;
                                if (reference.RouterInfo != null && one.Operation == "ice_add_proxy")
                                {
                                    // If we have a router, an ObjectNotExistException with an operation name
                                    // "ice_add_proxy" indicates to the client that the router isn't aware of the proxy
                                    // (for example, because it was evicted by the router). In this case, we must
                                    // *always* retry, so that the missing proxy is added to the router.
                                    reference.RouterInfo.ClearCache(reference);
                                    retryable = Retryable.AfterDelay;
                                }
                                else if (reference.IsIndirect)
                                {
                                    if (reference.IsWellKnown)
                                    {
                                        reference.LocatorInfo?.ClearCache(reference);
                                    }
                                    retryable = Retryable.AfterDelay;
                                }
                            }
                            else if (response.BinaryContext.TryGetValue((int)BinaryContext.RetryPolicy,
                                                                        out ReadOnlyMemory<byte> value))
                            {
                                retryable = (Retryable)value.Span[0];
                                afterDelay = retryable == Retryable.AfterDelay ?
                                    TimeSpan.FromMilliseconds(value[1..].Span.ReadVarULong().Value) : TimeSpan.Zero;
                            }
                        }

                        if (retryable == Retryable.No)
                        {
                            break; // We cannot retry
                        }
                        else if (++retryCount < reference.Communicator.RetryMaxAttempts)
                        {
                            if (handler is Connection connection)
                            {
                                if (retryable == Retryable.OtherReplica)
                                {
                                    excludedConnectors ??= new List<IConnector>();
                                    excludedConnectors.Add(connection.Connector);
                                    if (reference.Communicator.TraceLevels.Retry >= 1)
                                    {
                                        reference.Communicator.Logger.Trace(
                                            reference.Communicator.TraceLevels.RetryCategory,
                                            $"excluding connector\n{connection.Connector}");
                                    }
                                }

                                if (reference.IsConnectionCached)
                                {
                                    reference.ClearRequestHandler(handler);
                                }
                            }

                            if (reference.Communicator.TraceLevels.Retry >= 1)
                            {
                                reference.Communicator.Logger.Trace(
                                    reference.Communicator.TraceLevels.RetryCategory,
                                    "retrying operation call: because of exception");
                            }

                            if (retryable == Retryable.AfterDelay && afterDelay != TimeSpan.Zero)
                            {
                                // The delay task can be canceled either by the user code using the provided
                                // cancellation token or if the communicator is destroyed.
                                using var tokenSource = CancellationTokenSource.CreateLinkedTokenSource(
                                    cancel,
                                    proxy.Communicator.CancellationToken);
                                await Task.Delay(afterDelay, tokenSource.Token).ConfigureAwait(false);
                            }
                            observer?.Retried();
                        }
                    }

                    // No more retries return the response
                    if (lastException != null)
                    {
                        observer?.Failed(lastException.GetType().FullName ?? "System.Exception");
                        throw ExceptionUtil.Throw(lastException);
                    }
                    else
                    {
                        observer?.Failed("System.Exception"); // TODO cleanup observer logic
                        Debug.Assert(response != null && response.ResultType == ResultType.Failure);
                        return response;
                    }
                }
                finally
                {
                    // TODO: Use IDisposable for observers, this will allow using "using".
                    observer?.Detach();
                }
            }
        }

        private static Task<IncomingResponseFrame> InvokeWithInterceptorsAsync(
            this IObjectPrx proxy,
            OutgoingRequestFrame request,
            bool oneway,
            bool synchronous,
            IProgress<bool>? progress = null,
            CancellationToken cancel = default)
        {
            return InvokeWithInterceptorsAsync(proxy, request, oneway, synchronous, 0, progress, cancel);

            static Task<IncomingResponseFrame> InvokeWithInterceptorsAsync(
                IObjectPrx proxy,
                OutgoingRequestFrame request,
                bool oneway,
                bool synchronous,
                int i,
                IProgress<bool>? progress,
                CancellationToken cancel)
            {
                cancel.ThrowIfCancellationRequested();
                if (i < proxy.Communicator.InvocationInterceptors.Count)
                {
                    InvocationInterceptor interceptor = proxy.Communicator.InvocationInterceptors[i++];
                    return interceptor(
                        proxy,
                        request,
                        (target, request, cancel) =>
                            InvokeWithInterceptorsAsync(target, request, oneway, synchronous, i, progress, cancel),
                        cancel);
                }
                else
                {
                    return proxy.InvokeAsync(request, oneway, synchronous, progress, cancel);
                }
            }
        }

        private class ProgressWrapper : IProgress<bool>
        {
            internal bool IsSent { get; private set; }
            private readonly IProgress<bool>? _progress;

            public void Report(bool sentSynchronously)
            {
                if (_progress != null)
                {
                    Task.Run(() => _progress.Report(sentSynchronously));
                }
                IsSent = true;
            }

            internal ProgressWrapper(IProgress<bool>? progress) => _progress = progress;
        }
    }
}
