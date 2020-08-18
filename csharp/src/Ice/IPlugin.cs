//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    public interface IPlugin : IAsyncDisposable
    {
        /// <summary>Perform any necessary initialization steps.</summary>
        /// <param name="context">The plug-in initialization context enables the registration of invocation and dispatch
        /// interceptors, it can also be use to set the communicator logger.</param>
        void Initialize(PluginInitializationContext context);
    }
}
