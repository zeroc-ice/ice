//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    /// <summary>Proxy provides extension methods for IObjectPrx</summary>
    public static class Proxy
    {
        /// <summary>Creates a clone of this proxy, with a new identity and optionally other options. The clone
        /// is identical to this proxy except for its identity and other options set through parameters.</summary>
        /// <param name="prx">The source proxy.</param>
        /// <param name="identity">The identity of the clone.</param>
        /// <param name="factory">The proxy factory used to manufacture the clone.</param>
        /// <param name="adapterId">The adapter ID of the clone (optional).</param>
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
        /// <param name="invocationMode">The invocation mode of the clone (optional).</param>
        /// <param name="invocationTimeout">The invocation timeout of the clone (optional).</param>
        /// <param name="locator">The locator proxy of the clone (optional).</param>
        /// <param name="locatorCacheTimeout">The locator cache timeout of the clone (optional).</param>
        /// <param name="oneway">Determines whether the clone is oneway or twoway (optional). This is a simplified
        /// version of the invocationMode parameter.</param>
        /// <param name="preferNonSecure">Determines whether the clone prefers non-secure connections over secure
        /// connections (optional).</param>
        /// <param name="protocol">The Ice protocol of the clone (optional).</param>
        /// <param name="router">The router proxy of the clone (optional).</param>
        /// <returns>A new proxy manufactured by the proxy factory (see factory parameter).</returns>
        public static T Clone<T>(this IObjectPrx prx,
                                 Identity identity,
                                 ProxyFactory<T> factory,
                                 string? adapterId = null,
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
                                 InvocationMode? invocationMode = null,
                                 int? invocationTimeout = null,
                                 ILocatorPrx? locator = null,
                                 TimeSpan? locatorCacheTimeout = null,
                                 bool? oneway = null,
                                 bool? preferNonSecure = null,
                                 Protocol? protocol = null,
                                 IRouterPrx? router = null) where T : class, IObjectPrx
        {
            return factory(prx.IceReference.Clone(adapterId,
                                                  cacheConnection,
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
                                                  invocationMode,
                                                  invocationTimeout,
                                                  locator,
                                                  locatorCacheTimeout,
                                                  oneway,
                                                  preferNonSecure,
                                                  protocol,
                                                  router));
        }

        /// <summary>Creates a clone of this proxy, with a new facet and optionally other options. The clone is
        /// identical to this proxy except for its facet and other options set through parameters.</summary>
        /// <param name="prx">The source proxy.</param>
        /// <param name="facet">The facet of the clone.</param>
        /// <param name="factory">The proxy factory used to manufacture the clone.</param>
        /// <param name="adapterId">The adapter ID of the clone (optional).</param>
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
        /// <param name="invocationMode">The invocation mode of the clone (optional).</param>
        /// <param name="invocationTimeout">The invocation timeout of the clone (optional).</param>
        /// <param name="locator">The locator proxy of the clone (optional).</param>
        /// <param name="locatorCacheTimeout">The locator cache timeout of the clone (optional).</param>
        /// <param name="oneway">Determines whether the clone is oneway or twoway (optional). This is a simplified
        /// version of the invocationMode parameter.</param>
        /// <param name="preferNonSecure">Determines whether the clone prefers non-secure connections over secure
        /// connections (optional).</param>
        /// <param name="protocol">The Ice protocol of the clone (optional).</param>
        /// <param name="router">The router proxy of the clone (optional).</param>
        /// <returns>A new proxy manufactured by the proxy factory (see factory parameter).</returns>
        public static T Clone<T>(this IObjectPrx prx,
                                 string facet,
                                 ProxyFactory<T> factory,
                                 string? adapterId = null,
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
                                 int? invocationTimeout = null,
                                 ILocatorPrx? locator = null,
                                 TimeSpan? locatorCacheTimeout = null,
                                 bool? oneway = null,
                                 bool? preferNonSecure = null,
                                 Protocol? protocol = null,
                                 IRouterPrx? router = null) where T : class, IObjectPrx
        {
            return factory(prx.IceReference.Clone(adapterId,
                                                  cacheConnection,
                                                  clearLocator,
                                                  clearRouter,
                                                  connectionId,
                                                  context,
                                                  encoding,
                                                  endpointSelection,
                                                  endpoints,
                                                  facet,
                                                  fixedConnection,
                                                  identity: null,
                                                  invocationMode,
                                                  invocationTimeout,
                                                  locator,
                                                  locatorCacheTimeout,
                                                  oneway,
                                                  preferNonSecure,
                                                  protocol,
                                                  router));
        }

        /// <summary>Creates a clone of this proxy. The clone is identical to this proxy except for options set
        /// through parameters. This method returns this proxy instead of a new proxy in the event none of the options
        /// specified through the parameters change this proxy's options.</summary>
        /// <param name="prx">The source proxy.</param>
        /// <param name="adapterId">The adapter ID of the clone (optional).</param>
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
        /// <param name="invocationMode">The invocation mode of the clone (optional).</param>
        /// <param name="invocationTimeout">The invocation timeout of the clone (optional).</param>
        /// <param name="locator">The locator proxy of the clone (optional).</param>
        /// <param name="locatorCacheTimeout">The locator cache timeout of the clone (optional).</param>
        /// <param name="oneway">Determines whether the clone is oneway or twoway (optional). This is a simplified
        /// version of the invocationMode parameter.</param>
        /// <param name="preferNonSecure">Determines whether the clone prefers non-secure connections over secure
        /// connections (optional).</param>
        /// <param name="protocol">The Ice protocol of the clone (optional).</param>
        /// <param name="router">The router proxy of the clone (optional).</param>
        /// <returns>A new proxy with the same type as this proxy.</returns>
        public static T Clone<T>(this T prx,
                                 string? adapterId = null,
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
                                 int? invocationTimeout = null,
                                 ILocatorPrx? locator = null,
                                 TimeSpan? locatorCacheTimeout = null,
                                 bool? oneway = null,
                                 bool? preferNonSecure = null,
                                 Protocol? protocol = null,
                                 IRouterPrx? router = null) where T : IObjectPrx
        {
            Reference clone = prx.IceReference.Clone(adapterId,
                                                     cacheConnection,
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
                                                     invocationMode,
                                                     invocationTimeout,
                                                     locator,
                                                     locatorCacheTimeout,
                                                     oneway,
                                                     preferNonSecure,
                                                     protocol,
                                                     router);

            // Reference.Clone never returns a new reference == to itself.
            return ReferenceEquals(clone, prx.IceReference) ? prx : (T)prx.Clone(clone);
        }

        /// <summary>Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.</summary>
        /// <returns>The Connection for this proxy or null if colocation optimization is used.</returns>
        public static Connection? GetConnection(this IObjectPrx prx)
        {
            try
            {
                ValueTask<IRequestHandler> task = prx.IceReference.GetRequestHandlerAsync(cancel: default);
                if (task.IsCompleted)
                {
                    return (task.Result as ConnectionRequestHandler)?.GetConnection();
                }
                else
                {
                    return (task.AsTask().Result as ConnectionRequestHandler)?.GetConnection();
                }
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
        public static async ValueTask<Connection?> GetConnectionAsync(this IObjectPrx prx,
                                                                      CancellationToken cancel = default)
        {
            IRequestHandler handler = await prx.IceReference.GetRequestHandlerAsync(cancel).ConfigureAwait(false);
            return (handler as ConnectionRequestHandler)?.GetConnection();
        }

        /// <summary>Returns the cached Connection for this proxy. If the proxy does not yet have an established
        /// connection, it does not attempt to create a connection.</summary>
        /// <returns>The cached Connection for this proxy (null if the proxy does not have
        /// an established connection).</returns>
        public static Connection? GetCachedConnection(this IObjectPrx prx) => prx.IceReference.GetCachedConnection();

        /// <summary>Sends a request synchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The outgoing request frame for this invocation. Usually this request frame should have
        /// been created using the same proxy, however some differences are acceptable, for example proxy can have
        /// different endpoints.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// two-way request.</param>
        /// <returns>The response frame.</returns>
        public static IncomingResponseFrame Invoke(this IObjectPrx proxy, OutgoingRequestFrame request,
                                                   bool oneway = false)
        {
            try
            {
                ValueTask<IncomingResponseFrame> task = InvokeAsync(proxy, request, oneway, synchronous: true);
                return task.IsCompleted ? task.Result : task.AsTask().Result;
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Sends a request asynchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The outgoing request frame for this invocation. Usually this request frame should have
        /// been created using the same proxy, however some differences are acceptable, for example proxy can have
        /// different endpoints.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// two-way request.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static ValueTask<IncomingResponseFrame> InvokeAsync(this IObjectPrx proxy,
                                                                   OutgoingRequestFrame request,
                                                                   bool oneway = false,
                                                                   IProgress<bool>? progress = null,
                                                                   CancellationToken cancel = default) =>
            InvokeAsync(proxy, request, oneway, synchronous: false, progress, cancel);

        /// <summary>Forwards an incoming request to another Ice object.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// two-way request.</param>
        /// <param name="request">The incoming request frame.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static ValueTask<OutgoingResponseFrame> ForwardAsync(this IObjectPrx proxy,
                                                                    bool oneway,
                                                                    IncomingRequestFrame request,
                                                                    IProgress<bool>? progress = null,
                                                                    CancellationToken cancel = default)
        {
            var forwardedRequest = new OutgoingRequestFrame(proxy,
                                                            request.Operation,
                                                            request.IsIdempotent,
                                                            request.Context,
                                                            request.Payload);

            return WaitResponseAsync(request, proxy.InvokeAsync(forwardedRequest, oneway: oneway, progress, cancel));

            static async ValueTask<OutgoingResponseFrame> WaitResponseAsync(
                IncomingRequestFrame request,
                ValueTask<IncomingResponseFrame> task)
            {
                // TODO: need protocol bridging when the protocols are not the same.
                IncomingResponseFrame response = await task.ConfigureAwait(false);
                return new OutgoingResponseFrame(request, response.Payload);
            }
        }

        private static ValueTask<IncomingResponseFrame> InvokeAsync(this IObjectPrx proxy,
                                                                    OutgoingRequestFrame request,
                                                                    bool oneway,
                                                                    bool synchronous,
                                                                    IProgress<bool>? progress = null,
                                                                    CancellationToken cancel = default)
        {
            InvocationMode mode = proxy.IceReference.InvocationMode;
            if (mode == InvocationMode.BatchOneway || mode == InvocationMode.BatchDatagram)
            {
                Debug.Assert(false); // not implemented
                return default;
            }
            if (mode == InvocationMode.Datagram && !oneway)
            {
                throw new InvalidOperationException("cannot make two-way call on a datagram proxy");
            }
            return InvokeAsync(proxy, request, oneway, synchronous, progress, cancel);

            static async ValueTask<IncomingResponseFrame> InvokeAsync(IObjectPrx proxy,
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
                int retryCount = 0;
                CancellationTokenSource? invocationTimeout = null;
                if (reference.InvocationTimeout > 0)
                {
                    invocationTimeout = new CancellationTokenSource();
                    invocationTimeout.CancelAfter(reference.InvocationTimeout);
                    if (cancel.CanBeCanceled)
                    {
                        cancel = CancellationTokenSource.CreateLinkedTokenSource(cancel, invocationTimeout.Token).Token;
                    }
                    else
                    {
                        cancel = invocationTimeout.Token;
                    }
                }

                try
                {
                    while (true)
                    {
                        IRequestHandler? handler = null;
                        var progressWrapper = new ProgressWrapper(progress);
                        try
                        {
                            // Get the request handler, this will eventually establish a connection if needed.
                            handler = await reference.GetRequestHandlerAsync(cancel).ConfigureAwait(false);

                            // Send the request and if it's a twoway request get the task to wait for the response
                            IncomingResponseFrame response =
                                await handler.SendRequestAsync(request,
                                                               oneway,
                                                               synchronous,
                                                               observer,
                                                               progressWrapper,
                                                               cancel).ConfigureAwait(false);

                            switch (response.ReplyStatus)
                            {
                                case ReplyStatus.OK:
                                    break;
                                case ReplyStatus.UserException:
                                    observer?.RemoteException();
                                    break;
                                case ReplyStatus.ObjectNotExistException:
                                case ReplyStatus.FacetNotExistException:
                                case ReplyStatus.OperationNotExistException:
                                    throw response.ReadDispatchException();
                                case ReplyStatus.UnknownException:
                                case ReplyStatus.UnknownLocalException:
                                case ReplyStatus.UnknownUserException:
                                    throw response.ReadUnhandledException();
                            }
                            return response;
                        }
                        catch (RetryException)
                        {
                            // Clear the proxy's cached request handler if connection caching is enabled
                            if (reference.IsConnectionCached)
                            {
                                proxy.IceReference.ClearRequestHandler(handler!);
                            }
                        }
                        catch (OperationCanceledException)
                        {
                            throw; // Don't retry cancelled operations
                        }
                        catch (Exception ex)
                        {
                            // Clear the proxy's cached request handler if connection caching is enabled
                            if (reference.IsConnectionCached && handler != null)
                            {
                                proxy.IceReference.ClearRequestHandler(handler);
                            }

                            // TODO: revisit retry logic
                            // We only retry after failing with a DispatchException or a local exception.
                            int delay = reference.CheckRetryAfterException(ex,
                                                                           progressWrapper.IsSent,
                                                                           request.IsIdempotent,
                                                                           ref retryCount);
                            if (delay > 0)
                            {
                                // The delay task can be canceled either by the user code using the provided
                                // cancellation token or if the communicator is destroyed.
                                CancellationToken token = CancellationTokenSource.CreateLinkedTokenSource(
                                    cancel,
                                    proxy.Communicator.CancellationToken).Token;
                                await Task.Delay(delay, token).ConfigureAwait(false);
                            }

                            observer?.Retried();
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Check the reason of the cancellation
                    if (ex is OperationCanceledException)
                    {
                        if (invocationTimeout?.IsCancellationRequested ?? false)
                        {
                            ex = new TimeoutException();
                            observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                            throw ex;
                        }
                        else if (proxy.Communicator.CancellationToken.IsCancellationRequested)
                        {
                            ex = new CommunicatorDisposedException();
                            observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                            throw ex;
                        }
                    }
                    observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                    throw;
                }
                finally
                {
                    // Use IDisposable for observers, this will allow using "using".
                    observer?.Detach();
                }
            }
        }

        private class ProgressWrapper : IProgress<bool>
        {
            private readonly IProgress<bool>? _progress;

            internal bool IsSent { get; private set; }

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
