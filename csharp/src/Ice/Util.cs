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
    /// Interface for thread notification hooks. Applications can derive
    /// a class tat implements the start and stop
    /// methods to intercept creation and destruction of threads created
    /// by the Ice run time.
    /// </summary>
    public interface ThreadNotification
    {
        /// <summary>
        /// The Ice run time calls start for each new
        /// thread it creates. The call is made by the newly-started thread.
        /// </summary>
        void start();

        /// <summary>
        /// The Ice run time calls stop before it destroys
        /// a thread. The call is made by thread that is about to be
        /// destroyed.
        /// </summary>
        void stop();
    }

    /// <summary>
    /// A class that encpasulates data to initialize a communicator.
    /// </summary>
    public class InitializationData : ICloneable
    {
        /// <summary>
        /// Creates and returns a copy of this object.
        /// </summary>
        public object Clone()
        {
            //
            // A member-wise copy is safe because the members are immutable.
            //
            return MemberwiseClone();
        }

        /// <summary>
        /// The properties for the communicator.
        /// </summary>
        public Properties? properties;

        /// <summary>
        /// The logger for the communicator.
        /// </summary>
        public Logger? logger;

        /// <summary>
        /// The communicator observer used by the Ice run-time.
        /// </summary>
        public Instrumentation.CommunicatorObserver? observer;

        /// <summary>
        /// The thread start hook for the communicator. The Ice run time
        /// calls this hook for each new thread it creates. The call is
        /// made by the newly-started thread.
        /// </summary>
        public Action? threadStart;

        /// <summary>
        /// The thread stop hook for the communicator. The Ice run time
        /// calls stop before it destroys a thread. The call is made by
        /// thread that is about to be destroyed.
        /// </summary>
        public Action? threadStop;

        /// <summary>
        /// The dispatcher for the communicator.
        /// </summary>
        public Action<Action, Connection?>? dispatcher;

        /// <summary>
        /// The compact type ID resolver.
        /// </summary>
        public Func<int, string>? compactIdResolver;

        /// <summary>
        /// The list of TypeId namespaces. Default is Ice.TypeId.
        /// </summary>
        public string[] typeIdNamespaces = { "Ice.TypeId" };
    }

    /// <summary>
    /// Utility methods for the Ice run time.
    /// </summary>
    public sealed class Util
    {
        /// <summary>
        /// Creates a new empty property set.
        /// </summary>
        /// <returns>A new empty property set.</returns>
        public static Properties createProperties()
        {
            return new Properties();
        }

        /// <summary>
        /// Creates a property set initialized from an argument vector.
        /// </summary>
        /// <param name="args">A command-line argument vector, possibly containing
        /// options to set properties. If the command-line options include
        /// a --Ice.Config option, the corresponding configuration
        /// files are parsed. If the same property is set in a configuration
        /// file and in the argument vector, the argument vector takes precedence.
        /// This method modifies the argument vector by removing any Ice-related options.</param>
        /// <returns>A property set initialized with the property settings
        /// that were removed from args.</returns>
        public static Properties createProperties(ref string[] args)
        {
            return new Properties(ref args, null);
        }

        /// <summary>
        /// Creates a property set initialized from an argument vector.
        /// </summary>
        /// <param name="args">A command-line argument vector, possibly containing
        /// options to set properties. If the command-line options include
        /// a --Ice.Config option, the corresponding configuration
        /// files are parsed. If the same property is set in a configuration
        /// file and in the argument vector, the argument vector takes precedence.
        /// This method modifies the argument vector by removing any Ice-related options.</param>
        /// <param name="defaults">Default values for the property set. Settings in configuration
        /// files and args override these defaults.</param>
        /// <returns>A property set initialized with the property settings
        /// that were removed from args.</returns>
        public static Properties createProperties(ref string[] args, Properties? defaults)
        {
            return new Properties(ref args, defaults);
        }

        /// <summary>
        /// Creates a communicator.
        /// </summary>
        /// <param name="args">A command-line argument vector. Any Ice-related options
        /// in this vector are used to intialize the communicator.
        /// This method modifies the argument vector by removing any Ice-related options.</param>
        /// <returns>The initialized communicator.</returns>
        public static Communicator initialize(ref string[] args)
        {
            return initialize(ref args, (InitializationData?)null);
        }

        /// <summary>
        /// Creates a communicator.
        /// </summary>
        /// <param name="args">A command-line argument vector. Any Ice-related options
        /// in this vector are used to initialize the communicator.
        /// This method modifies the argument vector by removing any Ice-related options.</param>
        /// <param name="initData">Additional intialization data. Property settings in args
        /// override property settings in initData.</param>
        /// <returns>The initialized communicator.</returns>
        public static Communicator initialize(ref string[] args, InitializationData? initData)
        {
            if (initData == null)
            {
                initData = new InitializationData();
            }
            else
            {
                initData = (InitializationData)initData.Clone();
            }

            initData.properties = createProperties(ref args, initData.properties);

            return new Communicator(initData, ref args);
        }

        /// <summary>
        /// Creates a communicator.
        /// </summary>
        /// <param name="args">A command-line argument vector. Any Ice-related options
        /// in this vector are used to initialize the communicator.
        /// This method modifies the argument vector by removing any Ice-related options.</param>
        /// <param name="configFile">Path to a config file that sets the new communicator's default
        /// properties.</param>
        /// <returns>The initialized communicator.</returns>
        public static Communicator initialize(ref string[] args, string configFile)
        {
            InitializationData? initData = null;
            if (configFile != null)
            {
                initData = new InitializationData();
                initData.properties = Util.createProperties();
                initData.properties.load(configFile);
            }
            return initialize(ref args, initData);
        }

        /// <summary>
        /// Creates a communicator.
        /// </summary>
        /// <param name="initData">Additional intialization data.</param>
        /// <returns>The initialized communicator.</returns>
        public static Communicator initialize(InitializationData? initData)
        {
            if (initData == null)
            {
                initData = new InitializationData();
            }
            else
            {
                initData = (InitializationData)initData.Clone();
            }
            return new Communicator(initData);
        }

        /// <summary>
        /// Creates a communicator.
        /// </summary>
        /// <param name="configFile">Path to a config file that sets the new communicator's default
        /// properties.</param>
        /// <returns>The initialized communicator.</returns>
        public static Communicator initialize(string configFile)
        {
            InitializationData? initData = null;
            if (configFile != null)
            {
                initData = new InitializationData();
                initData.properties = createProperties();
                initData.properties.load(configFile);
            }
            return initialize(initData);
        }

        /// <summary>
        /// Creates a communicator using a default configuration.
        /// </summary>
        public static Communicator initialize()
        {
            return initialize((InitializationData?)null);
        }

        /// <summary>
        /// Compares the object identities of two proxies.
        /// </summary>
        /// <param name="lhs">A proxy.</param>
        /// <param name="rhs">A proxy.</param>
        /// <returns>-1 if the identity in lhs compares
        /// less than the identity in rhs; 0 if the identities
        /// compare equal; 1, otherwise.</returns>
        public static int proxyIdentityCompare(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return 0;
            }

            if (lhs == null)
            {
                return -1;
            }

            if (rhs == null)
            {
                return 1;
            }

            Identity lhsIdentity = lhs.Identity;
            Identity rhsIdentity = rhs.Identity;
            int n;
            n = string.CompareOrdinal(lhsIdentity.name, rhsIdentity.name);
            if (n != 0)
            {
                return n;
            }
            return string.CompareOrdinal(lhsIdentity.category, rhsIdentity.category);
        }

        /// <summary>
        /// Compares the object identities and facets of two proxies.
        /// </summary>
        /// <param name="lhs">A proxy.</param>
        /// <param name="rhs">A proxy.</param>
        /// <returns>-1 if the identity and facet in lhs compare
        /// less than the identity and facet in rhs; 0 if the identities
        /// and facets compare equal; 1, otherwise.</returns>
        public static int proxyIdentityAndFacetCompare(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return 0;
            }

            if (lhs == null && rhs != null)
            {
                return -1;
            }

            if (lhs != null && rhs == null)
            {
                return 1;
            }

            Identity lhsIdentity = lhs!.Identity;
            Identity rhsIdentity = rhs!.Identity;
            int n;
            n = string.CompareOrdinal(lhsIdentity.name, rhsIdentity.name);
            if (n != 0)
            {
                return n;
            }
            n = string.CompareOrdinal(lhsIdentity.category, rhsIdentity.category);
            if (n != 0)
            {
                return n;
            }
            return string.CompareOrdinal(lhs.Facet, rhs.Facet);
        }

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
                throw new VersionParseException("malformed version value `" + str + "'");
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
                throw new VersionParseException("invalid version value `" + str + "'");
            }

            if (majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
            {
                throw new VersionParseException("range error in version `" + str + "'");
            }

            major = (byte)majVersion;
            minor = (byte)minVersion;
        }

        private static string majorMinorToString(byte major, byte minor)
        {
            return string.Format("{0}.{1}", major, minor);
        }

        public static void registerPluginFactory(string name, PluginFactory factory, bool loadOnInit)
        {
            PluginManagerI.registerPluginFactory(name, factory, loadOnInit);
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

        public static ThreadPriority stringToThreadPriority(string s)
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
