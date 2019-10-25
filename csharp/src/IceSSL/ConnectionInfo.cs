//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    public partial class ConnectionInfo : global::Ice.ConnectionInfo
    {
        public string cipher;
        public System.Security.Cryptography.X509Certificates.X509Certificate2[] certs;
        public bool verified;
        partial void ice_initialize();

        public ConnectionInfo() : base()
        {
            this.cipher = "";
            ice_initialize();
        }

        public ConnectionInfo(global::Ice.ConnectionInfo underlying, bool incoming, string adapterName,
                              string connectionId, string cipher,
                              System.Security.Cryptography.X509Certificates.X509Certificate2[] certs,
                              bool verified) : base(underlying, incoming, adapterName, connectionId)
        {
            this.cipher = cipher;
            this.certs = certs;
            this.verified = verified;
            ice_initialize();
        }
    }
}
