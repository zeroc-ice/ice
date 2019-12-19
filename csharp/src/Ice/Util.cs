//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Threading;

namespace Ice
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
        public static Logger getProcessLogger()
        {
            lock (_processLoggerMutex)
            {
                if (_processLogger == null)
                {
                    _processLogger = new ConsoleLoggerI(AppDomain.CurrentDomain.FriendlyName);
                }
                return _processLogger;
            }
        }

        /// <summary>
        /// Changes the process-wide logger.
        /// </summary>
        /// <param name="logger">The new process-wide logger.</param>
        public static void setProcessLogger(Logger logger)
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
        public static string stringVersion()
        {
            return "3.7.3"; // "A.B.C", with A=major, B=minor, C=patch
        }

        /// <summary>
        /// Returns the Ice version as an integer in the form A.BB.CC, where A
        /// indicates the major version, BB indicates the minor version, and CC
        /// indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
        /// </summary>
        /// <returns>The Ice version.</returns>
        public static int intVersion()
        {
            return 30703; // AABBCC, with AA=major, BB=minor, CC=patch
        }

        /// <summary>
        /// Converts a string to a protocol version.
        /// </summary>
        /// <param name="version">The string to convert.</param>
        /// <returns>The converted protocol version.</returns>
        public static ProtocolVersion stringToProtocolVersion(string version)
        {
            stringToMajorMinor(version, out byte major, out byte minor);
            return new ProtocolVersion(major, minor);
        }

        /// <summary>
        /// Converts a string to an encoding version.
        /// </summary>
        /// <param name="version">The string to convert.</param>
        /// <returns>The converted encoding version.</returns>
        public static EncodingVersion stringToEncodingVersion(string version)
        {
            stringToMajorMinor(version, out byte major, out byte minor);
            return new EncodingVersion(major, minor);
        }

        /// <summary>
        /// Converts a protocol version to a string.
        /// </summary>
        /// <param name="v">The protocol version to convert.</param>
        /// <returns>The converted string.</returns>
        public static string protocolVersionToString(Ice.ProtocolVersion v)
        {
            return majorMinorToString(v.major, v.minor);
        }

        /// <summary>
        /// Converts an encoding version to a string.
        /// </summary>
        /// <param name="v">The encoding version to convert.</param>
        /// <returns>The converted string.</returns>
        public static string encodingVersionToString(Ice.EncodingVersion v)
        {
            return majorMinorToString(v.major, v.minor);
        }

        private static void stringToMajorMinor(string str, out byte major, out byte minor)
        {
            int pos = str.IndexOf('.');
            if (pos == -1)
            {
                throw new FormatException($"malformed version value `{str}'");
            }

            string majStr = str.Substring(0, (pos) - (0));
            string minStr = str.Substring(pos + 1, (str.Length) - (pos + 1));
            int majVersion;
            int minVersion;
            try
            {
                majVersion = int.Parse(majStr, CultureInfo.InvariantCulture);
                minVersion = int.Parse(minStr, CultureInfo.InvariantCulture);
            }
            catch (FormatException)
            {
                throw new FormatException($"invalid version value `{str}'");
            }

            if (majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
            {
                throw new FormatException($"range error in version `{str}'");
            }

            major = (byte)majVersion;
            minor = (byte)minVersion;
        }

        private static string majorMinorToString(byte major, byte minor)
        {
            return string.Format("{0}.{1}", major, minor);
        }

        public static readonly ProtocolVersion currentProtocol =
            new ProtocolVersion(IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor);

        public static readonly EncodingVersion currentProtocolEncoding =
            new EncodingVersion(IceInternal.Protocol.protocolEncodingMajor,
                                IceInternal.Protocol.protocolEncodingMinor);

        public static readonly EncodingVersion currentEncoding =
            new EncodingVersion(IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor);

        public static readonly ProtocolVersion Protocol_1_0 = new ProtocolVersion(1, 0);

        public static readonly EncodingVersion Encoding_1_0 = new EncodingVersion(1, 0);
        public static readonly EncodingVersion Encoding_1_1 = new EncodingVersion(1, 1);

        private static readonly object _processLoggerMutex = new object();
        private static Logger? _processLogger = null;
    }
}

namespace IceInternal
{
    public sealed class HashUtil
    {
        public static void hashAdd(ref int hashCode, bool value)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
        }

        public static void hashAdd(ref int hashCode, short value)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ (int)(2654435761 * value));
        }

        public static void hashAdd(ref int hashCode, byte value)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ (int)(2654435761 * value));
        }

        public static void hashAdd(ref int hashCode, int value)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ (int)(2654435761 * value));
        }

        public static void hashAdd(ref int hashCode, long value)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
        }

        public static void hashAdd(ref int hashCode, float value)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
        }

        public static void hashAdd(ref int hashCode, double value)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
        }

        public static void hashAdd(ref int hashCode, object? value)
        {
            if (value != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
            }
        }

        public static void hashAdd<T>(ref int hashCode, T[]? arr)
        {
            if (arr != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ Ice.Collections.GetHashCode(arr));
            }
        }

        public static void hashAdd(ref int hashCode, IEnumerable? s)
        {
            if (s != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ Ice.Collections.GetHashCode(s));
            }
        }

        public static void hashAdd<Key, Value>(ref int hashCode, Dictionary<Key, Value>? d)
        {
            if (d != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ Ice.Collections.GetHashCode(d));
            }
        }
    }

    public sealed class Util
    {
        public static ProtocolPluginFacade getProtocolPluginFacade(Ice.Communicator communicator)
        {
            return new ProtocolPluginFacadeI(communicator);
        }

        public static ThreadPriority stringToThreadPriority(string? s)
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
