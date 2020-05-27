//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;

namespace ZeroC.Ice
{
    /// <summary>
    /// Utility methods for the Ice run time.
    /// </summary>
    public static class Util
    {
        /// <summary>
        /// Returns the process-wide logger.
        /// </summary>
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

        /// <summary>
        /// Changes the process-wide logger.
        /// </summary>
        /// <param name="logger">The new process-wide logger.</param>
        public static void SetProcessLogger(ILogger logger)
        {
            lock (_processLoggerMutex)
            {
                _processLogger = logger;
            }
        }

        /// <summary>
        /// Returns the Ice version in the form A.B.C, where A indicates the
        /// major version, B indicates the minor version, and C indicates the
        /// patch level.
        /// </summary>
        /// <returns>The Ice version.</returns>
        public static string StringVersion() => "4.0.0-alpha.0"; // "A.B.C", with A=major, B=minor, C=patch

        /// <summary>
        /// Returns the Ice version as an integer in the form A.BB.CC, where A
        /// indicates the major version, BB indicates the minor version, and CC
        /// indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
        /// </summary>
        /// <returns>The Ice version.</returns>
        public static int IntVersion() => 40000; // AABBCC, with AA=major, BB=minor, CC=patch

        private static readonly object _processLoggerMutex = new object();
        private static ILogger? _processLogger = null;

        public static ITransportPluginFacade GetTransportPluginFacade(Communicator communicator) =>
            new TransportPluginFacade(communicator);

        public static ThreadPriority StringToThreadPriority(string? s)
        {
            if (string.IsNullOrEmpty(s))
            {
                return ThreadPriority.Normal;
            }
            if (s.StartsWith("ThreadPriority.", StringComparison.Ordinal))
            {
                s = s.Substring("ThreadPriority.".Length, s.Length);
            }
            if (s.Equals("Lowest"))
            {
                return ThreadPriority.Lowest;
            }
            else if (s.Equals("BelowNormal"))
            {
                return ThreadPriority.BelowNormal;
            }
            else if (s.Equals("Normal"))
            {
                return ThreadPriority.Normal;
            }
            else if (s.Equals("AboveNormal"))
            {
                return ThreadPriority.AboveNormal;
            }
            else if (s.Equals("Highest"))
            {
                return ThreadPriority.Highest;
            }
            return ThreadPriority.Normal;
        }
    }
}
