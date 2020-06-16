//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    public class SslConnectionInfo : TcpConnectionInfo
    {
        public string? Cipher;
        public X509Certificate2[]? Certs;
        public bool Verified;

        public SslConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receivedSize,
            int sendSize,
            string? cipher,
            X509Certificate2[]? certs,
            bool verified)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort,
                  receivedSize, sendSize)
        {
            Cipher = cipher;
            Certs = certs;
            Verified = verified;
        }
    }

    public class WssConnectionInfo : SslConnectionInfo
    {
        public Dictionary<string, string>? Headers;

        public WssConnectionInfo(
            string adapterName,
            string connectionId,
            bool incoming,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int receivedSize,
            int sendSize,
            string? cipher,
            X509Certificate2[]? certs,
            bool verified,
            Dictionary<string, string>? headers)
            : base(adapterName, connectionId, incoming, localAddress, localPort, remoteAddress, remotePort,
                  receivedSize, sendSize, cipher, certs, verified) => Headers = headers;
    }
}
