//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Security.Cryptography.X509Certificates;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    /// <summary>Base class providing access to the connection details.</summary>
    [Serializable]
    public class ConnectionInfo
    {
        /// <summary>Gets the name of the adapter associated with the connection if there is one otherwise return an
        /// empty string.</summary>
        public string AdapterName { get; }

        /// <summary>Gets the connection id of the endpoint associated with this connection if there is one otherwise
        /// return an empty string</summary>
        public string ConnectionId { get; }

        /// <summary>Gets whether or not the connection is an incoming connection.</summary>
        public bool Incoming { get; }

        protected ConnectionInfo(string adapterName, string connectionId, bool incoming)
        {
            AdapterName = adapterName;
            ConnectionId = connectionId;
            Incoming = incoming;
        }
    }

    /// <summary>Provides access to the connection details of an IP connection.</summary>
    [Serializable]
    public class IpConnectionInfo : ConnectionInfo
    {
        /// <summary>Gets the connection local IP address or an empty string if the local address is not available.
        /// </summary>
        public string LocalAddress { get; }
        /// <summary>Gets the connection local IP port or -1 if the local IP port is not available.</summary>
        public int LocalPort { get; }
        /// <summary>Gets the connection remote IP address or an empty string if the remote address is not available.
        /// </summary>
        public string RemoteAddress { get; }

        /// <summary>Gets the connection buffer receive size.</summary>
        public int ReceiveSize { get; }
        /// <summary>Gets the connection remote IP port or -1 if the remote IP port is not available.</summary>
        public int RemotePort { get; }
        /// <summary>Gets the connection buffer send size.</summary>
        public int SendSize { get; }

        protected IpConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receiveSize,
            int sendSize)
            : base(adapterName, connectionId, incoming)
        {
            LocalAddress = localAddress;
            LocalPort = localPort;
            RemoteAddress = remoteAddress;
            RemotePort = remotePort;
            ReceiveSize = receiveSize;
            SendSize = sendSize;
        }

        protected IpConnectionInfo(string adapterName, string connectionId, bool incoming)
            : this(adapterName, connectionId, incoming, "", -1, "", -1, 0, 0)
        {
        }
    }

    /// <summary>Provides access to the connection details of a TCP connection.</summary>
    [Serializable]
    public class TcpConnectionInfo : IpConnectionInfo
    {
        protected internal TcpConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receiveSize,
            int sendSize)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort,
                  receiveSize, sendSize)
        {
        }

        protected internal TcpConnectionInfo(string adapterName, string connectionId, bool incoming)
            : base(adapterName, connectionId, incoming)
        {
        }
    }

    /// <summary>Provides access to the connection details of a UDP connection.</summary>
    [Serializable]
    public class UdpConnectionInfo : IpConnectionInfo
    {
        /// <summary>Gets the multicast IP address associated with this connection or an empty string if there ins't
        /// one.</summary>
        public string McastAddress { get; }
        /// <summary>Gets the multicast IP port associated with this connection or -1 if there isn't one.</summary>
        public int McastPort { get; }

        protected internal UdpConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receiveSize,
            int sendSize,
            string mcastAddress,
            int mcastPort)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort,
                  receiveSize, sendSize)
        {
            McastAddress = mcastAddress;
            McastPort = mcastPort;
        }
    }

    /// <summary>Provides access to the connection details of a WebSocket connection.</summary>
    [Serializable]
    public class WsConnectionInfo : TcpConnectionInfo
    {
        /// <summary>Gets the HTTP upgrade request headers.</summary>
        public Dictionary<string, string> Headers { get; }

        protected internal WsConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receiveSize,
            int sendSize,
            Dictionary<string, string> headers)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort, receiveSize,
                sendSize) => Headers = headers;
    }

    /// <summary>Provides access to the connection details of an TLS connection.</summary>
    [Serializable]
    public class SslConnectionInfo : TcpConnectionInfo
    {
        /// <summary>Gets the TLS cipher suite negotiated for the connection.</summary>
        public string Cipher { get; }
        /// <summary>Gets the certificate chain provided by the peer.</summary>
        public X509Certificate2[]? Certs { get; }

        protected internal SslConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receiveSize,
            int sendSize,
            string cipher,
            X509Certificate2[]? certs)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort,
                  receiveSize, sendSize)
        {
            Cipher = cipher;
            Certs = certs;
        }
    }

    /// <summary>Provides access to the connection details of a secure WebSocket connection.</summary>
    [Serializable]
    public class WssConnectionInfo : SslConnectionInfo
    {
        /// <summary>Gets the HTTP upgrade request headers.</summary>
        public Dictionary<string, string> Headers { get; }

        protected internal WssConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receivedSize,
            int sendSize,
            string cipher,
            X509Certificate2[]? certs,
            Dictionary<string, string> headers)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort,
                  receivedSize, sendSize, cipher, certs) => Headers = headers;
    }
}
