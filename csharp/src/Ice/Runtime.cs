// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    /// <summary>The Ice runtime.</summary>
    public static class Runtime
    {
        /// <summary>The process-wide logger.</summary>
        public static ILogger Logger
        {
            get
            {
                if (_processLogger == null)
                {
                    _processLogger = new ConsoleLogger(AppDomain.CurrentDomain.FriendlyName);
                }
                return _processLogger;
            }
            set => _processLogger = value;
        }

        /// <summary>Returns the Ice version as an integer in the form A.BB.CC, where A indicates the major version,
        /// BB indicates the minor version, and CC indicates the patch level. For example, for Ice 3.3.1, the returned
        /// value is 30301.</summary>
        /// <returns>The Ice version.</returns>
        public const int IntVersion = 40000; // AABBCC, with AA=major, BB=minor, CC=patch

        /// <summary>Returns the Ice version in the form A.B.C, where A indicates the major version, B indicates the
        /// minor version, and C indicates the patch level.</summary>
        /// <returns>The Ice version.</returns>
        public const string StringVersion = "4.0.0-alpha.0"; // "A.B.C", with A=major, B=minor, C=patch

        private static volatile ILogger? _processLogger;
    }
}
