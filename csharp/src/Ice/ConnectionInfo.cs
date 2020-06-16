//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    [Serializable]
    public class ConnectionInfo
    {
        public string? AdapterName { get; }
        public string ConnectionId { get; }
        public bool Incoming { get; }

        public ConnectionInfo(string adapterName, string connectionId, bool incoming)
        {
            AdapterName = adapterName;
            ConnectionId = connectionId;
            Incoming = incoming;
        }
    }

    [Serializable]
    public class IpConnectionInfo : ConnectionInfo
    {
        public string LocalAddress { get; }
        public int LocalPort { get; }
        public string RemoteAddress { get; }
        public int RemotePort { get; }

        public IpConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort)
            : base(adapterName, connectionId, incoming)
        {
            LocalAddress = localAddress;
            LocalPort = localPort;
            RemoteAddress = remoteAddress;
            RemotePort = remotePort;
        }
    }

    [Serializable]
    public class TcpConnectionInfo : IpConnectionInfo
    {
        public int ReceivedSize;
        public int SendSize;

        public TcpConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receivedSize,
            int sendSize)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort)
        {
            ReceivedSize = receivedSize;
            SendSize = sendSize;
        }

        public TcpConnectionInfo(string adapterName, string connectionId, bool incoming)
            : this(adapterName, connectionId, incoming, "", 0, "", 0, 0, 0)
        {
        }
    }

    [Serializable]
    public class UdpConnectionInfo : IpConnectionInfo
    {
        public string McastAddress { get; }
        public int McastPort { get; }
        public int ReceivedSize { get; }
        public int SendSize { get; }

        public UdpConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int rcvSize,
            int sndSize,
            string mcastAddress,
            int mcastPort)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort)
        {
            McastAddress = mcastAddress;
            McastPort = mcastPort;
            ReceivedSize = rcvSize;
            SendSize = sndSize;
        }
    }

    [Serializable]
    public class WsConnectionInfo : TcpConnectionInfo
    {
        public Dictionary<string, string>? Headers;

        public WsConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receivedSize,
            int sendSize,
            Dictionary<string, string> headers)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort, receivedSize,
                sendSize) => Headers = headers;
    }
}
