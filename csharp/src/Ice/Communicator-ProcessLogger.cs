//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    public sealed partial class Communicator
    {
        private static ILogger? _processLogger = null;
        private static readonly object _processLoggerMutex = new object();

        /// <summary>Returns the process-wide logger.</summary>
        /// <returns>The process-wide logger.</returns>
        public static ILogger GetProcessLogger()
        {
            lock (_processLoggerMutex)
            {
                if (_processLogger == null)
                {
                    _processLogger = new ConsoleLogger(AppDomain.CurrentDomain.FriendlyName);
                }
                return _processLogger;
            }
        }

        /// <summary>Changes the process-wide logger.</summary>
        /// <param name="logger">The new process-wide logger.</param>
        public static void SetProcessLogger(ILogger logger)
        {
            lock (_processLoggerMutex)
            {
                _processLogger = logger;
            }
        }
    }
}
