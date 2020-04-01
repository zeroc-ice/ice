//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//

namespace Ice
{
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

    [System.Serializable]
    public abstract class EndpointInfo
    {
        public EndpointInfo? Underlying;
        public int Timeout;
        public bool Compress;

        /// <summary>
        /// Returns the type of the endpoint.
        /// </summary>
        /// <returns>The endpoint type.</returns>
        public abstract EndpointType Type();

        /// <summary>
        /// Returns true if this endpoint is a datagram endpoint.
        /// </summary>
        /// <returns>True for a datagram endpoint.</returns>
        public abstract bool Datagram();

        /// <summary>
        /// Returns true if this endpoint is a secure endpoint.
        /// </summary>
        /// <returns>True for a secure endpoint.</returns>
        public abstract bool Secure();

        protected EndpointInfo()
        {
        }

        protected EndpointInfo(EndpointInfo? underlying, int timeout, bool compress)
        {
            Underlying = underlying;
            Timeout = timeout;
            Compress = compress;
        }
    }

    public interface IEndpoint
    {
        /// <summary>
        /// Returns the endpoint information.
        /// </summary>
        /// <returns>The endpoint information class.</returns>
        EndpointInfo GetInfo();
    }

    [System.Serializable]
    public abstract class IPEndpointInfo : EndpointInfo
    {
        public string Host;
        public int Port;
        public string SourceAddress;

        protected IPEndpointInfo()
        {
            Host = "";
            SourceAddress = "";
        }

        protected IPEndpointInfo(EndpointInfo underlying,
                                 int timeout,
                                 bool compress,
                                 string host,
                                 int port,
                                 string sourceAddress) : base(underlying, timeout, compress)
        {
            Host = host;
            Port = port;
            SourceAddress = sourceAddress;
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
        public string McastInterface;
        public int McastTtl;

        protected UDPEndpointInfo() : base() => McastInterface = "";

        protected UDPEndpointInfo(EndpointInfo underlying,
                                  int timeout,
                                  bool compress,
                                  string host,
                                  int port,
                                  string sourceAddress,
                                  string mcastInterface,
                                  int mcastTtl) : base(underlying, timeout, compress, host, port, sourceAddress)
        {
            McastInterface = mcastInterface;
            McastTtl = mcastTtl;
        }
    }

    [System.Serializable]
    public abstract partial class WSEndpointInfo : EndpointInfo
    {
        public string Resource;

        protected WSEndpointInfo() => Resource = "";

        protected WSEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string resource) :
            base(underlying, timeout, compress) => Resource = resource;
    }

    [System.Serializable]
    public abstract partial class OpaqueEndpointInfo : EndpointInfo
    {
        public Encoding RawEncoding;
        public byte[]? RawBytes;

        protected OpaqueEndpointInfo() => RawEncoding = default;

        protected OpaqueEndpointInfo(EndpointInfo? underlying,
                                     int timeout,
                                     bool compress,
                                     Encoding rawEncoding,
                                     byte[] rawBytes) : base(underlying, timeout, compress)
        {
            RawEncoding = rawEncoding;
            RawBytes = rawBytes;
        }
    }
}
