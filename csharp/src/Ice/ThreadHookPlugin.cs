// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    /// <summary>
    /// Class to support thread notification hooks. Applications using 
    /// thread notification hooks instantiate a ThreadHookPlugin with a
    /// thread notification hook  and return the instance from their
    /// PluginFactory implementation.
    /// </summary>
    public class ThreadHookPlugin : Ice.Plugin
    {
        /// <summary>
        /// Installs a custom logger for a communicator.
        /// </summary>
        /// <param name="communicator">The communicator using the thread notification hook.</param>
        /// <param name="threadHook">The thread notification hook for the communicator.</param>
        public 
        ThreadHookPlugin(Communicator communicator, ThreadNotification threadHook)
        {
            if(communicator == null)
            {
                PluginInitializationException ex = new PluginInitializationException();
                ex.reason = "Communicator cannot be null";
                throw ex;
            }
    
            IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
            instance.setThreadHook(threadHook);
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
