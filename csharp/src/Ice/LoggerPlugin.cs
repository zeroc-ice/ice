// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    /// <summary>
    /// Class to support custom loggers. Applications using a custom logger
    /// instantiate a LoggerPlugin with a custom logger and
    /// return the instance from their PluginFactory implementation.
    /// </summary>
    public class LoggerPlugin : Ice.Plugin
    {
        /// <summary>
        /// Installs a custom logger for a communicator.
        /// </summary>
        /// <param name="communicator">The communicator using the custom logger.</param>
        /// <param name="logger">The custom logger for the communicator.</param>
        public 
        LoggerPlugin(Communicator communicator, Logger logger)
        {
            if(communicator == null)
            {
                PluginInitializationException ex = new PluginInitializationException();
                ex.reason = "Communicator cannot be null";
                throw ex;
            }
    
            if(logger == null)
            {
                PluginInitializationException ex = new PluginInitializationException();
                ex.reason = "Logger cannot be null";
                throw ex;
            }
    
            IceInternal.Instance instance = IceInternal.Util.getInstance(communicator);
            instance.setLogger(logger);
        }

        /// <summary>
        /// Called by the Ice run time during communicator initialization. The derived class
        /// can override this method to perform any initialization that might be required
        /// by a custom logger.
        /// </summary>
        public void 
        initialize()
        {
        }
    
        /// <summary>
        /// Called by the Ice run time when the communicator is destroyed. The derived class
        /// can override this method to perform any finalization that might be required
        /// by a custom logger.
        /// </summary>
        public void
        destroy()
        {
        }
    }
}
