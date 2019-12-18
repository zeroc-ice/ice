//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>
    /// Class to support thread notification hooks. Applications using
    /// thread notification hooks instantiate a ThreadHookPlugin with a
    /// thread notification hook  and return the instance from their
    /// PluginFactory implementation.
    /// </summary>
    public class ThreadHookPlugin : Plugin
    {
        /// <summary>
        /// Installs thread hooks for a communicator.
        /// </summary>
        /// <param name="communicator">The communicator using the thread notification hook.</param>
        /// <param name="threadStart">The start thread notification hook for the communicator.</param>
        /// <param name="threadStop">The stop thread notification hook for the communicator.</param>
        public
        ThreadHookPlugin(Communicator communicator, System.Action threadStart, System.Action threadStop)
        {
            if (communicator == null)
            {
                PluginInitializationException ex = new PluginInitializationException();
                ex.reason = "Communicator cannot be null";
                throw ex;
            }
            communicator.setThreadHook(threadStart, threadStop);
        }

        /// <summary>
        /// Called by the Ice run time during communicator initialization. The derived class
        /// can override this method to perform any initialization that might be required
        /// by the thread notification hook.
        /// </summary>
        public void
        initialize()
        {
        }

        /// <summary>
        /// Called by the Ice run time when the communicator is destroyed. The derived class
        /// can override this method to perform any finalization that might be required
        /// by thread notification hook.
        /// </summary>
        public void
        destroy()
        {
        }
    }
}
