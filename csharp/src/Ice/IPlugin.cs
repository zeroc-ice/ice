// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A plug-in adds a feature to a communicator, such as support for a transport. The plug-ins are created
    /// when the communicator is constructed, and are later activated asynchronously when the communicator is
    /// activated asynchronously.</summary>
    public interface IPlugin : IAsyncDisposable
    {
        /// <summary>Performs any necessary activation steps.</summary>
        /// <param name="cancel">The cancellation token.</param>
        /// <returns>A task that completes once the activation completes.</returns>
        Task ActivateAsync(CancellationToken cancel);
    }

    /// <summary>Applications implement this interface to provide a plug-in factory to the Ice runtime.</summary>
    public interface IPluginFactory
    {
        /// <summary>Creates a new plug-in.</summary>
        /// <param name="communicator">The communicator being constructed.</param>
        /// <param name="name">The name of the plug-in.</param>
        /// <param name="args">The arguments specified in the plug-in configuration.</param>
        /// <returns>The plug-in created by this factory.</returns>
        IPlugin Create(Communicator communicator, string name, string[] args);
    }
}
