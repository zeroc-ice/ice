//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    public sealed class Util
    {
        public static void
        registerIceSSL(bool loadOnInitialize)
        {
            Ice.Util.registerPluginFactory("IceSSL", new PluginFactory(), loadOnInitialize);
        }
    }
}
