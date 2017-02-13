// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import com.zeroc.IceUtilInternal.StringUtil;

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
    public static Properties createProperties()
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
     *
     * @return A new property set initialized with the property settings
     * that were removed from the argument vector.
     **/
    public static Properties createProperties(String[] args)
    {
        return createProperties(args, null, null);
    }

    /**
     * Creates a property set initialized from an argument vector and
     * return the remaining arguments.
     *
     * @param args A command-line argument vector, possibly containing
     * options to set properties. If the command-line options include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the argument vector, the argument vector takes precedence.
     *
     * @param remainingArgs If non null, the given list will contain on
     * return the command-line arguments that were not used to set properties.
     *
     * @return A new property set initialized with the property settings
     * that were removed from the argument vector.
     **/
    public static Properties createProperties(String[] args, java.util.List<String> remainingArgs)
    {
        return createProperties(args, null, remainingArgs);
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
     * @param defaults Default values for the property set. Settings in
     * configuration files and <code>args</code> override these defaults.
     *
     * @return A new property set initialized with the property settings
     * that were removed from the argument vector.
     **/
    public static Properties createProperties(String[] args, Properties defaults)
    {
        return createProperties(args, defaults, null);
    }

    /**
     * Creates a property set initialized from an argument vector and
     * return the remaining arguments.
     *
     * @param args A command-line argument vector, possibly containing
     * options to set properties. If the command-line options include
     * a <code>--Ice.Config</code> option, the corresponding configuration
     * files are parsed. If the same property is set in a configuration
     * file and in the argument vector, the argument vector takes precedence.
     *
     * @param defaults Default values for the property set. Settings in
     * configuration files and <code>args</code> override these defaults.
     *
     * @param remainingArgs If non null, the given list will contain on
     * return the command-line arguments that were not used to set properties.
     *
     * @return A new property set initialized with the property settings
     * that were removed from the argument vector.
     **/
    public static Properties createProperties(String[] args,
                                              Properties defaults,
                                              java.util.List<String> remainingArgs)
    {
        PropertiesI properties = new PropertiesI();
        properties.init(args, defaults, remainingArgs);
        return properties;
    }

    /**
     * Creates a communicator using a default configuration.
     *
     * @return A new communicator instance.
     **/
    public static Communicator initialize()
    {
        return initialize(new InitializationData());
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to initialize the communicator.
     *
     * @return The new communicator and a filtered argument vector.
     **/
    public static Communicator initialize(String[] args)
    {
        return initialize(args, null, null);
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to initialize the communicator.
     *
     * @param remainingArgs If non null, the given list will contain on
     * return the command-line arguments that were not used to set properties.
     *
     * @return The new communicator and a filtered argument vector.
     **/
    public static Communicator initialize(String[] args, java.util.List<String> remainingArgs)
    {
        return initialize(args, null, remainingArgs);
    }

    /**
     * Creates a communicator.
     *
     * @param initData Additional initialization data.
     *
     * @return The new communicator and a filtered argument vector.
     **/
    public static Communicator initialize(InitializationData initData)
    {
        return initialize(null, initData, null);
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to initialize the communicator.
     *
     * @param initData Additional initialization data.
     *
     * @return The new communicator and a filtered argument vector.
     **/
    public static Communicator initialize(String[] args, InitializationData initData)
    {
        return initialize(args, initData, null);
    }

    /**
     * Creates a communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options
     * in this vector are used to initialize the communicator.
     *
     * @param initData Additional initialization data. Property settings in <code>args</code>
     * override property settings in <code>initData</code>.
     *
     * @param remainingArgs If non null, the given list will contain on
     * return the command-line arguments that were not used to set properties.
     *
     * @return The new communicator and a filtered argument vector.
     *
     * @see InitializationData
     **/
    public static Communicator initialize(String[] args,
                                          InitializationData initData,
                                          java.util.List<String> remainingArgs)
    {
        if(initData == null)
        {
            initData = new InitializationData();
        }
        else
        {
            initData = initData.clone();
        }

        if(args != null)
        {
            java.util.List<String> rArgs = new java.util.ArrayList<>();
            initData.properties = createProperties(args, initData.properties, rArgs);
            args = rArgs.toArray(new String[rArgs.size()]);
        }

        CommunicatorI communicator = new CommunicatorI(initData);
        communicator.finishSetup(args != null ? args : new String[0], remainingArgs);
        return communicator;
    }

    /**
     * Converts a string to an object identity.
     *
     * @param s The string to convert.
     *
     * @return The converted object identity.
     **/
    public static Identity stringToIdentity(String s)
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
                ident.name = StringUtil.unescapeString(s, 0, s.length(), "/");
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
                ident.category = StringUtil.unescapeString(s, 0, slash, "/");
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
                    ident.name = StringUtil.unescapeString(s, slash + 1, s.length(), "/");
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
     * @param toStringMode Specifies if and how non-printable ASCII characters are escaped in the result.
     *
     * @return The string representation of the object identity.
     **/
    public static String identityToString(Identity ident, ToStringMode toStringMode)
    {
        if(ident.category == null || ident.category.length() == 0)
        {
            return StringUtil.escapeString(ident.name, "/", toStringMode);
        }
        else
        {
            return StringUtil.escapeString(ident.category, "/", toStringMode) + '/' +
                StringUtil.escapeString(ident.name, "/", toStringMode);
        }
    }

    /**
     * Converts an object identity to a string.
     *
     * @param ident The object identity to convert.
     *
     * @return The string representation of the object identity using the default mode (Unicode)
     **/
    public static String identityToString(Identity ident)
    {
        return identityToString(ident, ToStringMode.Unicode);
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
     * Returns the process-wide logger.
     *
     * @return The process-wide logger.
     **/
    public static Logger getProcessLogger()
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
    public static void setProcessLogger(Logger logger)
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
    public static String stringVersion()
    {
        return "3.7a4"; // "A.B.C", with A=major, B=minor, C=patch
    }

    /**
     * Returns the Ice version as an integer in the form <code>A.BB.CC</code>, where <code>A</code>
     * indicates the major version, <code>BB</code> indicates the minor version, and <code>CC</code>
     * indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
     *
     * @return The Ice version.
     **/
    public static int intVersion()
    {
        return 30754; // AABBCC, with AA=major, BB=minor, CC=patch
    }

    /**
     * Converts a string to a protocol version.
     *
     * @param version The string to convert.
     *
     * @return The converted protocol version.
     **/
    static public ProtocolVersion stringToProtocolVersion(String version)
    {
        return new ProtocolVersion(stringToMajor(version), stringToMinor(version));
    }

    /**
     * Converts a string to an encoding version.
     *
     * @param version The string to convert.
     *
     * @return The converted object identity.
     **/
    static public EncodingVersion stringToEncodingVersion(String version)
    {
        return new EncodingVersion(stringToMajor(version), stringToMinor(version));
    }

    /**
     * Converts a protocol version to a string.
     *
     * @param v The protocol version to convert.
     *
     * @return The converted string.
     **/
    static public String protocolVersionToString(ProtocolVersion v)
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
    static public String encodingVersionToString(EncodingVersion v)
    {
        return majorMinorToString(v.major, v.minor);
    }

    /**
     * Returns the supported Ice protocol version.
     *
     * @return The Ice protocol version.
     **/
    static public ProtocolVersion currentProtocol()
    {
        return com.zeroc.IceInternal.Protocol.currentProtocol.clone();
    }

    /**
     * Returns the supported Ice encoding version.
     *
     * @return The Ice encoding version.
     **/
    static public EncodingVersion currentEncoding()
    {
        return com.zeroc.IceInternal.Protocol.currentEncoding.clone();
    }

    /**
     * Returns the InvocationFuture equivalent of the given CompletableFuture.
     *
     * @param f The CompletableFuture returned by an asynchronous Ice proxy invocation.
     * @param <T> The result type.
     * @return The InvocationFuture object.
     **/
    @SuppressWarnings("unchecked")
    static public <T> InvocationFuture<T> getInvocationFuture(java.util.concurrent.CompletableFuture<T> f)
    {
        if(!(f instanceof InvocationFuture))
        {
            throw new IllegalArgumentException("future did not originate from an asynchronous proxy invocation");
        }
        return (InvocationFuture)f;
    }

    /**
     * Translates a Slice type id to a Java class name.
     *
     * @param id The Slice type id, such as <code>::Module::Type</code>.
     * @return The equivalent Java class name, or null if the type id is malformed.
     **/
    static public String typeIdToClass(String id)
    {
        if(!id.startsWith("::"))
        {
            return null;
        }

        StringBuilder buf = new StringBuilder(id.length());

        int start = 2;
        boolean done = false;
        while(!done)
        {
            int end = id.indexOf(':', start);
            String s;
            if(end != -1)
            {
                s = id.substring(start, end);
                start = end + 2;
            }
            else
            {
                s = id.substring(start);
                done = true;
            }
            if(buf.length() > 0)
            {
                buf.append('.');
            }
            buf.append(fixKwd(s));
        }

        return buf.toString();
    }

    static private String fixKwd(String name)
    {
        //
        // Keyword list. *Must* be kept in alphabetical order. Note that checkedCast and uncheckedCast
        // are not Java keywords, but are in this list to prevent illegal code being generated if
        // someone defines Slice operations with that name.
        //
        final String[] keywordList =
        {
            "abstract", "assert", "boolean", "break", "byte", "case", "catch",
            "char", "checkedCast", "class", "clone", "const", "continue", "default", "do",
            "double", "else", "enum", "equals", "extends", "false", "final", "finalize",
            "finally", "float", "for", "getClass", "goto", "hashCode", "if",
            "implements", "import", "instanceof", "int", "interface", "long",
            "native", "new", "notify", "notifyAll", "null", "package", "private",
            "protected", "public", "return", "short", "static", "strictfp", "super", "switch",
            "synchronized", "this", "throw", "throws", "toString", "transient",
            "true", "try", "uncheckedCast", "void", "volatile", "wait", "while"
        };
        boolean found =  java.util.Arrays.binarySearch(keywordList, name) >= 0;
        return found ? "_" + name : name;
    }

    static private byte stringToMajor(String str)
    {
        int pos = str.indexOf('.');
        if(pos == -1)
        {
            throw new VersionParseException("malformed version value `" + str + "'");
        }

        String majStr = str.substring(0, pos);
        int majVersion;
        try
        {
            majVersion = Integer.parseInt(majStr);
        }
        catch(NumberFormatException ex)
        {
            throw new VersionParseException("invalid version value `" + str + "'");
        }

        if(majVersion < 1 || majVersion > 255)
        {
            throw new VersionParseException("range error in version `" + str + "'");
        }

        return (byte)majVersion;
    }

    static private byte stringToMinor(String str)
    {
        int pos = str.indexOf('.');
        if(pos == -1)
        {
            throw new VersionParseException("malformed version value `" + str + "'");
        }

        String minStr = str.substring(pos + 1, str.length());
        int minVersion;
        try
        {
            minVersion = Integer.parseInt(minStr);
        }
        catch(NumberFormatException ex)
        {
            throw new VersionParseException("invalid version value `" + str + "'");
        }

        if(minVersion < 0 || minVersion > 255)
        {
            throw new VersionParseException("range error in version `" + str + "'");
        }

        return (byte)minVersion;
    }

    static private String majorMinorToString(byte major, byte minor)
    {
        StringBuilder str = new StringBuilder();
        str.append(major < 0 ? major + 255 : (int)major);
        str.append(".");
        str.append(minor < 0 ? minor + 255 : (int)minor);
        return str.toString();
    }

    public final static ProtocolVersion Protocol_1_0 = new ProtocolVersion((byte)1, (byte)0);

    public final static EncodingVersion Encoding_1_0 = new EncodingVersion((byte)1, (byte)0);
    public final static EncodingVersion Encoding_1_1 = new EncodingVersion((byte)1, (byte)1);

    private static java.lang.Object _processLoggerMutex = new java.lang.Object();
    private static Logger _processLogger = null;
}
