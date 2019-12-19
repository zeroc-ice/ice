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
            Ice.Communicator.RegisterPluginFactory("IceSSL", new PluginFactory(), loadOnInitialize);
        }
    }
}
