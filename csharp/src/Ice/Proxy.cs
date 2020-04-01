//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    public static class Proxy
    {
        public static T Clone<T>(this IObjectPrx prx,
                                 Identity identity,
                                 ProxyFactory<T> factory,
                                 string? adapterId = null,
                                 bool? cacheConnection = null,
                                 bool clearLocator = false,
                                 bool clearRouter = false,
                                 bool? collocationOptimized = null,
                                 bool? compress = null,
                                 string? connectionId = null,
                                 int? connectionTimeout = null,
                                 IReadOnlyDictionary<string, string>? context = null,
                                 Encoding? encoding = null,
                                 EndpointSelectionType? endpointSelection = null,
                                 IEndpoint[]? endpoints = null,
                                 string? facet = null,
                                 Connection? fixedConnection = null,
                                 InvocationMode? invocationMode = null,
                                 int? invocationTimeout = null,
                                 ILocatorPrx? locator = null,
                                 int? locatorCacheTimeout = null,
                                 bool? oneway = null,
                                 bool? preferSecure = null,
                                 Protocol? protocol = null,
                                 IRouterPrx? router = null,
                                 bool? secure = null) where T : class, IObjectPrx
        {
            return factory(prx.IceReference.Clone(adapterId,
                                                  cacheConnection,
                                                  clearLocator,
                                                  clearRouter,
                                                  collocationOptimized,
                                                  compress,
                                                  connectionId,
                                                  connectionTimeout,
                                                  context,
                                                  encoding,
                                                  endpointSelection,
                                                  endpoints?.Select(e => (Endpoint)e).ToArray(),
                                                  facet,
                                                  fixedConnection,
                                                  identity,
                                                  invocationMode,
                                                  invocationTimeout,
                                                  locator,
                                                  locatorCacheTimeout,
                                                  oneway,
                                                  preferSecure,
                                                  protocol,
                                                  router,
                                                  secure));
        }

        public static T Clone<T>(this IObjectPrx prx,
                                 string facet,
                                 ProxyFactory<T> factory,
                                 string? adapterId = null,
                                 bool? cacheConnection = null,
                                 bool clearLocator = false,
                                 bool clearRouter = false,
                                 bool? collocationOptimized = null,
                                 bool? compress = null,
                                 string? connectionId = null,
                                 int? connectionTimeout = null,
                                 IReadOnlyDictionary<string, string>? context = null,
                                 Encoding? encoding = null,
                                 EndpointSelectionType? endpointSelection = null,
                                 IEndpoint[]? endpoints = null,
                                 Connection? fixedConnection = null,
                                 InvocationMode? invocationMode = null,
                                 int? invocationTimeout = null,
                                 ILocatorPrx? locator = null,
                                 int? locatorCacheTimeout = null,
                                 bool? oneway = null,
                                 bool? preferSecure = null,
                                 Protocol? protocol = null,
                                 IRouterPrx? router = null,
                                 bool? secure = null) where T : class, IObjectPrx
        {
            return factory(prx.IceReference.Clone(adapterId,
                                                  cacheConnection,
                                                  clearLocator,
                                                  clearRouter,
                                                  collocationOptimized,
                                                  compress,
                                                  connectionId,
                                                  connectionTimeout,
                                                  context,
                                                  encoding,
                                                  endpointSelection,
                                                  endpoints?.Select(e => (Endpoint)e).ToArray(),
                                                  facet,
                                                  fixedConnection,
                                                  identity: null,
                                                  invocationMode,
                                                  invocationTimeout,
                                                  locator,
                                                  locatorCacheTimeout,
                                                  oneway,
                                                  preferSecure,
                                                  protocol,
                                                  router,
                                                  secure));
        }

        public static T Clone<T>(this T prx,
                                 string? adapterId = null,
                                 bool? cacheConnection = null,
                                 bool clearLocator = false,
                                 bool clearRouter = false,
                                 bool? collocationOptimized = null,
                                 bool? compress = null,
                                 string? connectionId = null,
                                 int? connectionTimeout = null,
                                 IReadOnlyDictionary<string, string>? context = null,
                                 Encoding? encoding = null,
                                 EndpointSelectionType? endpointSelection = null,
                                 IEndpoint[]? endpoints = null,
                                 Connection? fixedConnection = null,
                                 InvocationMode? invocationMode = null,
                                 int? invocationTimeout = null,
                                 ILocatorPrx? locator = null,
                                 int? locatorCacheTimeout = null,
                                 bool? oneway = null,
                                 bool? preferSecure = null,
                                 Protocol? protocol = null,
                                 IRouterPrx? router = null,
                                 bool? secure = null) where T : IObjectPrx
        {
            Reference clone = prx.IceReference.Clone(adapterId,
                                                     cacheConnection,
                                                     clearLocator,
                                                     clearRouter,
                                                     collocationOptimized,
                                                     compress,
                                                     connectionId,
                                                     connectionTimeout,
                                                     context,
                                                     encoding,
                                                     endpointSelection,
                                                     endpoints?.Select(e => (Endpoint)e).ToArray(),
                                                     facet: null,
                                                     fixedConnection,
                                                     identity: null,
                                                     invocationMode,
                                                     invocationTimeout,
                                                     locator,
                                                     locatorCacheTimeout,
                                                     oneway,
                                                     preferSecure,
                                                     protocol,
                                                     router,
                                                     secure);

            // Reference.Clone never returns a new reference == to itself.
            return ReferenceEquals(clone, prx.IceReference) ? prx : (T)prx.Clone(clone);
        }

        /// <summary>
        /// Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.
        /// </summary>
        /// <returns>The Connection for this proxy.</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
        public static Connection GetConnection(this IObjectPrx prx)
        {
            try
            {
                var completed = new GetConnectionTaskCompletionCallback();
                IceI_getConnection(prx, completed, true);
                return completed.Task.Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        public static Task<Connection> GetConnectionAsync(this IObjectPrx prx,
                                                          IProgress<bool>? progress = null,
                                                          CancellationToken cancel = new CancellationToken())
        {
            var completed = new GetConnectionTaskCompletionCallback(progress, cancel);
            IceI_getConnection(prx, completed, false);
            return completed.Task;
        }

        private static void IceI_getConnection(IObjectPrx prx, IOutgoingAsyncCompletionCallback completed, bool synchronous)
        {
            var outgoing = new ProxyGetConnection(prx, completed);
            try
            {
                outgoing.Invoke("ice_getConnection", synchronous);
            }
            catch (Exception ex)
            {
                outgoing.Abort(ex);
            }
        }

        /// <summary>
        /// Returns the cached Connection for this proxy. If the proxy does not yet have an established
        /// connection, it does not attempt to create a connection.
        /// </summary>
        /// <returns>The cached Connection for this proxy (null if the proxy does not have
        /// an established connection).</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
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
            => proxy.IceInvoke(request, oneway);

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
        public static Task<IncomingResponseFrame> InvokeAsync(this IObjectPrx proxy,
                                                              OutgoingRequestFrame request,
                                                              bool oneway = false,
                                                              IProgress<bool>? progress = null,
                                                              CancellationToken cancel = default)
            => proxy.IceInvokeAsync(request, oneway, progress, cancel);

        /// <summary>Forwards an incoming request to another Ice object.</summary>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// two-way request.</param>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The incoming request frame.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static async ValueTask<OutgoingResponseFrame> ForwardAsync(this IObjectPrx proxy,
                                                                          bool oneway,
                                                                          IncomingRequestFrame request,
                                                                          IProgress<bool>? progress = null,
                                                                          CancellationToken cancel = default)
        {
            var forwardedRequest = new OutgoingRequestFrame(proxy, request.Operation, request.IsIdempotent,
                request.Context, request.Payload);
            IncomingResponseFrame response =
                await proxy.InvokeAsync(forwardedRequest, oneway: oneway, progress, cancel)
                    .ConfigureAwait(false);
            return new OutgoingResponseFrame(request.Encoding, response.Payload);
        }

        private class GetConnectionTaskCompletionCallback : TaskCompletionCallback<Connection>
        {
            public GetConnectionTaskCompletionCallback(IProgress<bool>? progress = null,
                                                       CancellationToken cancellationToken = new CancellationToken()) :
                base(progress, cancellationToken)
            {
            }

            public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase og) =>
                SetResult(((ProxyGetConnection)og).GetConnection()!);
        }
    }
}
