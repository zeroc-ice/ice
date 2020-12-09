// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A plug-in adds a feature to a communicator, such as support for a transport. The plugins are created
    /// when the communicator is constructed, and are later activated asynchronously when the communicator is
    /// activated asynchronously.</summary>
    public interface IPlugin : IAsyncDisposable
    {
        /// <summary>Performs any necessary activation steps.</summary>
        /// <param name="cancel">The cancellation token.</param>
        /// <returns>A task that completes once the activation completes.</returns>
        Task ActivateAsync(CancellationToken cancel);
    }
}
