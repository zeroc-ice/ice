// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
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
    /// A class that encpasulates data to initalize a communicator.
    /// </summary>
    public class InitializationData : ICloneable
    {
        /// <summary>
        /// Creates and returns a copy of this object.
        /// </summary>
        public System.Object Clone()
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
        /// The Stats instance for the communicator.
        /// </summary>
        public Stats stats;

        /// <summary>
        /// The thread hook for the communicator.
        /// </summary>
        public ThreadNotification threadHook;
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
            return initialize(ref args, null);
        }

        /// <summary>
        /// Creates a communicator.
        /// </summary>
        /// <param name="args">A command-line argument vector. Any Ice-related options
        /// in this vector are used to intialize the communicator.
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
        /// Creates a communicator using a default configuration.
        /// </summary>
        public static Communicator initialize()
        {
            return initialize(null);
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
            // Find unescaped separator.
            //
            int slash = -1, pos = 0;
            while((pos = s.IndexOf((System.Char) '/', pos)) != -1)
            {
                if(pos == 0 || s[pos - 1] != '\\')
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
                        ex.str = s;
                        throw ex;
                    }
                }
                pos++;
            }

            if(slash == -1)
            {
                if(!IceUtilInternal.StringUtil.unescapeString(s, 0, s.Length, out ident.name))
                {
                    IdentityParseException ex = new IdentityParseException();
                    ex.str = s;
                    throw ex;
                }
                ident.category = "";
            }
            else
            {
                if(!IceUtilInternal.StringUtil.unescapeString(s, 0, slash, out ident.category))
                {
                    IdentityParseException ex = new IdentityParseException();
                    ex.str = s;
                    throw ex;
                }
                if(slash + 1 < s.Length)
                {
                    if(!IceUtilInternal.StringUtil.unescapeString(s, slash + 1, s.Length, out ident.name))
                    {
                        IdentityParseException ex = new IdentityParseException();
                        ex.str = s;
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
        /// <returns>The string representation of the object identity.</returns>
        public static string identityToString(Identity ident)
        {
            if(ident.category == null || ident.category.Length == 0)
            {
                return IceUtilInternal.StringUtil.escapeString(ident.name, "/");
            }
            else
            {
                return IceUtilInternal.StringUtil.escapeString(ident.category, "/") + '/' +
                    IceUtilInternal.StringUtil.escapeString(ident.name, "/");
            }
        }

        /// <summary>
        /// This method is deprecated. Use System.Guid instead.
        /// </summary>
        [Obsolete("This method is deprecated. Use System.Guid instead.")]
        public static string generateUUID()
        {
            return Guid.NewGuid().ToString().ToUpper();
        }

        /// <summary>
        /// Compares the object identities of two proxies.
        /// </summary>
        /// <param name="lhs">A proxy.</param>
        /// <param name="rhs">A proxy.</param>
        /// <returns>-1 if the identity in lhs compares
        /// less than the identity in rhs; 0 if the identities
        /// compare equal; 1, otherwise.</returns>
        public static int
        proxyIdentityCompare(ObjectPrx lhs, ObjectPrx rhs)
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
        /// Creates an input stream for dynamic invocation and dispatch.
        /// </summary>
        /// <param name="communicator">The communicator for the stream.</param>
        /// <param name="bytes">An encoded request or reply.</param>
        /// <returns>The input stream.</returns>
        public static InputStream createInputStream(Communicator communicator, byte[] bytes)
        {
            return new InputStreamI(communicator, bytes);
        }

        /// <summary>
        /// Creates an output stream for dynamic invocation and dispatch.
        /// </summary>
        /// <param name="communicator">The communicator for the stream.</param>
        /// <returns>The output stream.</returns>
        public static OutputStream createOutputStream(Communicator communicator)
        {
            return new OutputStreamI(communicator);
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
                    _processLogger = new LoggerI(System.AppDomain.CurrentDomain.FriendlyName, "");
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
            return "3.4b"; // "A.B.C", with A=major, B=minor, C=patch
        }

        /// <summary>
        /// Returns the Ice version as an integer in the form A.BB.CC, where A
        /// indicates the major version, BB indicates the minor version, and CC
        /// indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
        /// </summary>
        /// <returns>The Ice version.</returns>
        public static int intVersion()
        {
            return 30451; // AABBCC, with AA=major, BB=minor, CC=patch
        }

        private static object _processLoggerMutex = new object();
        private static Logger _processLogger = null;
    }
}

namespace IceInternal
{
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

        public static System.Threading.ThreadPriority stringToThreadPriority(string s)
        {
            if(String.IsNullOrEmpty(s))
            {
                return ThreadPriority.Normal;
            }
            if(s.StartsWith("ThreadPriority."))
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
