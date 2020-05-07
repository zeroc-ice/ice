//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    [Serializable]
    public class ConnectionInfo
    {
        public ConnectionInfo? Underlying;
        public bool Incoming;
        public string? AdapterName;
        public string ConnectionId;

        public ConnectionInfo()
        {
            AdapterName = "";
            ConnectionId = "";
        }

        public ConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId)
        {
            Underlying = underlying;
            Incoming = incoming;
            AdapterName = adapterName;
            ConnectionId = connectionId;
        }
    }

    [Serializable]
    public class IPConnectionInfo : ConnectionInfo
    {
        public string LocalAddress;
        public int LocalPort;
        public string RemoteAddress;
        public int RemotePort;

        public IPConnectionInfo()
        {
            LocalAddress = "";
            LocalPort = -1;
            RemoteAddress = "";
            RemotePort = -1;
        }

        public IPConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
                                string localAddress, int localPort, string remoteAddress, int remotePort)
            : base(underlying, incoming, adapterName, connectionId)
        {
            LocalAddress = localAddress;
            LocalPort = localPort;
            RemoteAddress = remoteAddress;
            RemotePort = remotePort;
        }
    }

    [Serializable]
    public class TCPConnectionInfo : IPConnectionInfo
    {
        public int RcvSize;
        public int SndSize;

        public TCPConnectionInfo()
        {
            RcvSize = 0;
            SndSize = 0;
        }

        public TCPConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
                                 string localAddress, int localPort, string remoteAddress, int remotePort, int rcvSize,
                                 int sndSize)
            : base(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
        {
            RcvSize = rcvSize;
            SndSize = sndSize;
        }
    }

    [Serializable]
    public class UDPConnectionInfo : IPConnectionInfo
    {
        public string McastAddress;
        public int McastPort;
        public int RcvSize;
        public int SndSize;

        public UDPConnectionInfo()
        {
            McastAddress = "";
            McastPort = -1;
            RcvSize = 0;
            SndSize = 0;
        }

        public UDPConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
                                 string localAddress, int localPort, string remoteAddress, int remotePort,
                                 string mcastAddress, int mcastPort, int rcvSize, int sndSize)
            : base(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
        {
            McastAddress = mcastAddress;
            McastPort = mcastPort;
            RcvSize = rcvSize;
            SndSize = sndSize;
        }
    }

    [Serializable]
    public class WSConnectionInfo : ConnectionInfo
    {
        public Dictionary<string, string>? Headers;

        public WSConnectionInfo()
        {
        }

        public WSConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
                                Dictionary<string, string> headers)
            : base(underlying, incoming, adapterName, connectionId) => Headers = headers;
    }
}
