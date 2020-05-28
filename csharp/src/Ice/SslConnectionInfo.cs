//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    public class SslConnectionInfo : ConnectionInfo
    {
        public string? Cipher;
        public X509Certificate2[]? Certs;
        public bool Verified;

        public SslConnectionInfo() => Cipher = "";

        public SslConnectionInfo(ConnectionInfo underlying,
                              bool incoming,
                              string adapterName,
                              string connectionId,
                              string cipher,
                              X509Certificate2[] certs,
                              bool verified)
            : base(underlying, incoming, adapterName, connectionId)
        {
            Cipher = cipher;
            Certs = certs;
            Verified = verified;
        }
    }
}
