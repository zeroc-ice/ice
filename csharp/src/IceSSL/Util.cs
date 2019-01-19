//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    using System;
    using System.Diagnostics;
    using System.Security.Cryptography.X509Certificates;

    public sealed class Util
    {
        public static void
        registerIceSSL(bool loadOnInitialize)
        {
            Ice.Util.registerPluginFactory("IceSSL", new PluginFactory(), loadOnInitialize);
        }
    }
}
