//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
namespace Ice
{
    public partial interface Plugin
    {
        /// <summary>
        /// Perform any necessary initialization steps.
        /// </summary>
        void initialize();

        /// <summary>
        /// Called when the communicator is being destroyed.
        /// </summary>
        void destroy();
    }
}
