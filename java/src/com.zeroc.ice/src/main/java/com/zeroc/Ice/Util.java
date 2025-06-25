// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ThreadFactory;

/** Utility methods for the Ice runtime. */
public final class Util {
    /**
     * Creates a new empty property set.
     *
     * @return A new empty property set.
     * @deprecated Use {@link Properties#Properties()} instead.
     */
    @Deprecated
    public static Properties createProperties() {
        return new Properties();
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a <code>--Ice.Config</code> option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     * @return A new property set initialized with the property settings that were removed from the
     *     argument vector.
     * @deprecated Use {@link Properties#Properties(String[])} instead.
     */
    @Deprecated
    public static Properties createProperties(String[] args) {
        return new Properties(args, null, null);
    }

    /**
     * Creates a property set initialized from an argument vector and returns the remaining
     * arguments.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a {@code --Ice.Config} option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     * @param remainingArgs If non null, the given list will contain on return the command-line
     *     arguments that were not used to set properties.
     * @return A new property set initialized with the property settings that were removed from the
     *     argument vector.
     * @deprecated Use {@link Properties#Properties(String[], java.util.List)} instead.
     */
    @Deprecated
    public static Properties createProperties(String[] args, List<String> remainingArgs) {
        return new Properties(args, null, remainingArgs);
    }

    /**
     * Creates a property set initialized from an argument vector.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a {@code --Ice.Config} option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     * @param defaults Default values for the property set. Settings in configuration files and
     *     {@code args} override these defaults.
     * @return A new property set initialized with the property settings that were removed from the
     *     argument vector.
     * @deprecated Use {@link Properties#Properties(String[], Properties)} instead.
     */
    @Deprecated
    public static Properties createProperties(String[] args, Properties defaults) {
        return new Properties(args, defaults, null);
    }

    /**
     * Creates a property set initialized from an argument vector and returns the remaining
     * arguments.
     *
     * @param args A command-line argument vector, possibly containing options to set properties. If
     *     the command-line options include a {@code --Ice.Config} option, the corresponding
     *     configuration files are parsed. If the same property is set in a configuration file and
     *     in the argument vector, the argument vector takes precedence.
     * @param defaults Default values for the property set. Settings in configuration files and
     *     {@code args} override these defaults.
     * @param remainingArgs If non null, the given list will contain on return the command-line
     *     arguments that were not used to set properties.
     * @return A new property set initialized with the property settings that were removed from the
     *     argument vector.
     * @deprecated Use {@link Properties#Properties(String[], Properties, java.util.List)} instead.
     */
    @Deprecated
    public static Properties createProperties(
            String[] args, Properties defaults, List<String> remainingArgs) {
        return new Properties(args, defaults, remainingArgs);
    }

    /**
     * Creates a new communicator.
     *
     * @return The new communicator.
     */
    public static Communicator initialize() {
        return initialize(new InitializationData());
    }

    /**
     * Creates a new communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options in this vector are used
     *     to initialize the communicator.
     * @return The new communicator.
     */
    public static Communicator initialize(String[] args) {
        return initialize(args, (InitializationData) null, null);
    }

    /**
     * Creates a new communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options in this vector are used
     *     to initialize the communicator.
     * @param remainingArgs If non null, the given list will contain on return the command-line
     *     arguments that were not used to set properties.
     * @return The new communicator.
     */
    public static Communicator initialize(String[] args, List<String> remainingArgs) {
        return initialize(args, (InitializationData) null, remainingArgs);
    }

    /**
     * Creates a new communicator.
     *
     * @param initData Additional initialization data.
     * @return The new communicator.
     */
    public static Communicator initialize(InitializationData initData) {
        return initialize(null, initData, null);
    }

    /**
     * Creates a new communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options in this vector are used
     *     to initialize the communicator.
     * @param initData Additional initialization data.
     * @return The new communicator.
     */
    public static Communicator initialize(String[] args, InitializationData initData) {
        return initialize(args, initData, null);
    }

    /**
     * Creates a new communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options in this vector are used
     *     to initialize the communicator.
     * @param configFile Path to a config file that sets the new communicator's default properties.
     * @return The new communicator.
     */
    public static Communicator initialize(String[] args, String configFile) {
        return initialize(args, configFile, null);
    }

    /**
     * Creates a new communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options in this vector are used
     *     to initialize the communicator.
     * @param initData Additional initialization data. Property settings in {@code args}
     *     override property settings in {@code initData}.
     * @param remainingArgs If non null, the given list will contain on return the command-line
     *     arguments that were not used to set properties.
     * @return The new communicator.
     * @see InitializationData
     */
    public static Communicator initialize(
            String[] args, InitializationData initData, List<String> remainingArgs) {
        if (initData == null) {
            initData = new InitializationData();
        } else {
            initData = initData.clone();
        }

        if (args != null) {
            List<String> rArgs = new ArrayList<>();
            initData.properties = createProperties(args, initData.properties, rArgs);
            args = rArgs.toArray(new String[rArgs.size()]);
        }

        var communicator = new Communicator(initData);
        communicator.finishSetup(args != null ? args : new String[0], remainingArgs);
        return communicator;
    }

    /**
     * Creates a new communicator.
     *
     * @param args A command-line argument vector. Any Ice-related options in this vector are used
     *     to initialize the communicator.
     * @param configFile Path to a config file that sets the new communicator's default properties.
     * @param remainingArgs If non null, the given list will contain on return the command-line
     *     arguments that were not used to set properties.
     * @return The new communicator.
     */
    public static Communicator initialize(
            String[] args, String configFile, List<String> remainingArgs) {
        InitializationData initData = null;
        if (configFile != null) {
            initData = new InitializationData();
            initData.properties = new Properties();
            initData.properties.load(configFile);
        }

        return initialize(args, initData, remainingArgs);
    }

    /**
     * Converts a stringified identity into an Identity.
     *
     * @param s The string to convert.
     * @return The converted object identity.
     */
    public static Identity stringToIdentity(String s) {
        Identity ident = new Identity();

        //
        // Find unescaped separator; note that the string may contain an escaped
        // backslash before the separator.
        //
        int slash = -1, pos = 0;
        while ((pos = s.indexOf('/', pos)) != -1) {
            int escapes = 0;
            while (pos - escapes > 0 && s.charAt(pos - escapes - 1) == '\\') {
                escapes++;
            }

            //
            // We ignore escaped escapes
            //
            if (escapes % 2 == 0) {
                if (slash == -1) {
                    slash = pos;
                } else {
                    //
                    // Extra unescaped slash found.
                    //
                    throw new ParseException("unescaped backslash in identity string '" + s + "'");
                }
            }
            pos++;
        }

        if (slash == -1) {
            ident.category = "";
            try {
                ident.name = StringUtil.unescapeString(s, 0, s.length(), "/");
            } catch (IllegalArgumentException ex) {
                throw new ParseException("invalid name in identity string '" + s + "'", ex);
            }
        } else {
            try {
                ident.category = StringUtil.unescapeString(s, 0, slash, "/");
            } catch (IllegalArgumentException ex) {
                throw new ParseException("invalid category in identity string '" + s + "'", ex);
            }
            if (slash + 1 < s.length()) {
                try {
                    ident.name = StringUtil.unescapeString(s, slash + 1, s.length(), "/");
                } catch (IllegalArgumentException ex) {
                    throw new ParseException("invalid name in identity string '" + s + "'", ex);
                }
            } else {
                ident.name = "";
            }
        }

        return ident;
    }

    /**
     * Converts an Identity into a string using the specified mode.
     *
     * @param ident The object identity to convert.
     * @param toStringMode Specifies how to handle non-ASCII characters and non-printable ASCII characters.
     * @return The string representation of the object identity.
     */
    public static String identityToString(Identity ident, ToStringMode toStringMode) {
        if (ident.category == null || ident.category.isEmpty()) {
            return StringUtil.escapeString(ident.name, "/", toStringMode);
        } else {
            return StringUtil.escapeString(ident.category, "/", toStringMode)
                + '/'
                + StringUtil.escapeString(ident.name, "/", toStringMode);
        }
    }

    /**
     * Converts an object identity to a string.
     *
     * @param ident The object identity to convert.
     * @return The string representation of the object identity using the default mode (Unicode)
     */
    public static String identityToString(Identity ident) {
        return identityToString(ident, ToStringMode.Unicode);
    }

    /**
     * Compares the object identities of two proxies.
     *
     * @param lhs A proxy.
     * @param rhs A proxy.
     * @return -1 if the identity in {@code lhs} compares less than the identity in {@code rhs};
     *     0 if the identities compare equal; 1, otherwise.
     * @see ProxyIdentityKey
     * @see ProxyIdentityFacetKey
     * @see #proxyIdentityAndFacetCompare
     */
    public static int proxyIdentityCompare(ObjectPrx lhs, ObjectPrx rhs) {
        if (lhs == null && rhs == null) {
            return 0;
        } else if (lhs == null && rhs != null) {
            return -1;
        } else if (lhs != null && rhs == null) {
            return 1;
        } else {
            Identity lhsIdentity = lhs.ice_getIdentity();
            Identity rhsIdentity = rhs.ice_getIdentity();
            int n;
            if ((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0) {
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
     * @return -1 if the identity and facet in {@code lhs} compare less than the identity and
     *     facet in {@code rhs}; 0 if the identities and facets compare equal; 1, otherwise.
     * @see ProxyIdentityFacetKey
     * @see ProxyIdentityKey
     * @see #proxyIdentityCompare
     */
    public static int proxyIdentityAndFacetCompare(ObjectPrx lhs, ObjectPrx rhs) {
        if (lhs == null && rhs == null) {
            return 0;
        } else if (lhs == null && rhs != null) {
            return -1;
        } else if (lhs != null && rhs == null) {
            return 1;
        } else {
            Identity lhsIdentity = lhs.ice_getIdentity();
            Identity rhsIdentity = rhs.ice_getIdentity();
            int n;
            if ((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0) {
                return n;
            }
            if ((n = lhsIdentity.category.compareTo(rhsIdentity.category)) != 0) {
                return n;
            }

            String lhsFacet = lhs.ice_getFacet();
            String rhsFacet = rhs.ice_getFacet();
            if (lhsFacet == null && rhsFacet == null) {
                return 0;
            } else if (lhsFacet == null) {
                return -1;
            } else if (rhsFacet == null) {
                return 1;
            }
            return lhsFacet.compareTo(rhsFacet);
        }
    }

    /**
     * Gets the per-process logger. This logger is used by all communicators that do not have their own specific logger
     * configured at the time the communicator is created.
     *
     * @return The current per-process logger instance.
     */
    public static Logger getProcessLogger() {
        synchronized (_processLoggerMutex) {
            if (_processLogger == null) {
                //
                // TODO: Would be nice to be able to use process name as prefix by default.
                //
                _processLogger = new LoggerI("");
            }

            return _processLogger;
        }
    }

    /**
     * Sets the per-process logger. This logger is used by all communicators that do not have their own specific logger
     * configured at the time the communicator is created.
     *
     * @param logger The new per-process logger instance.
     */
    public static void setProcessLogger(Logger logger) {
        synchronized (_processLoggerMutex) {
            _processLogger = logger;
        }
    }

    /**
     * Returns the Ice version in the form {@code A.B.C}, where {@code A} indicates the
     * major version, {@code B} indicates the minor version, and {@code C} indicates the
     * patch level.
     *
     * @return The Ice version.
     */
    public static String stringVersion() {
        return "3.8.0-alpha.0"; // "A.B.C", with A=major, B=minor, C=patch
    }

    /**
     * Returns the Ice version as an integer in the form {@code A.BB.CC}, where {@code A}
     * indicates the major version, {@code BB} indicates the minor version, and {@code CC}
     * indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
     *
     * @return The Ice version.
     */
    public static int intVersion() {
        return 30850; // AABBCC, with AA=major, BB=minor, CC=patch
    }

    /**
     * Converts a string to a protocol version.
     *
     * @param version The string to convert.
     * @return The converted protocol version.
     */
    public static ProtocolVersion stringToProtocolVersion(String version) {
        return new ProtocolVersion(stringToMajor(version), stringToMinor(version));
    }

    /**
     * Converts a string to an encoding version.
     *
     * @param version The string to convert.
     * @return The converted encoding version.
     */
    public static EncodingVersion stringToEncodingVersion(String version) {
        return new EncodingVersion(stringToMajor(version), stringToMinor(version));
    }

    /**
     * Converts a protocol version to a string.
     *
     * @param v The protocol version to convert.
     * @return The converted string.
     */
    public static String protocolVersionToString(ProtocolVersion v) {
        return majorMinorToString(v.major, v.minor);
    }

    /**
     * Converts an encoding version to a string.
     *
     * @param v The encoding version to convert.
     * @return The converted string.
     */
    public static String encodingVersionToString(EncodingVersion v) {
        return majorMinorToString(v.major, v.minor);
    }

    /**
     * Returns the supported Ice protocol version.
     *
     * @return The Ice protocol version.
     */
    public static ProtocolVersion currentProtocol() {
        return Protocol.currentProtocol.clone();
    }

    /**
     * Returns the supported Ice encoding version.
     *
     * @return The Ice encoding version.
     */
    public static EncodingVersion currentEncoding() {
        return Protocol.currentEncoding.clone();
    }

    /**
     * Returns the InvocationFuture equivalent of the given CompletableFuture.
     *
     * @param f The CompletableFuture returned by an asynchronous Ice proxy invocation.
     * @param <T> The result type.
     * @return The InvocationFuture object.
     */
    public static <T> InvocationFuture<T> getInvocationFuture(
            CompletableFuture<T> f) {
        if (!(f instanceof InvocationFuture)) {
            throw new IllegalArgumentException(
                "future did not originate from an asynchronous proxy invocation");
        }
        return (InvocationFuture<T>) f;
    }

    /**
     * Translates a Slice type id to a Java class name.
     *
     * @param id The Slice type id, such as {@code ::Module::Type}.
     * @return The equivalent Java class name, or null if the type id is malformed.
     */
    public static String typeIdToClass(String id) {
        if (!id.startsWith("::")) {
            return null;
        }

        StringBuilder buf = new StringBuilder(id.length());
        int start = 2;
        boolean done = false;
        while (!done) {
            int end = id.indexOf(':', start);
            String s;
            if (end != -1) {
                s = id.substring(start, end);
                start = end + 2;
            } else {
                s = id.substring(start);
                done = true;
            }
            if (buf.length() > 0) {
                buf.append('.');
            }
            buf.append(s);
        }

        return buf.toString();
    }

    static String createThreadName(final Properties properties, final String name) {
        String threadName = properties.getIceProperty("Ice.ProgramName");
        if (!threadName.isEmpty()) {
            threadName += '-';
        }

        threadName = threadName + name;
        return threadName;
    }

    static ThreadFactory createThreadFactory(final Properties properties, final String name) {
        return new ThreadFactory() {
            @Override
            public Thread newThread(Runnable r) {
                Thread t = new Thread(r);
                t.setName(name);

                if (properties.getIceProperty("Ice.ThreadPriority").length() > 0) {
                    t.setPriority(Util.getThreadPriorityProperty(properties, "Ice"));
                }
                return t;
            }
        };
    }

    /**
     * Given a path name, first try to open it as a class path resource (the path is treated as
     * absolute). If that fails, fall back to the file system. Returns null if the file does not
     * exist and raises IOException if an error occurs.
     *
     * @param cl the class loader to use
     * @param path the path to the resource
     * @return an InputStream, or null if the resource could not be found
     * @throws IOException if an I/O error occurs while trying to open the resource
     * @hidden Public because it's used by SSL.
     */
    public static InputStream openResource(ClassLoader cl, String path)
        throws IOException {
        //
        // Calling getResourceAsStream on the class loader means all paths are absolute,
        // whereas calling it on the class means all paths are relative to the class
        // unless the path has a leading forward slash. We call it on the class loader.
        //
        // getResourceAsStream returns null if the resource can't be found.
        //
        InputStream stream = null;
        try {
            stream = cl.getResourceAsStream(path);
        } catch (IllegalArgumentException ex) {
            //
            // With JDK-7 this can happen if the result url (base url + path) produces a
            // malformed url for an URLClassLoader. For example the code in following
            // comment will produce this exception under Windows.
            //
            // URLClassLoader cl = new URLClassLoader(new URL[] {new
            // URL("http://localhost:8080/")});
            // java.io.InputStream in = Util.openResource(cl, "c:\\foo.txt");
            //
        }
        if (stream == null) {
            try {
                File f = new File(path);
                if (f.exists()) {
                    stream = new FileInputStream(f);
                }
            } catch (java.lang.SecurityException ex) {
                // Ignore - a security manager may forbid access to the local file system.
            }
        }

        return stream;
    }

    /**
     * Finds a class by name using the specified class loader.
     *
     * @param className the name of the class to find
     * @param cl the class loader to use, or null to use the system class loader
     * @return the class object for the specified class, or null if the class could not be found
     * @hidden Public because it's used by IceBox and IceGridGUI.
     */
    public static Class<?> findClass(String className, ClassLoader cl) throws LinkageError {
        //
        // Try to load the class using the given class loader (if any). If that fails (or
        // none is provided), we try to load the class a few more ways before giving up.
        //
        // Calling Class.forName() doesn't always work. For example, if Ice.jar is installed
        // as an extension (in $JAVA_HOME/jre/lib/ext), calling Class.forName(name) uses the
        // extension class loader, which will not look in CLASSPATH for the target class.
        //

        Class<?> c = null;

        if (cl != null) {
            c = loadClass(className, cl);
        }

        //
        // Try using the current thread's class loader.
        //
        if (c == null) {
            try {
                cl = Thread.currentThread().getContextClassLoader();
                if (cl != null) {
                    c = loadClass(className, cl);
                }
            } catch (java.lang.SecurityException ex) {}
        }

        //
        // Try using Class.forName().
        //
        try {
            if (c == null) {
                c = Class.forName(className);
            }
        } catch (ClassNotFoundException ex) {
            // Ignore
        }

        //
        // Fall back to the system class loader (which knows about CLASSPATH).
        //
        if (c == null) {
            try {
                cl = ClassLoader.getSystemClassLoader();
                if (cl != null) {
                    c = loadClass(className, cl);
                }
            } catch (java.lang.SecurityException ex) {}
        }

        return c;
    }

    private static Class<?> loadClass(String className, ClassLoader cl) {
        if (cl != null) {
            try {
                return cl.loadClass(className);
            } catch (ClassNotFoundException ex) {
                // Ignore
            }
        }

        return null;
    }

    private static int getThreadPriorityProperty(Properties properties, String prefix) {
        String pri = properties.getProperty(prefix + ".ThreadPriority");
        if ("MIN_PRIORITY".equals(pri) || "java.lang.Thread.MIN_PRIORITY".equals(pri)) {
            return Thread.MIN_PRIORITY;
        } else if ("NORM_PRIORITY".equals(pri) || "java.lang.Thread.NORM_PRIORITY".equals(pri)) {
            return Thread.NORM_PRIORITY;
        } else if ("MAX_PRIORITY".equals(pri) || "java.lang.Thread.MAX_PRIORITY".equals(pri)) {
            return Thread.MAX_PRIORITY;
        }

        try {
            return Integer.parseInt(pri);
        } catch (NumberFormatException ex) {}
        return Thread.NORM_PRIORITY;
    }

    // TODO can we just move this to TestHelper?
    /**
     * Returns true if we're running on Android.
     *
     * @return true if running on Android, false otherwise
     * @hidden Public because it's used by the tests.
     */
    public static boolean isAndroid() {
        return System.getProperty("java.vm.name").startsWith("Dalvik");
    }

    private static byte stringToMajor(String str) {
        int pos = str.indexOf('.');
        if (pos == -1) {
            throw new ParseException("malformed version value in '" + str + "'");
        }

        String majStr = str.substring(0, pos);
        int majVersion;
        try {
            majVersion = Integer.parseInt(majStr);
        } catch (NumberFormatException ex) {
            throw new ParseException("invalid version value in '" + str + "'", ex);
        }

        if (majVersion < 1 || majVersion > 255) {
            throw new ParseException("range error in version '" + str + "'");
        }

        return (byte) majVersion;
    }

    private static byte stringToMinor(String str) {
        int pos = str.indexOf('.');
        if (pos == -1) {
            throw new ParseException("malformed version value in '" + str + "'");
        }

        String minStr = str.substring(pos + 1, str.length());
        int minVersion;
        try {
            minVersion = Integer.parseInt(minStr);
        } catch (NumberFormatException ex) {
            throw new ParseException("invalid version value in '" + str + "'", ex);
        }

        if (minVersion < 0 || minVersion > 255) {
            throw new ParseException("range error in version '" + str + "'");
        }

        return (byte) minVersion;
    }

    private static String majorMinorToString(byte major, byte minor) {
        StringBuilder str = new StringBuilder();
        str.append(major < 0 ? major + 255 : (int) major);
        str.append(".");
        str.append(minor < 0 ? minor + 255 : (int) minor);
        return str.toString();
    }

    /** The protocol version 1.0. */
    public static final ProtocolVersion Protocol_1_0 = new ProtocolVersion((byte) 1, (byte) 0);

    /** The encoding version 1.0. */
    public static final EncodingVersion Encoding_1_0 = new EncodingVersion((byte) 1, (byte) 0);

    /** The encoding version 1.1. */
    public static final EncodingVersion Encoding_1_1 = new EncodingVersion((byte) 1, (byte) 1);

    private static final java.lang.Object _processLoggerMutex = new java.lang.Object();
    private static Logger _processLogger;

    private Util() {}
}
