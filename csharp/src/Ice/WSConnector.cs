// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Net;

namespace ZeroC.Ice
{
    internal sealed class WSConnector : TcpConnector
    {
        private readonly int _hashCode;

        private string Resource => ((WSEndpoint)Endpoint).Resource;

        public override bool Equals(object? obj) =>
            base.Equals(obj) && obj is WSConnector wsConnector && Resource == wsConnector.Resource;

        public override int GetHashCode() => _hashCode;

        internal WSConnector(WSEndpoint endpoint, EndPoint addr, INetworkProxy? proxy)
            : base(endpoint, addr, proxy) => _hashCode = HashCode.Combine(base.GetHashCode(), endpoint["resource"]);
    }
}
