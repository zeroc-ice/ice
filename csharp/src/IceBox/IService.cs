// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.IceBox
{
    /// <summary>An application service created, started and stopped by the IceBox service manager. The IceBox service
    /// manager serializes calls to StartAsync and StopAsync. For example, it will not call StartAsync on a service
    /// when a StartAsync is already in progress for that service.</summary>
    public interface IService
    {
        /// <summary>Starts the service.</summary>
        /// <param name="name">The service's name, as determined by the configuration.</param>
        /// <param name="communicator">A communicator for use by the service. This communicator is created and destroyed
        /// by the IceBox service manager.</param>
        /// <param name="args">The service arguments.</param>
        /// <param name="cancel">The cancellation token.</param>
        /// <return>A task that completes when the service is started.</return>
        /// <remarks>If StartAsync throws an exception, the service manager will consider the service to be stopped.
        /// An exception thrown during the IceBox startup will cause IceBox to exit.</remarks>
        Task StartAsync(string name, Ice.Communicator communicator, string[] args, CancellationToken cancel);

        /// <summary>Stops the service.</summary>
        /// <return>A task that completes once the service is stopped.</return>
        /// <remarks>If StopAsync throws an exception, the service manager will consider the service to remain in the
        /// started state.</remarks>
        Task StopAsync();
    }
}
