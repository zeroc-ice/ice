// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Utility methods for the Ice run time.
 **/
public final class Util
{
    /**
     * Creates a new empty property set.
     *
     * @return A new empty property set.
     **/
    public static Properties
    createProperties()
    {
        return new PropertiesI();
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing
     * options to set properties. If the command-line options include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the argument vector, the argument vector takes precedence.
     * <p>
     * This method modifies the argument vector by removing any Ice-related options.
     *
     * @return A property set initialized with the property settings
     * that were removed from <code>args</code>.
     **/
    public static Properties
    createProperties(StringSeqHolder args)
    {
        return new PropertiesI(args, null);
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing
     * options to set properties. If the command-line options include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the argument vector, the argument vector takes precedence.
     * <p>
     * This method modifies the argument vector by removing any Ice-related options.
     * @param defaults Default values for the property set. Settings in configuration
     * files and <code>args</code> override these defaults.
     *
     * @return An initalized property set.
     **/
    public static Properties
    createProperties(StringSeqHolder args, Properties defaults)
    {
        return new PropertiesI(args, defaults);
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing
     * options to set properties. If the command-line options include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the argument vector, the argument vector takes precedence.
     *
     * @return A property set initialized with the property settings
     * in <code>args</code>.
     **/
    public static Properties
    createProperties(String[] args)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
        return createProperties(argsH);
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing
     * options to set properties. If the command-line options include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the argument vector, the argument vector takes precedence.
     * @param defaults Default values for the property set. Settings in configuration
     * files and <code>args</code> override these defaults.
     *
     * @return An initalized property set.
     **/
    public static Properties
    createProperties(String[] args, Properties defaults)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
        return createProperties(argsH, defaults);
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to intialize the communicator.
     * This method modifies the argument vector by removing any Ice-related options.
     *
     * @return The initialized communicator.
     **/
    public static Communicator
    initialize(StringSeqHolder args)
    {
        return initialize(args, null);
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to intialize the communicator.
     *
     * @return The initialized communicator.
     **/
    public static Communicator
    initialize(String[] args)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
        return initialize(argsH);
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to intialize the communicator.
     * This method modifies the argument vector by removing any Ice-related options.
     *
     * @param initData Additional intialization data. Property settings in <code>args</code>
     * override property settings in <code>initData</code>.
     *
     * @return The initialized communicator.
     *
     * @see InitializationData
     **/
    public static Communicator
    initialize(StringSeqHolder args, InitializationData initData)
    {
        if(initData == null)
        {
            initData = new InitializationData();
        }
        else
        {
            initData = initData.clone();
        }
        initData.properties = createProperties(args, initData.properties);

        CommunicatorI result = new CommunicatorI(initData);
        result.finishSetup(args);
        return result;
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to intialize the communicator.
     *
     * @param initData Additional intialization data. Property settings in <code>args</code>
     * override property settings in <code>initData</code>.
     *
     * @return The initialized communicator.
     *
     * @see InitializationData
     **/
    public static Communicator
    initialize(String[] args, InitializationData initData)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
        return initialize(argsH, initData);
    }

    /**
     * Creates a communicator.
     *
     * @param initData Additional intialization data.
     *
     * @return The initialized communicator.
     *
     * @see InitializationData
     **/
    public static Communicator
    initialize(InitializationData initData)
    {
        if(initData == null)
        {
            initData = new InitializationData();
        }
        else
        {
            initData = initData.clone();
        }

        CommunicatorI result = new CommunicatorI(initData);
        result.finishSetup(new StringSeqHolder(new String[0]));
        return result;
    }

    /**
     * Creates a communicator using a default configuration.
     **/
    public static Communicator
    initialize()
    {
        return initialize(new InitializationData());
    }

    /**
     * Converts a string to an object identity.
     *
     * @param s The string to convert.
     *
     * @return The converted object identity.
     **/
    public static Identity
    stringToIdentity(String s)
    {
        Identity ident = new Identity();

        //
        // Find unescaped separator; note that the string may contain an escaped
        // backslash before the separator.
        //
        int slash = -1, pos = 0;
        while((pos = s.indexOf('/', pos)) != -1)
        {
            int escapes = 0;
            while(pos - escapes > 0 && s.charAt(pos - escapes - 1) == '\\')
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
                ident.name = IceUtilInternal.StringUtil.unescapeString(s, 0, s.length());
            }
            catch(IllegalArgumentException e)
            {
                IdentityParseException ex = new IdentityParseException();
                ex.str = "invalid identity name `" + s + "': " + e.getMessage();
                throw ex;
            }
        }
        else
        {
            try
            {
                ident.category = IceUtilInternal.StringUtil.unescapeString(s, 0, slash);
            }
            catch(IllegalArgumentException e)
            {
                IdentityParseException ex = new IdentityParseException();
                ex.str = "invalid category in identity `" + s + "': " + e.getMessage();
                throw ex;
            }
            if(slash + 1 < s.length())
            {
                try
                {
                    ident.name = IceUtilInternal.StringUtil.unescapeString(s, slash + 1, s.length());
                }
                catch(IllegalArgumentException e)
                {
                    IdentityParseException ex = new IdentityParseException();
                    ex.str = "invalid name in identity `" + s + "': " + e.getMessage();
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

    /**
     * Converts an object identity to a string.
     *
     * @param ident The object identity to convert.
     *
     * @return The string representation of the object identity.
     **/
    public static String
    identityToString(Identity ident)
    {
        if(ident.category == null || ident.category.length() == 0)
        {
            return IceUtilInternal.StringUtil.escapeString(ident.name, "/");
        }
        else
        {
            return IceUtilInternal.StringUtil.escapeString(ident.category, "/") + '/' +
                IceUtilInternal.StringUtil.escapeString(ident.name, "/");
        }
    }

    /**
     * Compares the object identities of two proxies.
     *
     * @param lhs A proxy.
     * @param rhs A proxy.
     * @return -1 if the identity in <code>lhs</code> compares
     * less than the identity in <code>rhs</code>; 0 if the identities
     * compare equal; 1, otherwise.
     *
     * @see ProxyIdentityKey
     * @see ProxyIdentityFacetKey
     * @see #proxyIdentityAndFacetCompare
     **/
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
            if((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0)
            {
                return n;
            }
            return lhsIdentity.category.compareTo(rhsIdentity.category);
        }
    }

    /**
     * Compares the object identities and facets of two proxies.
     *
     * @param lhs A proxy.
     * @param rhs A proxy.
     * @return -1 if the identity and facet in <code>lhs</code> compare
     * less than the identity and facet in <code>rhs</code>; 0 if the identities
     * and facets compare equal; 1, otherwise.
     *
     * @see ProxyIdentityFacetKey
     * @see ProxyIdentityKey
     * @see #proxyIdentityCompare
     **/
    public static int
    proxyIdentityAndFacetCompare(ObjectPrx lhs, ObjectPrx rhs)
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
            if((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0)
            {
                return n;
            }
            if((n = lhsIdentity.category.compareTo(rhsIdentity.category)) != 0)
            {
                return n;
            }

            String lhsFacet = lhs.ice_getFacet();
            String rhsFacet = rhs.ice_getFacet();
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
            return lhsFacet.compareTo(rhsFacet);
        }
    }

    /**
     * Creates an input stream for dynamic invocation and dispatch. The stream uses
     * the communicator's default encoding version.
     *
     * @param communicator The communicator for the stream.
     * @param bytes An encoded request or reply.
     * @return The input stream.
     **/
    public static InputStream
    createInputStream(Communicator communicator, byte[] bytes)
    {
        return new InputStreamI(communicator, bytes, true);
    }

    /**
     * Creates an input stream for dynamic invocation and dispatch. The stream uses
     * the given encoding version.
     *
     * @param communicator The communicator for the stream.
     * @param bytes An encoded request or reply.
     * @param v The desired encoding version.
     * @return The input stream.
     **/
    public static InputStream
    createInputStream(Communicator communicator, byte[] bytes, EncodingVersion v)
    {
        return new InputStreamI(communicator, bytes, v, true);
    }

    /**
     * Wraps encoded data with an input stream for dynamic invocation
     * and dispatch. The stream uses the communicator's default
     * encoding version.
     *
     * @param communicator The communicator for the stream.
     * @param bytes An encoded request or reply.
     * @return The input stream.
     **/
    public static InputStream
    wrapInputStream(Communicator communicator, byte[] bytes)
    {
        return new InputStreamI(communicator, bytes, false);
    }

    /**
     * Wraps encoded data with an input stream for dynamic invocation
     * and dispatch. The stream uses the given encoding version.
     *
     * @param communicator The communicator for the stream.
     * @param bytes An encoded request or reply.
     * @param v The desired encoding version.
     * @return The input stream.
     **/
    public static InputStream
    wrapInputStream(Communicator communicator, byte[] bytes, EncodingVersion v)
    {
        return new InputStreamI(communicator, bytes, v, false);
    }

    /**
     * Creates an output stream for dynamic invocation and dispatch. The stream uses
     * the communicator's default encoding version.
     *
     * @param communicator The communicator for the stream.
     * @return The output stream.
     **/
    public static OutputStream
    createOutputStream(Communicator communicator)
    {
        return new OutputStreamI(communicator);
    }

    /**
     * Creates an output stream for dynamic invocation and dispatch. The stream uses
     * the given encoding version.
     *
     * @param communicator The communicator for the stream.
     * @param v The desired encoding version.
     * @return The output stream.
     **/
    public static OutputStream
    createOutputStream(Communicator communicator, EncodingVersion v)
    {
        return new OutputStreamI(communicator, v);
    }

    /**
     * Returns the process-wide logger.
     *
     * @return The process-wide logger.
     **/
    public static Logger
    getProcessLogger()
    {
        synchronized(_processLoggerMutex)
        {
            if(_processLogger == null)
            {
                //
                // TODO: Would be nice to be able to use process name as prefix by default.
                //
                _processLogger = new LoggerI("", "");
            }

            return _processLogger;
        }
    }

    /**
     * Changes the process-wide logger.
     *
     * @param logger The new process-wide logger.
     **/
    public static void
    setProcessLogger(Logger logger)
    {
        synchronized(_processLoggerMutex)
        {
            _processLogger = logger;
        }
    }

    /**
     * Returns the Ice version in the form <code>A.B.C</code>, where <code>A</code> indicates the
     * major version, <code>B</code> indicates the minor version, and <code>C</code> indicates the
     * patch level.
     *
     * @return The Ice version.
     **/
    public static String
    stringVersion()
    {
        return "3.6.5"; // "A.B.C", with A=major, B=minor, C=patch
    }

    /**
     * Returns the Ice version as an integer in the form <code>A.BB.CC</code>, where <code>A</code>
     * indicates the major version, <code>BB</code> indicates the minor version, and <code>CC</code>
     * indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
     *
     * @return The Ice version.
     **/
    public static int
    intVersion()
    {
        return 30605; // AABBCC, with AA=major, BB=minor, CC=patch
    }

    /**
     * Converts a string to a protocol version.
     *
     * @param version The string to convert.
     *
     * @return The converted protocol version.
     **/
    static public Ice.ProtocolVersion
    stringToProtocolVersion(String version)
    {
        return new Ice.ProtocolVersion(stringToMajor(version), stringToMinor(version));
    }

    /**
     * Converts a string to an encoding version.
     *
     * @param version The string to convert.
     *
     * @return The converted object identity.
     **/
    static public Ice.EncodingVersion
    stringToEncodingVersion(String version)
    {
        return new Ice.EncodingVersion(stringToMajor(version), stringToMinor(version));
    }

    /**
     * Converts a protocol version to a string.
     *
     * @param v The protocol version to convert.
     *
     * @return The converted string.
     **/
    static public String
    protocolVersionToString(Ice.ProtocolVersion v)
    {
        return majorMinorToString(v.major, v.minor);
    }

    /**
     * Converts an encoding version to a string.
     *
     * @param v The encoding version to convert.
     *
     * @return The converted string.
     **/
    static public String
    encodingVersionToString(Ice.EncodingVersion v)
    {
        return majorMinorToString(v.major, v.minor);
    }

    /**
     * Returns the supported Ice protocol version.
     *
     * @return The Ice protocol version.
     **/
    static public Ice.ProtocolVersion
    currentProtocol()
    {
        return IceInternal.Protocol.currentProtocol.clone();
    }

    /**
     * Returns the supported Ice encoding version.
     *
     * @return The Ice encoding version.
     **/
    static public Ice.EncodingVersion
    currentEncoding()
    {
        return IceInternal.Protocol.currentEncoding.clone();
    }

    static private byte
    stringToMajor(String str)
    {
        int pos = str.indexOf('.');
        if(pos == -1)
        {
            throw new Ice.VersionParseException("malformed version value `" + str + "'");
        }

        String majStr = str.substring(0, pos);
        int majVersion;
        try
        {
            majVersion = Integer.parseInt(majStr);
        }
        catch(NumberFormatException ex)
        {
            throw new Ice.VersionParseException("invalid version value `" + str + "'");
        }

        if(majVersion < 1 || majVersion > 255)
        {
            throw new Ice.VersionParseException("range error in version `" + str + "'");
        }

        return (byte)majVersion;
    }

    static private byte
    stringToMinor(String str)
    {
        int pos = str.indexOf('.');
        if(pos == -1)
        {
            throw new Ice.VersionParseException("malformed version value `" + str + "'");
        }

        String minStr = str.substring(pos + 1, str.length());
        int minVersion;
        try
        {
            minVersion = Integer.parseInt(minStr);
        }
        catch(NumberFormatException ex)
        {
            throw new Ice.VersionParseException("invalid version value `" + str + "'");
        }

        if(minVersion < 0 || minVersion > 255)
        {
            throw new Ice.VersionParseException("range error in version `" + str + "'");
        }

        return (byte)minVersion;
    }

    static private String
    majorMinorToString(byte major, byte minor)
    {
        StringBuilder str = new StringBuilder();
        str.append(major < 0 ? major + 255 : (int)major);
        str.append(".");
        str.append(minor < 0 ? minor + 255 : (int)minor);
        return str.toString();
    }

    public final static Ice.ProtocolVersion Protocol_1_0 = new Ice.ProtocolVersion((byte)1, (byte)0);

    public final static Ice.EncodingVersion Encoding_1_0 = new Ice.EncodingVersion((byte)1, (byte)0);
    public final static Ice.EncodingVersion Encoding_1_1 = new Ice.EncodingVersion((byte)1, (byte)1);

    private static java.lang.Object _processLoggerMutex = new java.lang.Object();
    private static Logger _processLogger = null;
}
