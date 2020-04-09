//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;

using System;
using System.Collections.Generic;

namespace Ice
{
    /// <summary>Each endpoint's type is identified by a short value. The enumerators of EndpointType represent the
    /// types of endpoints that the Ice runtime knows and implements. Other endpoint types, with values not represented
    /// by these enumerators, can be implemented and registered using transport plug-ins.</summary>
    public enum EndpointType : short
    {
        TCP = 1,
        SSL = 2,
        UDP = 3,
        WS = 4,
        WSS = 5,
        BT = 6,
        BTS = 7,
        iAP = 8,
        iAPS = 9
    }

    public interface IEndpointConnectors
    {
        void Connectors(List<IConnector> connectors);
        void Exception(Exception ex);
    }

     /// <summary>An endpoint describes a server-side network sink for Ice requests: an object adapter listens on one or
     /// more endpoints and a client establishes a connection to a given object adapter endpoint. An endpoint
     /// encapsulates a network transport protocol such as TCP or Bluetooth RFCOMM, plus transport-specific addressing
     /// information. The Endpoint class is the base class for all endpoint classes.</summary>
    public abstract class Endpoint : IEquatable<Endpoint>
    {
        /// <summary>The connection ID of this endpoint.</summary>
        public abstract string ConnectionId { get; }

        /// <summary>Indicates whether or not this endpoint's compression flag is set. When the compression flag is
        /// set, a request sent to this endpoint using the ice1 protocol is automatically compressed using bzip2 if
        /// the request's uncompressed size is greater than 100 bytes.</summary>
        /// <value>True when the compression flag is set; otherwise, false.</value>
        public abstract bool HasCompressionFlag { get; }

        /// <summary>Indicates whether or not this endpoint's transport uses datagrams with no ordering or delivery
        /// guarantees.</summary>
        /// <value>True when this endpoint's transport is datagram-based; otherwise, false.</value>
        public abstract bool IsDatagram { get; }

        /// <summary>Indicates whether or not this endpoint's transport is secure.</summary>
        /// <value>True when this endpoint's transport is secure; otherwise, false.</value>
        public abstract bool IsSecure { get; }

        /// <summary>The name of the endpoint's transport in lowercase, or "opaque" when the transport's name is
        /// unknown.</summary>
        public virtual string Name => Type.ToString().ToLowerInvariant();

        /// <summary>The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.</summary>
        public abstract int Timeout { get; }

        /// <summary>The EndpointType of this endpoint.</summary>
        public abstract EndpointType Type { get; }

        /// <summary>The underlying endpoint.</summary>
        public virtual Endpoint? Underlying => null;

        public static bool operator ==(Endpoint? lhs, Endpoint? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs is null || rhs is null)
            {
                return false;
            }
            return rhs.Equals(lhs);
        }

        public static bool operator !=(Endpoint? lhs, Endpoint? rhs) => !(lhs == rhs);

        public override bool Equals(object? obj) => obj != null && obj is Endpoint other && Equals(other);
        public abstract bool Equals(Endpoint? other);
        public abstract override int GetHashCode();

        public override string ToString() => $"{Name}{OptionsToString()}";

        // Converts all the options to a string with a leading empty space character.
        public abstract string OptionsToString();

        // Checks whether the endpoint is equivalent to another one.
        // TODO: describe what equivalent means / is used for.
        public abstract bool Equivalent(Endpoint endpoint);

        // Marshal the endpoint.
        public virtual void IceWrite(OutputStream s)
        {
            s.StartEndpointEncapsulation();
            IceWriteImpl(s);
            s.EndEndpointEncapsulation();
        }
        public abstract void IceWriteImpl(Ice.OutputStream s);

        // Returns a new endpoint with a different timeout value, provided that timeouts are supported by the endpoint.
        // Otherwise the same endpoint is returned.
        public abstract Endpoint NewTimeout(int t);

        // Returns a new endpoint with a different connection id.
        public abstract Endpoint NewConnectionId(string connectionId);

        // Returns a new endpoint with a different compression flag, provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        public abstract Endpoint NewCompressionFlag(bool compressionFlag);

        // Returns a connector for this endpoint, or empty list if no connector is available.
        public abstract void ConnectorsAsync(EndpointSelectionType endpointSelection, IEndpointConnectors callback);

        // Expands endpoint out in to separate endpoints for each local host if listening on INADDR_ANY on server side
        // or if no host was specified on client side.
        public abstract List<Endpoint> ExpandIfWildcard();

        // Expands endpoint out into separate endpoints for each IP address returned by the DNS resolver. Also returns
        // the endpoint which can be used to connect to the returned endpoints or null if no specific endpoint can be
        // used to connect to these endpoints (e.g.: with the IP endpoint, it returns this endpoint if it uses a fixed
        // port, null otherwise).
        public abstract List<Endpoint> ExpandHost(out Endpoint? publishedEndpoint);

        // Returns an acceptor for this endpoint, or null if no acceptor is available.
        public abstract IAcceptor? GetAcceptor(string adapterName);

        // Return a server side transceiver for this endpoint, or null if a transceiver can only be created by an
        // acceptor.
        public abstract ITransceiver? GetTransceiver();
    }
}
