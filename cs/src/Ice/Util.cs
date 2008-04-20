// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using IceInternal;
    using System;

    public interface ThreadNotification
    {
        void start();
        void stop();
    }

    public class InitializationData : ICloneable
    {
        public System.Object Clone()
        {
            //
            // A member-wise copy is safe because the members are immutable.
            //
            return MemberwiseClone();
        }

        public Properties properties;
        public Logger logger;
        public Stats stats;
        public ThreadNotification threadHook;
    }

    public sealed class Util
    {
        public static Properties createProperties()
        {
            return new PropertiesI();
        }

        public static Properties createProperties(ref string[] args)
        {
            return new PropertiesI(ref args, null);
        }

        public static Properties createProperties(ref string[] args, Properties defaults)
        {
            return new PropertiesI(ref args, defaults);
        }

        public static Communicator initialize(ref string[] args)
        {
            return initialize(ref args, null);
        }

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

        public static Communicator initialize()
        {
            return initialize(null);
        }

        public static IceInternal.Instance getInstance(Communicator communicator)
        {
            CommunicatorI p = (CommunicatorI) communicator;
            return p.getInstance();
        }

        public static IceInternal.ProtocolPluginFacade getProtocolPluginFacade(Communicator communicator)
        {
            return new IceInternal.ProtocolPluginFacadeI(communicator);
        }

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

        public static string identityToString(Identity ident)
        {
            if(ident.category.Length == 0)
            {
                return IceUtilInternal.StringUtil.escapeString(ident.name, "/");
            }
            else
            {
                return IceUtilInternal.StringUtil.escapeString(ident.category, "/") + '/' +
                    IceUtilInternal.StringUtil.escapeString(ident.name, "/");
            }
        }

        public static string generateUUID()
        {
            return Guid.NewGuid().ToString().ToUpper();
        }

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

        public static InputStream createInputStream(Communicator communicator, byte[] bytes)
        {
            return new InputStreamI(communicator, bytes);
        }

        public static OutputStream createOutputStream(Communicator communicator)
        {
            return new OutputStreamI(communicator);
        }

        public static Logger getProcessLogger()
        {
            lock(_processLoggerMutex)
            {
                if(_processLogger == null)
                {
                    _processLogger = new LoggerI(System.AppDomain.CurrentDomain.FriendlyName);
                }
                return _processLogger;
            }
        }

        public static void setProcessLogger(Logger logger)
        {
            lock(_processLoggerMutex)
            {
                _processLogger = logger;
            }
        }

        public static string stringVersion()
        {
            return "3.3.0"; // "A.B.C", with A=major, B=minor, C=patch
        }

        public static int intVersion()
        {
            return 30300; // AABBCC, with AA=major, BB=minor, CC=patch
        }

        private static object _processLoggerMutex = new object();
        private static Logger _processLogger = null;
    }
}
