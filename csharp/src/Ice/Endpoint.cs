//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /** TODO Add this constant once it is removed from Endpoint.ice
    public abstract class TCPEndpointType
    {
        public const short value = 1;
    }

    public abstract class SSLEndpointType
    {
        public const short value = 2;
    }

    public abstract class UDPEndpointType
    {
        public const short value = 3;
    }

    public abstract class WSEndpointType
    {
        public const short value = 4;
    }

    public abstract class WSSEndpointType
    {
        public const short value = 5;
    }

    public abstract class BTEndpointType
    {
        public const short value = 6;
    }

    public abstract class BTSEndpointType
    {
        public const short value = 7;
    }

    public abstract class iAPEndpointType
    {
        public const short value = 8;
    }

    public abstract class iAPSEndpointType
    {
        public const short value = 9;
    }**/

    [global::System.Serializable]
    public abstract partial class EndpointInfo
    {
        public EndpointInfo underlying;
        public int timeout;
        public bool compress;

        /// <summary>
        /// Returns the type of the endpoint.
        /// </summary>
        /// <returns>The endpoint type.</returns>
        public abstract short type();

        /// <summary>
        /// Returns true if this endpoint is a datagram endpoint.
        /// </summary>
        /// <returns>True for a datagram endpoint.</returns>
        public abstract bool datagram();

        /// <summary>
        /// Returns true if this endpoint is a secure endpoint.
        /// </summary>
        /// <returns>True for a secure endpoint.</returns>
        public abstract bool secure();

        partial void ice_initialize();

        protected EndpointInfo()
        {
            ice_initialize();
        }

        protected EndpointInfo(EndpointInfo underlying, int timeout, bool compress)
        {
            this.underlying = underlying;
            this.timeout = timeout;
            this.compress = compress;
            ice_initialize();
        }
    }

    public partial interface Endpoint
    {
        /// <summary>
        /// Return a string representation of the endpoint.
        /// </summary>
        /// <returns>The string representation of the endpoint.</returns>
        string ice_toString_();

        /// <summary>
        /// Returns the endpoint information.
        /// </summary>
        /// <returns>The endpoint information class.</returns>
        EndpointInfo getInfo();
    }

    [global::System.Serializable]
    public abstract partial class IPEndpointInfo : EndpointInfo
    {
        public string host;
        public int port;
        public string sourceAddress;
        partial void ice_initialize();

        protected IPEndpointInfo() : base()
        {
            this.host = "";
            this.sourceAddress = "";
            ice_initialize();
        }

        protected IPEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string host, int port, string sourceAddress) : base(underlying, timeout, compress)
        {
            this.host = host;
            this.port = port;
            this.sourceAddress = sourceAddress;
            ice_initialize();
        }
    }

    [global::System.Serializable]
    public abstract partial class TCPEndpointInfo : IPEndpointInfo
    {
        partial void ice_initialize();

        protected TCPEndpointInfo() : base()
        {
            ice_initialize();
        }

        protected TCPEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string host, int port, string sourceAddress) : base(underlying, timeout, compress, host, port, sourceAddress)
        {
            ice_initialize();
        }
    }

    [global::System.Serializable]
    public abstract partial class UDPEndpointInfo : IPEndpointInfo
    {
        public string mcastInterface;
        public int mcastTtl;
        partial void ice_initialize();
        protected UDPEndpointInfo() : base()
        {
            this.mcastInterface = "";
            ice_initialize();
        }

        protected UDPEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string host, int port, string sourceAddress, string mcastInterface, int mcastTtl) : base(underlying, timeout, compress, host, port, sourceAddress)
        {
            this.mcastInterface = mcastInterface;
            this.mcastTtl = mcastTtl;
            ice_initialize();
        }
    }

    [global::System.Serializable]
    public abstract partial class WSEndpointInfo : EndpointInfo
    {
        public string resource;
        partial void ice_initialize();

        protected WSEndpointInfo() : base()
        {
            this.resource = "";
            ice_initialize();
        }

        protected WSEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string resource) : base(underlying, timeout, compress)
        {
            this.resource = resource;
            ice_initialize();
        }
    }

    [global::System.Serializable]
    public abstract partial class OpaqueEndpointInfo : EndpointInfo
    {
        public EncodingVersion rawEncoding;
        public byte[] rawBytes;
        partial void ice_initialize();
        protected OpaqueEndpointInfo() : base()
        {
            this.rawEncoding = new EncodingVersion();
            ice_initialize();
        }

        protected OpaqueEndpointInfo(EndpointInfo underlying, int timeout, bool compress, EncodingVersion rawEncoding, byte[] rawBytes) : base(underlying, timeout, compress)
        {
            this.rawEncoding = rawEncoding;
            this.rawBytes = rawBytes;
            ice_initialize();
        }
    }
}
