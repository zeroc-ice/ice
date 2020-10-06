// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    /// <summary>A communicator plug-in, a plug-in generally adds a feature to a communicator, such as support for a
    /// protocol. The communicator loads its plug-ins in two stages: the first stage creates the plug-ins, and the
    /// second stage invokes Plugin#initialize on each one.</summary>
    public interface IPlugin : IAsyncDisposable
    {
        /// <summary>Perform any necessary initialization steps.</summary>
        /// <param name="context">The plug-in initialization context enables the registration of invocation and dispatch
        /// interceptors, it can also be use to set the communicator logger.</param>
        void Initialize(PluginInitializationContext context);
    }
}
