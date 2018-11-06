// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
