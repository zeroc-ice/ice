// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>Base interface of all object proxies.</summary>
    public interface IObjectPrx : IEquatable<IObjectPrx>
    {
        /// <summary>Provides an <see cref="OutgoingRequestFrame"/> factory method for each remote operation defined in
        /// the pseudo-interface Object.</summary>
        public static class Request
        {
            /// <summary>Creates an <see cref="OutgoingRequestFrame"/> for operation ice_id.</summary>
            /// <param name="proxy">Proxy to the target Ice Object.</param>
            /// <param name="context">The context to write into the request.</param>
            /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
            /// <returns>A new <see cref="OutgoingRequestFrame"/>.</returns>
            public static OutgoingRequestFrame IceId(
                IObjectPrx proxy,
                IReadOnlyDictionary<string, string>? context,
                CancellationToken cancel) =>
                OutgoingRequestFrame.WithEmptyArgs(proxy, "ice_id", idempotent: true, context, cancel);

            /// <summary>Creates an <see cref="OutgoingRequestFrame"/> for operation ice_ids.</summary>
            /// <param name="proxy">Proxy to the target Ice Object.</param>
            /// <param name="context">The context to write into the request.</param>
            /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
            /// <returns>A new <see cref="OutgoingRequestFrame"/>.</returns>
            public static OutgoingRequestFrame IceIds(
                IObjectPrx proxy,
                IReadOnlyDictionary<string, string>? context,
                CancellationToken cancel) =>
                OutgoingRequestFrame.WithEmptyArgs(proxy, "ice_ids", idempotent: true, context, cancel);

            /// <summary>Creates an <see cref="OutgoingRequestFrame"/> for operation ice_isA.</summary>
            /// <param name="proxy">Proxy to the target Ice Object.</param>
            /// <param name="id">The type ID argument to write into the request.</param>
            /// <param name="context">The context to write into the request.</param>
            /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
            /// <returns>A new <see cref="OutgoingRequestFrame"/>.</returns>
            public static OutgoingRequestFrame IceIsA(
                IObjectPrx proxy,
                string id,
                IReadOnlyDictionary<string, string>? context,
                CancellationToken cancel) =>
                OutgoingRequestFrame.WithArgs(
                    proxy,
                    "ice_isA",
                    idempotent: true,
                    compress: false,
                    format: default,
                    context,
                    id,
                    OutputStream.IceWriterFromString,
                    cancel);

            /// <summary>Creates an <see cref="OutgoingRequestFrame"/> for operation ice_ping.</summary>
            /// <param name="proxy">Proxy to the target Ice Object.</param>
            /// <param name="context">The context to write into the request.</param>
            /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
            /// <returns>A new <see cref="OutgoingRequestFrame"/>.</returns>
            public static OutgoingRequestFrame IcePing(
                IObjectPrx proxy,
                IReadOnlyDictionary<string, string>? context,
                CancellationToken cancel) =>
                OutgoingRequestFrame.WithEmptyArgs(proxy, "ice_ping", idempotent: true, context, cancel);
        }

        /// <summary>Holds an <see cref="InputStreamReader{T}"/> for each non-void remote operation defined in the
        /// pseudo-interface Object.</summary>
        public static class Response
        {
            /// <summary>The <see cref="InputStreamReader{T}"/> reader for the return type of operation ice_id.
            /// </summary>
            public static readonly InputStreamReader<string> IceId = InputStream.IceReaderIntoString;

            /// <summary>The <see cref="InputStreamReader{T}"/> reader for the return type of operation ice_ids.
            /// </summary>
            public static readonly InputStreamReader<string[]> IceIds =
                istr => istr.ReadArray(minElementSize: 1, InputStream.IceReaderIntoString);

            /// <summary>The <see cref="InputStreamReader{T}"/> reader for the return type of operation ice_isA.
            /// </summary>
            public static readonly InputStreamReader<bool> IceIsA = InputStream.IceReaderIntoBool;
        }

        /// <summary>Factory for <see cref="IObjectPrx"/> proxies.</summary>
        public static readonly ProxyFactory<IObjectPrx> Factory = (reference) => new ObjectPrx(reference);

        /// <summary>An <see cref="InputStreamReader{T}"/> used to read <see cref="IObjectPrx"/> proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<IObjectPrx> IceReader = (istr) => istr.ReadProxy(Factory);

        /// <summary>An <see cref="InputStreamReader{T}"/> used to read <see cref="IObjectPrx"/> nullable proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<IObjectPrx?> IceReaderIntoNullable =
            (istr) => istr.ReadNullableProxy(Factory);

        /// <summary>An OutputStream writer used to write <see cref="IObjectPrx"/> proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<IObjectPrx> IceWriter = (ostr, value) => ostr.WriteProxy(value);

        /// <summary>An OutputStream writer used to write <see cref="IObjectPrx"/> nullable proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<IObjectPrx?> IceWriterFromNullable =
            (ostr, value) => ostr.WriteNullableProxy(value);

        /// <summary>Returns the communicator that created this proxy.</summary>
        /// <returns>The communicator that created this proxy.</returns>
        public Communicator Communicator => IceReference.Communicator;

        /// <summary>The connection ID of this proxy.</summary>
        public string ConnectionId => IceReference.ConnectionId;

        /// <summary>The context of this proxy, which will be sent with each invocation made using this proxy.
        /// </summary>
        public IReadOnlyDictionary<string, string> Context => IceReference.Context;

        /// <summary>The encoding used to marshal request parameters.</summary>
        public Encoding Encoding => IceReference.Encoding;

        /// <summary>The endpoints of this proxy. A proxy with a non-empty endpoint list is a direct proxy.</summary>
        public IReadOnlyList<Endpoint> Endpoints => IceReference.Endpoints;

        /// <summary>The facet to use on the target Ice object. The empty string corresponds to the default facet.
        /// </summary>
        public string Facet => IceReference.Facet;

        /// <summary>The proxy's underlying reference. This is a publicly visible Ice-internal property. Applications
        /// should not use it directly.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public Reference IceReference { get; }

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity => IceReference.Identity;

        /// <summary>The invocation mode of this proxy. Only useful for ice1 proxies.</summary>
        public InvocationMode InvocationMode => IceReference.InvocationMode;

        /// <summary>The invocation timeout of this proxy.</summary>
        public TimeSpan InvocationTimeout => IceReference.InvocationTimeout;

        /// <summary>Indicates whether or not this proxy caches its connection.</summary>
        /// <value>True when the proxy caches its connection; otherwise, false.</value>
        public bool IsConnectionCached => IceReference.IsConnectionCached;

        /// <summary>Indicates whether or not this proxy is fixed proxy.</summary>
        /// <value>When true, the proxy is a fixed proxy; when false, the proxy is a regular proxy, not bound to
        /// any connection.</value>
        public bool IsFixed => IceReference.IsFixed;

        /// <summary>Indicates whether or not using this proxy to invoke an operation that does not return anything
        /// waits for an empty response from the target Ice object.</summary>
        /// <value>When true, invoking such an operation does not wait for the response from the target object. When
        /// false, invoking such an operation waits for the empty response from the target object, unless this behavior
        /// is overridden by metadata on the Slice operation's definition.</value>
        public bool IsOneway => IceReference.IsOneway;

        /// <summary>Gets the location of this proxy. Ice uses this location to find the target object.</summary>
        public IReadOnlyList<string> Location => IceReference.Location;

        /// <summary>The locator associated with this proxy. This property is null when no locator is associated with
        /// this proxy.</summary>
        public ILocatorPrx? Locator => IceReference.LocatorInfo?.Locator;

        /// <summary>The locator cache timeout of this proxy.</summary>
        public TimeSpan LocatorCacheTimeout => IceReference.LocatorCacheTimeout;

        /// <summary>Indicates whether or not this proxy prefers non-secure connections.</summary>
        /// <value>When true, the proxy attempts to establish a non-secure connection if such a connection is available;
        /// when false, the proxy attempts first to establish a secure connection and falls back to a non-secure
        /// connection. The fallback to a non-secure connection applies only when using the ice1 protocol.</value>
        public bool PreferNonSecure => IceReference.PreferNonSecure;

        /// <summary>The Ice protocol of this proxy. Requests sent with this proxy use only this Ice protocol.</summary>
        public Protocol Protocol => IceReference.Protocol;

        /// <summary>The router associated with this proxy. This property is null when no router is associated with this
        /// proxy.</summary>
        public IRouterPrx? Router => IceReference.RouterInfo?.Router;

        /// <summary>Indicates whether the two proxy operands are equal.</summary>
        /// <param name="lhs">The left hand-side operand.</param>
        /// <param name="rhs">The right hand-side operand.</param>
        /// <returns><c>True</c> if the tow proxies are equal, <c>False</c> otherwise.</returns>
        public static bool Equals(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs is null || rhs is null)
            {
                return false;
            }

            return lhs.IceReference.Equals(rhs.IceReference);
        }

        /// <summary>Converts the string representation of a proxy to its <see cref="IObjectPrx"/> equivalent.</summary>
        /// <param name="s">The proxy string representation.</param>
        /// <param name="communicator">The communicator for the new proxy.</param>
        /// <returns>The new proxy.</returns>
        /// <exception cref="FormatException"><c>s</c> does not contain a valid string representation of a proxy.
        /// </exception>
        public static IObjectPrx Parse(string s, Communicator communicator) =>
            new ObjectPrx(Reference.Parse(s, communicator));

        /// <summary>Converts the string representation of a proxy to its <see cref="IObjectPrx"/> equivalent.</summary>
        /// <param name="s">The proxy string representation.</param>
        /// <param name="communicator">The communicator for the new proxy.</param>
        /// <param name="prx">When this method returns it contains the new proxy, if the conversion succeeded or null
        /// if the conversion failed.</param>
        /// <returns><c>true</c> if the s parameter was converted successfully; otherwise, <c>false</c>.</returns>
        public static bool TryParse(string s, Communicator communicator, out IObjectPrx? prx)
        {
            try
            {
                prx = new ObjectPrx(Reference.Parse(s, communicator));
            }
            catch (Exception)
            {
                prx = null;
                return false;
            }
            return true;
        }

        /// <summary>Returns the Slice type ID of the most-derived interface supported by the target object of this
        /// proxy.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(IReadOnlyDictionary<string, string>? context = null, CancellationToken cancel = default) =>
            IceInvoke(Request.IceId(this, context, cancel), Response.IceId);

        /// <summary>Returns the Slice type ID of the most-derived interface supported by the target object of this
        /// proxy.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string> IceIdAsync(
            IReadOnlyDictionary<string, string>? context = null,
            IProgress<bool>? progress = null,
            CancellationToken cancel = default) =>
            IceInvokeAsync(Request.IceId(this, context, cancel), Response.IceId, progress);

        /// <summary>Returns the Slice type IDs of the interfaces supported by the target object of this proxy.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(
            IReadOnlyDictionary<string, string>? context = null,
            CancellationToken cancel = default) =>
            IceInvoke(Request.IceIds(this, context, cancel), Response.IceIds);

        /// <summary>Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string[]> IceIdsAsync(
            IReadOnlyDictionary<string, string>? context = null,
            IProgress<bool>? progress = null,
            CancellationToken cancel = default) =>
            IceInvokeAsync(Request.IceIds(this, context, cancel), Response.IceIds, progress);

        /// <summary>Tests whether the target object implements a specific Slice interface.</summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>True if the target object implements the Slice interface identified by id, false otherwise.
        /// </returns>
        public bool IceIsA(
            string id,
            IReadOnlyDictionary<string, string>? context = null,
            CancellationToken cancel = default) =>
            IceInvoke(Request.IceIsA(this, id, context, cancel), Response.IceIsA);

        /// <summary>Tests whether this object supports a specific Slice interface.</summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<bool> IceIsAAsync(
            string id,
            IReadOnlyDictionary<string, string>? context = null,
            IProgress<bool>? progress = null,
            CancellationToken cancel = default) =>
            IceInvokeAsync(Request.IceIsA(this, id, context, cancel), Response.IceIsA, progress);

        /// <summary>Tests whether the target object of this proxy can be reached.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public void IcePing(IReadOnlyDictionary<string, string>? context = null, CancellationToken cancel = default) =>
            IceInvoke(Request.IcePing(this, context, cancel), IsOneway);

        /// <summary>Tests whether the target object of this proxy can be reached.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task IcePingAsync(
            IReadOnlyDictionary<string, string>? context = null,
            IProgress<bool>? progress = null,
            CancellationToken cancel = default) =>
            IceInvokeAsync(Request.IcePing(this, context, cancel), IsOneway, progress);

        /// <summary>Marshals the proxy into an OutputStream.</summary>
        /// <param name="ostr">The OutputStream used to marshal the proxy.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceWrite(OutputStream ostr) => IceReference.Write(ostr);

        /// <summary>Converts a proxy to a set of proxy properties.</summary>
        /// <param name="property">The base property name.</param>
        /// <returns>The property set.</returns>
        public Dictionary<string, string> ToProperty(string property) => IceReference.ToProperty(property);

        /// <summary>Creates a clone of the current object, IceClone is re-implemented by all generated proxy classes.
        /// </summary>
        /// <param name="reference">The proxy's reference for the cloned proxy.</param>
        /// <returns>The new proxy.</returns>
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected IObjectPrx IceClone(Reference reference) => new ObjectPrx(reference);

        /// <summary>Sends a request that returns a value and returns the result synchronously.</summary>
        /// <typeparam name="T">The operation's return type.</typeparam>
        /// <param name="request">The <see cref="OutgoingRequestFrame"/> for this invocation.</param>
        /// <param name="reader">An <see cref="InputStreamReader{T}"/> for the operation's return value. Typically
        /// {IInterfaceNamePrx}.Response.{OperationName}.</param>
        /// <returns>The operation's return value.</returns>
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected T IceInvoke<T>(OutgoingRequestFrame request, InputStreamReader<T> reader)
        {
            try
            {
                IncomingResponseFrame response = this.InvokeAsync(request, oneway: false).Result;
                return response.ReadReturnValue(Communicator, reader);
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
            finally
            {
                request.Dispose();
            }
        }

        /// <summary>Sends a request that returns void and waits synchronously for the result.</summary>
        /// <param name="request">The <see cref="OutgoingRequestFrame"/> for this invocation.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// twoway request.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected void IceInvoke(OutgoingRequestFrame request, bool oneway)
        {
            try
            {
                IncomingResponseFrame response = this.InvokeAsync(request, oneway).Result;
                if (!oneway)
                {
                    response.ReadVoidReturnValue(Communicator);
                }
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
            finally
            {
                request.Dispose();
            }
        }

        /// <summary>Sends a request that returns a value and returns the result asynchronously.</summary>
        /// <typeparam name="T">The operation's return type.</typeparam>
        /// <param name="request">The <see cref="OutgoingRequestFrame"/> for this invocation.</param>
        /// <param name="reader">An <see cref="InputStreamReader{T}"/> for the operation's return value. Typically
        /// {IInterfaceNamePrx}.Response.{OperationName}.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <returns>A task that provides the return value asynchronously.</returns>
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected Task<T> IceInvokeAsync<T>(
            OutgoingRequestFrame request,
            InputStreamReader<T> reader,
            IProgress<bool>? progress)
        {
            Task<IncomingResponseFrame> responseTask;
            try
            {
                responseTask = this.InvokeAsync(request, oneway: false, progress);
            }
            catch
            {
                request.Dispose();
                throw;
            }

            return ReadResponseAsync();

            async Task<T> ReadResponseAsync()
            {
                try
                {
                    return (await responseTask.ConfigureAwait(false)).ReadReturnValue(Communicator, reader);
                }
                finally
                {
                    request.Dispose();
                }
            }
        }

        /// <summary>Sends a request that returns void and returns the result asynchronously.</summary>
        /// <param name="request">The <see cref="OutgoingRequestFrame"/> for this invocation.</param>
        /// <param name="oneway">When true, the request is sent as a oneway request. When false, it is sent as a
        /// twoway request.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <returns>A task that completes when the request completes.</returns>
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected Task IceInvokeAsync(
            OutgoingRequestFrame request,
            bool oneway,
            IProgress<bool>? progress)
        {
            Task<IncomingResponseFrame> responseTask;
            try
            {
                responseTask = this.InvokeAsync(request, oneway, progress);
            }
            catch
            {
                request.Dispose();
                throw;
            }
            // A oneway request still need to await the dummy response to return only once the request is sent.
            return ReadResponseAsync();

            async Task ReadResponseAsync()
            {
                try
                {
                    IncomingResponseFrame response = await responseTask.ConfigureAwait(false);
                    if (!oneway)
                    {
                        response.ReadVoidReturnValue(Communicator);
                    }
                }
                finally
                {
                    request.Dispose();
                }
            }
        }

        internal IObjectPrx Clone(Reference reference) => IceClone(reference);
    }

    /// <summary>The base class for all proxies. It's a publicly visible Ice-internal class. Applications
    /// should not use it directly.</summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public class ObjectPrx : IObjectPrx
    {
        /// <summary>Returns whether this proxy equals the given object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.</summary>
        /// <param name="other">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to other; otherwise, false.</returns>
        public override bool Equals(object? other) => Equals(other as IObjectPrx);

        // Implements IEquatable<IObjectPrx>:
        /// <summary>Returns whether this proxy equals the given proxy. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.</summary>
        /// <param name="other">The proxy to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to other; otherwise, false.</returns>
        public bool Equals(IObjectPrx? other) => other != null && IceReference.Equals(other.IceReference);

        /// <summary>Returns a hash code for this proxy.</summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode() => IceReference.GetHashCode();

        /// <summary>The proxy's underlying reference.</summary>
        public Reference IceReference { get; }

        /// <summary>Returns the stringified form of this proxy.</summary>
        /// <returns>The stringified proxy.</returns>
        public override string ToString() => IceReference.ToString();

        /// <summary>Constructs a new proxy.</summary>
        /// <param name="reference">The proxy's underlying reference.</param>
        protected internal ObjectPrx(Reference reference) => IceReference = reference;
    }
}
