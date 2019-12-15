//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//

namespace Ice
{
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
    }

    [System.Serializable]
    public abstract class EndpointInfo
    {
        public EndpointInfo? underlying;
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

        protected EndpointInfo()
        {
        }

        protected EndpointInfo(EndpointInfo? underlying, int timeout, bool compress)
        {
            this.underlying = underlying;
            this.timeout = timeout;
            this.compress = compress;
        }
    }

    public interface Endpoint
    {
        /// <summary>
        /// Returns the endpoint information.
        /// </summary>
        /// <returns>The endpoint information class.</returns>
        EndpointInfo getInfo();
    }

    [System.Serializable]
    public abstract class IPEndpointInfo : EndpointInfo
    {
        public string host;
        public int port;
        public string sourceAddress;

        protected IPEndpointInfo()
        {
            this.host = "";
            this.sourceAddress = "";
        }

        protected IPEndpointInfo(EndpointInfo underlying,
                                 int timeout,
                                 bool compress,
                                 string host,
                                 int port,
                                 string sourceAddress) : base(underlying, timeout, compress)
        {
            this.host = host;
            this.port = port;
            this.sourceAddress = sourceAddress;
        }
    }

    [System.Serializable]
    public abstract partial class TCPEndpointInfo : IPEndpointInfo
    {
        protected TCPEndpointInfo()
        {
        }

        protected TCPEndpointInfo(EndpointInfo underlying,
                                  int timeout,
                                  bool compress,
                                  string host,
                                  int port,
                                  string sourceAddress) : base(underlying, timeout, compress, host, port, sourceAddress)
        {
        }
    }

    [System.Serializable]
    public abstract partial class UDPEndpointInfo : IPEndpointInfo
    {
        public string mcastInterface;
        public int mcastTtl;

        protected UDPEndpointInfo() : base()
        {
            mcastInterface = "";
        }

        protected UDPEndpointInfo(EndpointInfo underlying,
                                  int timeout,
                                  bool compress,
                                  string host,
                                  int port,
                                  string sourceAddress,
                                  string mcastInterface,
                                  int mcastTtl) : base(underlying, timeout, compress, host, port, sourceAddress)
        {
            this.mcastInterface = mcastInterface;
            this.mcastTtl = mcastTtl;
        }
    }

    [System.Serializable]
    public abstract partial class WSEndpointInfo : EndpointInfo
    {
        public string resource;

        protected WSEndpointInfo()
        {
            this.resource = "";
        }

        protected WSEndpointInfo(EndpointInfo underlying,
                                 int timeout,
                                 bool compress,
                                 string resource) : base(underlying, timeout, compress)
        {
            this.resource = resource;
        }
    }

    [System.Serializable]
    public abstract partial class OpaqueEndpointInfo : EndpointInfo
    {
        public EncodingVersion rawEncoding;
        public byte[] rawBytes;

        protected OpaqueEndpointInfo()
        {
            this.rawEncoding = new EncodingVersion();
        }

        protected OpaqueEndpointInfo(EndpointInfo? underlying,
                                     int timeout,
                                     bool compress,
                                     EncodingVersion rawEncoding,
                                     byte[] rawBytes) : base(underlying, timeout, compress)
        {
            this.rawEncoding = rawEncoding;
            this.rawBytes = rawBytes;
        }
    }
}
