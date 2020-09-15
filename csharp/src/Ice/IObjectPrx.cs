//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The proxy's invocation mode.</summary>
    public enum InvocationMode : byte
    {
        /// <summary>This is the default invocation mode, a request using twoway mode always expects a response.
        /// </summary>
        Twoway,
        /// <summary>A request using oneway mode returns control to the application code as soon as it has been
        /// accepted by the local transport.</summary>
        Oneway,
        /// <summary>The batch-oneway invocation mode is no longer supported, it was supported with version up to 3.7.
        /// </summary>
        [Obsolete("The batch-oneway invocation mode is no longer supported, it was supported with version up to 3.7")]
        BatchOneway,
        /// <summary>Invocation mode use by datagram based transports.</summary>
        Datagram,
        /// <summary>
        /// The batch-datagram invocation mode is no longer supported, it was supported with version up to 3.7
        /// </summary>
        [Obsolete("The batch-datagram invocation mode is no longer supported, it was supported with version up to 3.7")]
        BatchDatagram,

        /// <summary>Marker for the last value.</summary>
#pragma warning disable CS0618 // Type or member is obsolete
        Last = BatchDatagram
#pragma warning restore CS0618 // Type or member is obsolete
    }

    /// <summary>Delegate used to create typed proxies.</summary>
    /// <typeparam name="T">The proxy type.</typeparam>
    /// <param name="reference">The underlying reference.</param>
    /// <returns>The new proxy.</returns>
    public delegate T ProxyFactory<T>(Reference reference) where T : IObjectPrx;

    /// <summary>Base interface of all object proxies.</summary>
    public interface IObjectPrx : IEquatable<IObjectPrx>
    {
        public static class Request
        {

        }

        public static class Response
        {

        }

        /// <summary>An InputStream reader used to read non nullable proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<IObjectPrx> IceReader = (istr) => istr.ReadProxy(Factory);

        /// <summary>An InputStream reader used to read nullable proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<IObjectPrx?> IceReaderIntoNullable =
            (istr) => istr.ReadNullableProxy(Factory);

        /// <summary>An OutputStream writer used to write non nullable proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<IObjectPrx> IceWriter = (ostr, value) => ostr.WriteProxy(value);

        /// <summary>An OutputStream writer used to write nullable proxies.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<IObjectPrx?> IceWriterFromNullable =
            (ostr, value) => ostr.WriteNullableProxy(value);

        /// <summary>The proxy's underlying reference. This is a publicly visible Ice-internal method. Applications
        /// should not use it directly.</summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public Reference IceReference { get; }

        /// <summary>Creates a clone of the current object, IceClone is re-implemented by all generated proxy classes.
        /// </summary>
        /// <param name="reference">The proxy's reference for the cloned proxy.</param>
        /// <returns>The new proxy.</returns>
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected IObjectPrx IceClone(Reference reference) => new ObjectPrx(reference);
        internal IObjectPrx Clone(Reference reference) => IceClone(reference);

        /// <summary>Returns the communicator that created this proxy.</summary>
        /// <returns>The communicator that created this proxy.</returns>
        public Communicator Communicator => IceReference.Communicator;

        /// <summary>Convert a proxy to a set of proxy properties.</summary>
        /// <param name="property">The base property name.</param>
        /// <returns>The property set.</returns>
        public Dictionary<string, string> ToProperty(string property) => IceReference.ToProperty(property);

        /// <summary>Tests whether the target object implements a specific Slice interface.</summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>True if the target object implements the Slice interface identified by id</returns>.
        public bool IceIsA(
            string id,
            IReadOnlyDictionary<string, string>? context = null,
            CancellationToken cancel = default) =>
            IceI_IsARequest.Invoke(this, id, context, cancel);

        /// <summary>Tests whether this object supports a specific Slice interface.</summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<bool> IceIsAAsync(string id,
                                      IReadOnlyDictionary<string, string>? context = null,
                                      IProgress<bool>? progress = null,
                                      CancellationToken cancel = default) =>
            IceI_IsARequest.InvokeAsync(this, id, context, progress, cancel);

        /// <summary>Tests whether the target object of this proxy can be reached.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public void IcePing(IReadOnlyDictionary<string, string>? context = null, CancellationToken cancel = default) =>
            IceI_PingRequest.Invoke(this, context, cancel);

        /// <summary>Tests whether the target object of this proxy can be reached.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task IcePingAsync(IReadOnlyDictionary<string, string>? context = null,
                                 IProgress<bool>? progress = null,
                                 CancellationToken cancel = default) =>
            IceI_PingRequest.InvokeAsync(this, context, progress, cancel);

        /// <summary>Returns the Slice type IDs of the interfaces supported by the target object of this proxy.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(IReadOnlyDictionary<string, string>? context = null, CancellationToken cancel = default) =>
            IceI_IdsRequest.Invoke(this, context, cancel);

        /// <summary>Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string[]> IceIdsAsync(IReadOnlyDictionary<string, string>? context = null,
                                          IProgress<bool>? progress = null,
                                          CancellationToken cancel = default) =>
            IceI_IdsRequest.InvokeAsync(this, context, progress, cancel);

        /// <summary>Returns the Slice type ID of the most-derived interface supported by the target object of this
        /// proxy.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(IReadOnlyDictionary<string, string>? context = null, CancellationToken cancel = default) =>
            IceI_IdRequest.Invoke(this, context, cancel);

        /// <summary>Returns the Slice type ID of the most-derived interface supported by the target object of this
        /// proxy.</summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string> IceIdAsync(IReadOnlyDictionary<string, string>? context = null,
                                       IProgress<bool>? progress = null,
                                       CancellationToken cancel = default) =>
            IceI_IdRequest.InvokeAsync(this, context, progress, cancel);

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity => IceReference.Identity;

        /// <summary>The context of this proxy, which will be sent with each invocation made using this proxy.
        /// </summary>
        public IReadOnlyDictionary<string, string> Context => IceReference.Context;

        /// <summary>The facet to use on the target Ice object. The empty string corresponds to the default facet.
        /// </summary>
        public string Facet => IceReference.Facet;

        /// <summary>The adapter ID of this proxy, or an empty string if this proxy does not have an adapter ID.
        /// A proxy with an adapter ID is an indirect proxy. </summary>
        public string AdapterId => IceReference.AdapterId;

        /// <summary>The endpoints of this proxy. A proxy with a non-empty endpoint list is a direct proxy.</summary>
        public IReadOnlyList<Endpoint> Endpoints => IceReference.Endpoints;

        /// <summary>Indicates whether or not this proxy caches its connection.</summary>
        /// <value>True when the proxy caches its connection; otherwise, false.</value>
        public bool IsConnectionCached => IceReference.IsConnectionCached;

        /// <summary>The endpoint selection policy of this proxy, which can be Random or Ordered.</summary>
        public EndpointSelectionType EndpointSelection => IceReference.EndpointSelection;

        /// <summary>The encoding used to marshal request parameters.</summary>
        public Encoding Encoding => IceReference.Encoding;

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

        /// <summary>The locator associated with this proxy. This property is null when no locator is associated with
        /// this proxy.</summary>
        public ILocatorPrx? Locator => IceReference.LocatorInfo?.Locator;

        /// <summary>Indicates whether or not using this proxy to invoke an operation that does not return anything
        /// waits for an empty response from the target Ice object.</summary>
        /// <value>When true, invoking such an operation does not wait for the response from the target object. This
        /// corresponds to several <see cref="InvocationMode"/> enumerators, such as Oneway and Datagram. When false,
        /// invoking such an operation waits for the empty response from the target object, unless this behavior is
        /// overridden by metadata on the Slice operation's definition.</value>
        public bool IsOneway => IceReference.InvocationMode != InvocationMode.Twoway;

        /// <summary>The invocation mode of this proxy. </summary>
        public InvocationMode InvocationMode => IceReference.InvocationMode;

        /// <summary>The connection ID of this proxy.</summary>
        public string ConnectionId => IceReference.ConnectionId;

        /// <summary>Indicates whether or not this proxy is fixed proxy.</summary>
        /// <value>When true, the proxy is a fixed proxy; when false, the proxy is a regular proxy, not bound to
        /// any connection.</value>
        public bool IsFixed => IceReference.IsFixed;

        /// <summary>Marshals the proxy into an OutputStream.</summary>
        /// <param name="ostr">The OutputStream used to marshal the proxy.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceWrite(OutputStream ostr) => IceReference.Write(ostr);

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

        /// <summary>Factory for IObjectPrx proxies.</summary>
        public static readonly ProxyFactory<IObjectPrx> Factory = (reference) => new ObjectPrx(reference);

        /// <summary>Converts the string representation of a proxy to its <see cref="IObjectPrx"/> equivalent.</summary>
        /// <param name="s">The proxy string representation.</param>
        /// <param name="communicator">The communicator for the new proxy</param>
        /// <returns>The new proxy</returns>
        /// <exception cref="FormatException"><c>s</c> does not contain a valid string representation of a proxy.
        /// </exception>
        public static IObjectPrx Parse(string s, Communicator communicator) =>
            new ObjectPrx(Reference.Parse(s, communicator));

        /// <summary>Converts the string representation of a proxy to its <see cref="IObjectPrx"/> equivalent.</summary>
        /// <param name="s">The proxy string representation.</param>
        /// <param name="communicator">The communicator for the new proxy</param>
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

        private static OutgoingRequestWithParam<string, bool>? _iceI_IsARequest;
        private static OutgoingRequestWithParam<string, bool> IceI_IsARequest =>
            _iceI_IsARequest ??= new OutgoingRequestWithParam<string, bool>("ice_isA",
                                                                            idempotent: true,
                                                                            compress: false,
                                                                            format: default,
                                                                            writer: OutputStream.IceWriterFromString,
                                                                            reader: InputStream.IceReaderIntoBool);

        private static OutgoingRequestWithEmptyParamList<string[]>? _iceI_IdsRequest;
        private static OutgoingRequestWithEmptyParamList<string[]> IceI_IdsRequest =>
            _iceI_IdsRequest ??= new OutgoingRequestWithEmptyParamList<string[]>("ice_ids", idempotent: true,
                reader: istr => istr.ReadArray(1, InputStream.IceReaderIntoString));

        private static OutgoingRequestWithEmptyParamList<string>? _iceI_IdRequest;
        private static OutgoingRequestWithEmptyParamList<string> IceI_IdRequest =>
            _iceI_IdRequest ??= new OutgoingRequestWithEmptyParamList<string>("ice_id", idempotent: true,
                reader: InputStream.IceReaderIntoString);

        private static OutgoingRequestWithEmptyParamList? _iceI_PingRequest;
        private static OutgoingRequestWithEmptyParamList IceI_PingRequest =>
            _iceI_PingRequest ??= new OutgoingRequestWithEmptyParamList("ice_ping", idempotent: true, oneway: false);
    }

    /// <summary>The base class for all proxies. It's a publicly visible Ice-internal class. Applications
    /// should not use it directly.</summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public class ObjectPrx : IObjectPrx
    {
        /// <summary>The proxy's underlying reference.</summary>
        public Reference IceReference { get; }

        /// <summary>Returns the stringified form of this proxy.</summary>
        /// <returns>The stringified proxy.</returns>
        public override string ToString() => IceReference.ToString();

        /// <summary>Returns a hash code for this proxy.</summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode() => IceReference.GetHashCode();

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

        /// <summary>Constructs a new proxy.</summary>
        /// <param name="reference">The proxy's underlying reference.</param>
        protected internal ObjectPrx(Reference reference) => IceReference = reference;
    }
}
