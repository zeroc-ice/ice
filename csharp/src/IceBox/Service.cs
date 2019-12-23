//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceBox
{
    public partial interface Service
    {
        /// <summary>
        /// Start the service.
        /// The given communicator is created by the
        /// ServiceManager for use by the service. This communicator may
        /// also be used by other services, depending on the service
        /// configuration.
        ///
        /// The ServiceManager owns this communicator, and is
        /// responsible for destroying it.
        ///
        /// </summary>
        /// <param name="name">The service's name, as determined by the
        /// configuration.
        ///
        /// </param>
        /// <param name="communicator">A communicator for use by the service.
        ///
        /// </param>
        /// <param name="args">The service arguments that were not converted into
        /// properties.
        ///
        /// </param>
        /// <exception name="FailureException">Raised if start failed.</exception>
        void start(string name, global::Ice.Communicator communicator, string[] args);

        /// <summary>
        /// Stop the service.
        /// </summary>
        void stop();
    }
}
