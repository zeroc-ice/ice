// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Net;

namespace ZeroC.Ice
{
    internal sealed class WSConnector : TcpConnector
    {
        private int _hashCode;

        private string Resource => ((WSEndpoint)Endpoint).Resource;

        public override bool Equals(object? obj) =>
            base.Equals(obj) && obj is WSConnector wsConnector && Resource == wsConnector.Resource;

        public override int GetHashCode()
        {
            // This code is thread safe because reading/writing _hashCode (an int) is atomic.
            if (_hashCode != 0)
            {
                // Return cached value
                return _hashCode;
            }
            else
            {
                _hashCode = HashCode.Combine(base.GetHashCode(), Resource);
                return _hashCode;
            }
        }

        internal WSConnector(WSEndpoint endpoint, EndPoint addr, INetworkProxy? proxy)
            : base(endpoint, addr, proxy) => _hashCode = HashCode.Combine(base.GetHashCode(), endpoint["resource"]);
    }
}
