//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    public static class Proxy
    {
        public static IObjectPrx Clone(this IObjectPrx prx,
                                       Identity id,
                                       string? adapterId = null,
                                       bool clearLocator = false,
                                       bool clearRouter = false,
                                       bool? collocationOptimized = null,
                                       bool? compress = null,
                                       bool? connectionCached = null,
                                       string? connectionId = null,
                                       int? connectionTimeout = null,
                                       Dictionary<string, string>? context = null,
                                       Encoding? encoding = null,
                                       EndpointSelectionType? endpointSelectionType = null,
                                       IEndpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       ILocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       IRouterPrx? router = null,
                                       bool? secure = null)
        {
            Reference reference = prx.IceReference.Clone(
                id,
                null,
                adapterId,
                clearLocator,
                clearRouter,
                collocationOptimized,
                compress,
                connectionCached,
                connectionId,
                connectionTimeout,
                context,
                encoding,
                endpointSelectionType,
                endpoints,
                fixedConnection,
                invocationMode,
                invocationTimeout,
                locator,
                locatorCacheTimeout,
                oneway,
                preferSecure,
                router,
                secure);
            return reference.Equals(prx.IceReference) ? prx : prx.Clone(reference);
        }

        public static IObjectPrx Clone(this IObjectPrx prx,
                                       string facet,
                                       string? adapterId = null,
                                       bool clearLocator = false,
                                       bool clearRouter = false,
                                       bool? collocationOptimized = null,
                                       bool? compress = null,
                                       bool? connectionCached = null,
                                       string? connectionId = null,
                                       int? connectionTimeout = null,
                                       Dictionary<string, string>? context = null,
                                       Encoding? encoding = null,
                                       EndpointSelectionType? endpointSelectionType = null,
                                       IEndpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       ILocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       IRouterPrx? router = null,
                                       bool? secure = null)
        {
            Reference reference = prx.IceReference.Clone(
                null,
                facet,
                adapterId,
                clearLocator,
                clearRouter,
                collocationOptimized,
                compress,
                connectionCached,
                connectionId,
                connectionTimeout,
                context,
                encoding,
                endpointSelectionType,
                endpoints,
                fixedConnection,
                invocationMode,
                invocationTimeout,
                locator,
                locatorCacheTimeout,
                oneway,
                preferSecure,
                router,
                secure);
            return reference.Equals(prx.IceReference) ? prx : prx.Clone(reference);
        }

        public static Prx Clone<Prx>(this Prx prx,
                                     string? adapterId = null,
                                     bool clearLocator = false,
                                     bool clearRouter = false,
                                     bool? collocationOptimized = null,
                                     bool? compress = null,
                                     bool? connectionCached = null,
                                     string? connectionId = null,
                                     int? connectionTimeout = null,
                                     Dictionary<string, string>? context = null,
                                     Encoding? encoding = null,
                                     EndpointSelectionType? endpointSelectionType = null,
                                     IEndpoint[]? endpoints = null,
                                     Connection? fixedConnection = null,
                                     InvocationMode? invocationMode = null,
                                     int? invocationTimeout = null,
                                     ILocatorPrx? locator = null,
                                     int? locatorCacheTimeout = null,
                                     bool? oneway = null,
                                     bool? preferSecure = null,
                                     IRouterPrx? router = null,
                                     bool? secure = null) where Prx : IObjectPrx
        {
            Reference reference = prx.IceReference.Clone(
                null,
                null,
                adapterId,
                clearLocator,
                clearRouter,
                collocationOptimized,
                compress,
                connectionCached,
                connectionId,
                connectionTimeout,
                context,
                encoding,
                endpointSelectionType,
                endpoints,
                fixedConnection,
                invocationMode,
                invocationTimeout,
                locator,
                locatorCacheTimeout,
                oneway,
                preferSecure,
                router,
                secure);
            return reference.Equals(prx.IceReference) ? prx : (Prx)prx.Clone(reference);
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
        public static Connection? GetCachedConnection(this IObjectPrx prx)
        {
            IRequestHandler? handler;
            lock (prx)
            {
                handler = prx.RequestHandler;
            }

            if (handler != null)
            {
                try
                {
                    return handler.GetConnection();
                }
                catch (LocalException)
                {
                }
            }
            return null;
        }

        /// <summary>Sends a request synchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The outgoing request frame for this invocation. Usually this request frame should have
        /// been created using the same proxy, however some differences are acceptable, for example proxy can have
        /// different endpoints.</param>
        /// <returns>The response frame.</returns>
        public static IncomingResponseFrame Invoke(this IObjectPrx proxy, OutgoingRequestFrame request)
        {
            try
            {
                return proxy.IceInvokeAsync(request, null, CancellationToken.None, true).Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>Sends a request asynchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The outgoing request frame for this invocation. Usually this request frame should have
        /// been created using the same proxy, however some differences are acceptable, for example proxy can have
        /// different endpoints.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static Task<IncomingResponseFrame> InvokeAsync(this IObjectPrx proxy,
                                                              OutgoingRequestFrame request,
                                                              IProgress<bool>? progress = null,
                                                              CancellationToken cancel = default)
            => proxy.IceInvokeAsync(request, progress, cancel, false);

        /// <summary>Forwards an incoming request to another Ice object.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The incoming request frame.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static async ValueTask<OutputStream> ForwardAsync(this IObjectPrx proxy,
                                                                 IncomingRequestFrame request,
                                                                 IProgress<bool>? progress = null,
                                                                 CancellationToken cancel = default)
        {
            var forwardedRequest = new OutgoingRequestFrame(proxy, request.Current.Operation,
                request.Current.IsIdempotent, request.Current.Context, request.TakePayload());

            IncomingResponseFrame response =
                await proxy.InvokeAsync(forwardedRequest, progress, cancel).ConfigureAwait(false);
            return new OutgoingResponseFrame(request.Current, response.ReplyStatus, response.TakePayload());
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
