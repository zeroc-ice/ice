// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Net;

namespace ZeroC.Ice
{
    internal sealed class WSConnector : TcpConnector
    {
        private readonly int _hashCode;

        public override bool Equals(object? obj)
        {
            if (base.Equals(obj) && obj is WSConnector wsConnector)
            {
                return ((WSEndpoint)Endpoint).Resource == ((WSEndpoint)wsConnector.Endpoint).Resource;
            }
            else
            {
                return false;
            }
        }

        public override int GetHashCode() => _hashCode;

        internal WSConnector(WSEndpoint endpoint, EndPoint addr, INetworkProxy? proxy)
            : base(endpoint, addr, proxy) => _hashCode = HashCode.Combine(base.GetHashCode(), endpoint["resource"]);
    }
}
