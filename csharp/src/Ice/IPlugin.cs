//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    public partial interface IPlugin
    {
        /// <summary>
        /// Perform any necessary initialization steps.
        /// </summary>
        void Initialize();

        /// <summary>
        /// Called when the communicator is being destroyed.
        /// </summary>
        void Destroy();
    }
}
