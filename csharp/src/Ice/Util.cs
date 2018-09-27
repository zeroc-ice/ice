// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;
using System.Collections;
using System.Globalization;

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
        public Properties properties;

        /// <summary>
        /// The logger for the communicator.
        /// </summary>
        public Logger logger;

        /// <summary>
        /// The communicator observer used by the Ice run-time.
        /// </summary>
        public Instrumentation.CommunicatorObserver observer;

        /// <summary>
        /// The thread hook for the communicator.
        /// </summary>
        [Obsolete("This data member is deprecated. Use threadStart or threadStop instead.")]
        public ThreadNotification threadHook;

        /// <summary>
        /// The thread start hook for the communicator. The Ice run time
        /// calls this hook for each new thread it creates. The call is
        /// made by the newly-started thread.
        /// </summary>
        public System.Action threadStart;

        /// <summary>
        /// The thread stop hook for the communicator. The Ice run time
        /// calls stop before it destroys a thread. The call is made by
        /// thread that is about to be destroyed.
        /// </summary>
        public System.Action threadStop;

        /// <summary>
        /// The dispatcher for the communicator.
        /// </summary>
        public System.Action<System.Action, Connection> dispatcher;

        /// <summary>
        /// The compact type ID resolver.
        /// </summary>
        public System.Func<int, string> compactIdResolver;

        /// <summary>
        /// The batch request interceptor.
        /// </summary>
        public System.Action<BatchRequest, int, int> batchRequestInterceptor;

        /// <summary>
        /// The value factory manager.
        /// </summary>
        public ValueFactoryManager valueFactoryManager;
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
            return new PropertiesI();
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
            return new PropertiesI(ref args, null);
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
        public static Properties createProperties(ref string[] args, Properties defaults)
        {
            return new PropertiesI(ref args, defaults);
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
            return initialize(ref args, (InitializationData)null);
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
        public static Communicator initialize(ref string[] args, InitializationData initData)
        {
            if(initData == null)
            {
                initData = new InitializationData();
            }
            else
            {
                initData = (InitializationData)initData.Clone();
            }

            initData.properties = createProperties(ref args, initData.properties);

            CommunicatorI result = new CommunicatorI(initData);
            result.finishSetup(ref args);
            return result;
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
            InitializationData initData = null;
            if(configFile != null)
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
        public static Communicator initialize(InitializationData initData)
        {
            if(initData == null)
            {
                initData = new InitializationData();
            }
            else
            {
                initData = (InitializationData)initData.Clone();
            }

            CommunicatorI result = new CommunicatorI(initData);
            string[] args = new string[0];
            result.finishSetup(ref args);
            return result;
        }

        /// <summary>
        /// Creates a communicator.
        /// </summary>
        /// <param name="configFile">Path to a config file that sets the new communicator's default
        /// properties.</param>
        /// <returns>The initialized communicator.</returns>
        public static Communicator initialize(string configFile)
        {
            InitializationData initData = null;
            if(configFile != null)
            {
                initData = new InitializationData();
                initData.properties = Util.createProperties();
                initData.properties.load(configFile);
            }
            return initialize(initData);
        }

        /// <summary>
        /// Creates a communicator using a default configuration.
        /// </summary>
        public static Communicator initialize()
        {
            return initialize((InitializationData)null);
        }

        /// <summary>
        /// Converts a string to an object identity.
        /// </summary>
        /// <param name="s">The string to convert.</param>
        /// <returns>The converted object identity.</returns>
        public static Identity stringToIdentity(string s)
        {
            Identity ident = new Identity();

            //
            // Find unescaped separator; note that the string may contain an escaped
            // backslash before the separator.
            //
            int slash = -1, pos = 0;
            while((pos = s.IndexOf((System.Char) '/', pos)) != -1)
            {
                int escapes = 0;
                while(pos - escapes > 0 && s[pos - escapes - 1] == '\\')
                {
                    escapes++;
                }

                //
                // We ignore escaped escapes
                //
                if(escapes % 2 == 0)
                {
                    if(slash == -1)
                    {
                        slash = pos;
                    }
                    else
                    {
                        //
                        // Extra unescaped slash found.
                        //
                        IdentityParseException ex = new IdentityParseException();
                        ex.str = "unescaped backslash in identity `" + s + "'";
                        throw ex;
                    }
                }
                pos++;
            }

            if(slash == -1)
            {
                ident.category = "";
                try
                {
                    ident.name = IceUtilInternal.StringUtil.unescapeString(s, 0, s.Length, "/");
                }
                catch(ArgumentException e)
                {
                    IdentityParseException ex = new IdentityParseException();
                    ex.str = "invalid identity name `" + s + "': " + e.Message;
                    throw ex;
                }
            }
            else
            {
                try
                {
                    ident.category = IceUtilInternal.StringUtil.unescapeString(s, 0, slash, "/");
                }
                catch(ArgumentException e)
                {
                    IdentityParseException ex = new IdentityParseException();
                    ex.str = "invalid category in identity `" + s + "': " + e.Message;
                    throw ex;
                }
                if(slash + 1 < s.Length)
                {
                    try
                    {
                        ident.name = IceUtilInternal.StringUtil.unescapeString(s, slash + 1, s.Length, "/");
                    }
                    catch(ArgumentException e)
                    {
                        IdentityParseException ex = new IdentityParseException();
                        ex.str = "invalid name in identity `" + s + "': " + e.Message;
                        throw ex;
                    }
                }
                else
                {
                    ident.name = "";
                }
            }

            return ident;
        }

        /// <summary>
        /// Converts an object identity to a string.
        /// </summary>
        /// <param name="ident">The object identity to convert.</param>
        /// <param name="toStringMode">Specifies if and how non-printable ASCII characters are escaped in the result.</param>
        /// <returns>The string representation of the object identity.</returns>
        public static string identityToString(Identity ident, ToStringMode toStringMode = ToStringMode.Unicode)
        {
            if(ident.category == null || ident.category.Length == 0)
            {
                return IceUtilInternal.StringUtil.escapeString(ident.name, "/", toStringMode);
            }
            else
            {
                return IceUtilInternal.StringUtil.escapeString(ident.category, "/", toStringMode) + '/' +
                    IceUtilInternal.StringUtil.escapeString(ident.name, "/", toStringMode);
            }
        }

        /// <summary>
        /// This method is deprecated. Use System.Guid instead.
        /// </summary>
        [Obsolete("This method is deprecated. Use System.Guid instead.")]
        public static string generateUUID()
        {
            return Guid.NewGuid().ToString().ToUpper(System.Globalization.CultureInfo.InvariantCulture);
        }

        /// <summary>
        /// Compares the object identities of two proxies.
        /// </summary>
        /// <param name="lhs">A proxy.</param>
        /// <param name="rhs">A proxy.</param>
        /// <returns>-1 if the identity in lhs compares
        /// less than the identity in rhs; 0 if the identities
        /// compare equal; 1, otherwise.</returns>
        public static int proxyIdentityCompare(ObjectPrx lhs, ObjectPrx rhs)
        {
            if(lhs == null && rhs == null)
            {
                return 0;
            }
            else if(lhs == null && rhs != null)
            {
                return -1;
            }
            else if(lhs != null && rhs == null)
            {
                return 1;
            }
            else
            {
                Identity lhsIdentity = lhs.ice_getIdentity();
                Identity rhsIdentity = rhs.ice_getIdentity();
                int n;
                n = string.CompareOrdinal(lhsIdentity.name, rhsIdentity.name);
                if(n != 0)
                {
                    return n;
                }
                return string.CompareOrdinal(lhsIdentity.category, rhsIdentity.category);
            }
        }

        /// <summary>
        /// Compares the object identities and facets of two proxies.
        /// </summary>
        /// <param name="lhs">A proxy.</param>
        /// <param name="rhs">A proxy.</param>
        /// <returns>-1 if the identity and facet in lhs compare
        /// less than the identity and facet in rhs; 0 if the identities
        /// and facets compare equal; 1, otherwise.</returns>
        public static int proxyIdentityAndFacetCompare(ObjectPrx lhs, ObjectPrx rhs)
        {
            if(lhs == null && rhs == null)
            {
                return 0;
            }
            else if(lhs == null && rhs != null)
            {
                return -1;
            }
            else if(lhs != null && rhs == null)
            {
                    return 1;
            }
            else
            {
                Identity lhsIdentity = lhs.ice_getIdentity();
                Identity rhsIdentity = rhs.ice_getIdentity();
                int n;
                n = string.CompareOrdinal(lhsIdentity.name, rhsIdentity.name);
                if(n != 0)
                {
                    return n;
                }
                n = string.CompareOrdinal(lhsIdentity.category, rhsIdentity.category);
                if(n != 0)
                {
                    return n;
                }

                string lhsFacet = lhs.ice_getFacet();
                string rhsFacet = rhs.ice_getFacet();
                if(lhsFacet == null && rhsFacet == null)
                {
                    return 0;
                }
                else if(lhsFacet == null)
                {
                    return -1;
                }
                else if(rhsFacet == null)
                {
                    return 1;
                }
                else
                {
                    return string.CompareOrdinal(lhsFacet, rhsFacet);
                }
            }
        }

        /// <summary>
        /// Returns the process-wide logger.
        /// </summary>
        /// <returns>The process-wide logger.</returns>
        public static Logger getProcessLogger()
        {
            lock(_processLoggerMutex)
            {
                if(_processLogger == null)
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
            lock(_processLoggerMutex)
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
            return "3.7.1"; // "A.B.C", with A=major, B=minor, C=patch
        }

        /// <summary>
        /// Returns the Ice version as an integer in the form A.BB.CC, where A
        /// indicates the major version, BB indicates the minor version, and CC
        /// indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
        /// </summary>
        /// <returns>The Ice version.</returns>
        public static int intVersion()
        {
            return 30701; // AABBCC, with AA=major, BB=minor, CC=patch
        }

        /// <summary>
        /// Converts a string to a protocol version.
        /// </summary>
        /// <param name="version">The string to convert.</param>
        /// <returns>The converted protocol version.</returns>
        public static ProtocolVersion stringToProtocolVersion(string version)
        {
            byte major, minor;
            stringToMajorMinor(version, out major, out minor);
            return new ProtocolVersion(major, minor);
        }

        /// <summary>
        /// Converts a string to an encoding version.
        /// </summary>
        /// <param name="version">The string to convert.</param>
        /// <returns>The converted object identity.</returns>
        public static EncodingVersion stringToEncodingVersion(string version)
        {
            byte major, minor;
            stringToMajorMinor(version, out major, out minor);
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

        static private void stringToMajorMinor(string str, out byte major, out byte minor)
        {
            int pos = str.IndexOf('.');
            if(pos == -1)
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
            catch(FormatException)
            {
                throw new VersionParseException("invalid version value `" + str + "'");
            }

            if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
            {
                throw new VersionParseException("range error in version `" + str + "'");
            }

            major = (byte)majVersion;
            minor = (byte)minVersion;
        }

        static private string majorMinorToString(byte major, byte minor)
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

        public static readonly NoneType None = new NoneType();

        private static object _processLoggerMutex = new object();
        private static Logger _processLogger = null;
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

        public static void hashAdd(ref int hashCode, object value)
        {
            if(value != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
            }
        }

        public static void hashAdd(ref int hashCode, object[] arr)
        {
            if(arr != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Arrays.GetHashCode(arr));
            }
        }

        public static void hashAdd(ref int hashCode, Array arr)
        {
            if(arr != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Arrays.GetHashCode(arr));
            }
        }

        public static void hashAdd(ref int hashCode, IEnumerable s)
        {
            if(s != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Collections.SequenceGetHashCode(s));
            }
        }

        public static void hashAdd(ref int hashCode, IDictionary d)
        {
            if(d != null)
            {
                hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Collections.DictionaryGetHashCode(d));
            }
        }
    }

    public sealed class Util
    {
        public static Instance getInstance(Ice.Communicator communicator)
        {
            Ice.CommunicatorI p = (Ice.CommunicatorI) communicator;
            return p.getInstance();
        }

        public static ProtocolPluginFacade getProtocolPluginFacade(Ice.Communicator communicator)
        {
            return new ProtocolPluginFacadeI(communicator);
        }

        public static ThreadPriority stringToThreadPriority(string s)
        {
            if(string.IsNullOrEmpty(s))
            {
                return ThreadPriority.Normal;
            }
            if(s.StartsWith("ThreadPriority.", StringComparison.Ordinal))
            {
                s = s.Substring("ThreadPriority.".Length, s.Length);
            }
            if(s.Equals("Lowest"))
            {
                return ThreadPriority.Lowest;
            }
            else if(s.Equals("BelowNormal"))
            {
                return ThreadPriority.BelowNormal;
            }
            else if(s.Equals("Normal"))
            {
                return ThreadPriority.Normal;
            }
            else if(s.Equals("AboveNormal"))
            {
                return ThreadPriority.AboveNormal;
            }
            else if(s.Equals("Highest"))
            {
                return ThreadPriority.Highest;
            }
            return ThreadPriority.Normal;
        }
    }
}
