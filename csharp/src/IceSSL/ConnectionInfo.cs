//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    public class ConnectionInfo : Ice.ConnectionInfo
    {
        public string? Cipher;
        public X509Certificate2[]? Certs;
        public bool Verified;

        public ConnectionInfo() => Cipher = "";

        public ConnectionInfo(Ice.ConnectionInfo underlying,
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
