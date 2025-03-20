// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Globalization;
using System.Net.Security;

namespace Ice;

/// <summary>
/// A class that encapsulates data to initialize a communicator.
/// </summary>
public sealed record class InitializationData
{
    /// <summary>
    /// Gets or sets the properties for the communicator.
    /// </summary>
    public Properties? properties { get; set; }

    /// <summary>
    /// Gets or sets the logger for the communicator.
    /// </summary>
    public Logger? logger { get; set; }

    /// <summary>
    /// Gets or sets the communicator observer used by the Ice run-time.
    /// </summary>
    public Instrumentation.CommunicatorObserver? observer { get; set; }

    /// <summary>
    /// Gets or sets the thread start hook for the communicator.
    /// </summary>
    /// <value>
    /// The Ice run time calls this hook for each new thread it creates. The call is made by the newly-started thread.
    /// </value>
    public Action? threadStart { get; set; }

    /// <summary>
    /// Gets or sets the thread stop hook for the communicator.
    /// </summary>
    /// <value>
    /// The Ice run time calls stop before it destroys a thread. The call is made by thread that is about to be
    /// destroyed.
    /// </value>
    public Action? threadStop { get; set; }

    /// <summary>
    /// Gets or sets the executor for the communicator.
    /// </summary>
    public Action<Action, Connection?>? executor { get; set; }

    /// <summary>
    /// Gets or sets the batch request interceptor.
    /// </summary>
    public Action<BatchRequest, int, int>? batchRequestInterceptor { get; set; }

    /// <summary>
    /// Gets or sets the value factory manager.
    /// </summary>
    public ValueFactoryManager? valueFactoryManager { get; set; }

    /// <summary>
    /// Gets or sets the <see cref="SslClientAuthenticationOptions"/> used by the client-side ssl transport.
    /// </summary>
    public SslClientAuthenticationOptions? clientAuthenticationOptions { get; set; }
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
    [Obsolete("Use Ice.Properties() constructor instead.")]
    public static Properties createProperties() => new();

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
    [Obsolete("Use Ice.Properties(ref string[], Ice.Properties) constructor instead.")]
    public static Properties createProperties(ref string[] args) => new(ref args, null);

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
    [Obsolete("Use Ice.Properties(ref string[], Ice.Properties) constructor instead.")]
    public static Properties createProperties(ref string[] args, Properties defaults) => new(ref args, defaults);

    /// <summary>
    /// Creates a communicator.
    /// </summary>
    /// <param name="args">A command-line argument vector. Any Ice-related options
    /// in this vector are used to initialize the communicator.
    /// This method modifies the argument vector by removing any Ice-related options.</param>
    /// <param name="initData">Additional initialization data. Property settings in args
    /// override property settings in initData.</param>
    /// <returns>The initialized communicator.</returns>
    public static Communicator initialize(ref string[] args, InitializationData? initData = null)
    {
        initData = initData is null ? new InitializationData() : initData with { };
        initData.properties = new Properties(ref args, initData.properties);
        var result = new Communicator(initData);
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
        var initData = new InitializationData();
        initData.properties = new Properties();
        initData.properties.load(configFile);
        return initialize(ref args, initData);
    }

    /// <summary>
    /// Creates a communicator.
    /// </summary>
    /// <param name="initData">Additional initialization data.</param>
    /// <returns>The initialized communicator.</returns>
    public static Communicator initialize(InitializationData? initData = null)
    {
        initData = initData is null ? new InitializationData() : initData with { };
        // TODO: some tests rely on updating the properties after initialize.
        // initData.properties = initData.properties?.Clone();
        var result = new Communicator(initData);
        string[] args = [];
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
        var initData = new InitializationData();
        initData.properties = new Properties();
        initData.properties.load(configFile);
        return initialize(initData);
    }

    /// <summary>
    /// Converts a string to an object identity.
    /// </summary>
    /// <param name="s">The string to convert.</param>
    /// <returns>The converted object identity.</returns>
    public static Identity stringToIdentity(string s)
    {
        var ident = new Identity();

        //
        // Find unescaped separator; note that the string may contain an escaped
        // backslash before the separator.
        //
        int slash = -1, pos = 0;
        while ((pos = s.IndexOf('/', pos)) != -1)
        {
            int escapes = 0;
            while (pos - escapes > 0 && s[pos - escapes - 1] == '\\')
            {
                escapes++;
            }

            //
            // We ignore escaped escapes
            //
            if (escapes % 2 == 0)
            {
                if (slash == -1)
                {
                    slash = pos;
                }
                else
                {
                    //
                    // Extra unescaped slash found.
                    //
                    throw new ParseException($"unescaped backslash in identity string '{s}'");
                }
            }
            pos++;
        }

        if (slash == -1)
        {
            ident.category = "";
            try
            {
                ident.name = Ice.UtilInternal.StringUtil.unescapeString(s, 0, s.Length, "/");
            }
            catch (ArgumentException e)
            {
                throw new ParseException($"invalid name in identity string '{s}'", e);
            }
        }
        else
        {
            try
            {
                ident.category = Ice.UtilInternal.StringUtil.unescapeString(s, 0, slash, "/");
            }
            catch (ArgumentException e)
            {
                throw new ParseException($"invalid category in identity string '{s}'", e);
            }
            if (slash + 1 < s.Length)
            {
                try
                {
                    ident.name = Ice.UtilInternal.StringUtil.unescapeString(s, slash + 1, s.Length, "/");
                }
                catch (ArgumentException e)
                {
                    throw new ParseException($"invalid name in identity string '{s}'", e);
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
        if (ident.category.Length == 0)
        {
            return Ice.UtilInternal.StringUtil.escapeString(ident.name, "/", toStringMode);
        }
        else
        {
            return Ice.UtilInternal.StringUtil.escapeString(ident.category, "/", toStringMode) + '/' +
                Ice.UtilInternal.StringUtil.escapeString(ident.name, "/", toStringMode);
        }
    }

    /// <summary>
    /// This method is deprecated. Use System.Guid instead.
    /// </summary>
    /// <returns>A new UUID.</returns>
    [Obsolete("This method is deprecated. Use System.Guid instead.")]
    public static string generateUUID() => Guid.NewGuid().ToString().ToUpper(System.Globalization.CultureInfo.InvariantCulture);

    /// <summary>
    /// Compares the object identities of two proxies.
    /// </summary>
    /// <param name="lhs">The first proxy to compare.</param>
    /// <param name="rhs">The second proxy to compare.</param>
    /// <returns>-1 if the identity in lhs compares less than the identity in rhs; 0 if the identities compare equal;
    /// 1, otherwise.</returns>
    public static int proxyIdentityCompare(ObjectPrx? lhs, ObjectPrx? rhs)
    {
        if (lhs == null && rhs == null)
        {
            return 0;
        }
        else if (lhs == null && rhs != null)
        {
            return -1;
        }
        else if (lhs != null && rhs == null)
        {
            return 1;
        }
        else
        {
            Identity lhsIdentity = lhs!.ice_getIdentity();
            Identity rhsIdentity = rhs!.ice_getIdentity();
            int n;
            n = string.CompareOrdinal(lhsIdentity.name, rhsIdentity.name);
            if (n != 0)
            {
                return n;
            }
            return string.CompareOrdinal(lhsIdentity.category, rhsIdentity.category);
        }
    }

    /// <summary>
    /// Compares the object identities and facets of two proxies.
    /// </summary>
    /// <param name="lhs">The first proxy to compare.</param>
    /// <param name="rhs">The second proxy to compare.</param>
    /// <returns>-1 if the identity and facet in lhs compare
    /// less than the identity and facet in rhs; 0 if the identities
    /// and facets compare equal; 1, otherwise.</returns>
    public static int proxyIdentityAndFacetCompare(ObjectPrx? lhs, ObjectPrx? rhs)
    {
        if (lhs == null && rhs == null)
        {
            return 0;
        }
        else if (lhs == null && rhs != null)
        {
            return -1;
        }
        else if (lhs != null && rhs == null)
        {
            return 1;
        }
        else
        {
            Identity lhsIdentity = lhs!.ice_getIdentity();
            Identity rhsIdentity = rhs!.ice_getIdentity();
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

            string lhsFacet = lhs.ice_getFacet();
            string rhsFacet = rhs.ice_getFacet();
            if (lhsFacet == null && rhsFacet == null)
            {
                return 0;
            }
            else if (lhsFacet == null)
            {
                return -1;
            }
            else if (rhsFacet == null)
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
        lock (_processLoggerMutex)
        {
            _processLogger ??= new ConsoleLoggerI(AppDomain.CurrentDomain.FriendlyName);
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
    public static string stringVersion() => "3.8.0-alpha.0"; // "A.B.C", with A=major, B=minor, C=patch

    /// <summary>
    /// Returns the Ice version as an integer in the form A.BB.CC, where A
    /// indicates the major version, BB indicates the minor version, and CC
    /// indicates the patch level. For example, for Ice 3.3.1, the returned value is 30301.
    /// </summary>
    /// <returns>The Ice version.</returns>
    public static int intVersion() => 30850; // AABBCC, with AA=major, BB=minor, CC=patch

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
    public static string protocolVersionToString(Ice.ProtocolVersion v) => majorMinorToString(v.major, v.minor);

    /// <summary>
    /// Converts an encoding version to a string.
    /// </summary>
    /// <param name="v">The encoding version to convert.</param>
    /// <returns>The converted string.</returns>
    public static string encodingVersionToString(Ice.EncodingVersion v) => majorMinorToString(v.major, v.minor);

    private static void stringToMajorMinor(string str, out byte major, out byte minor)
    {
        int pos = str.IndexOf('.', StringComparison.Ordinal);
        if (pos == -1)
        {
            throw new ParseException($"malformed version value in '{str}'");
        }

        string majStr = str[..pos];
        string minStr = str[(pos + 1)..];
        int majVersion;
        int minVersion;
        try
        {
            majVersion = int.Parse(majStr, CultureInfo.InvariantCulture);
            minVersion = int.Parse(minStr, CultureInfo.InvariantCulture);
        }
        catch (FormatException ex)
        {
            throw new ParseException($"invalid version value in '{str}'", ex);
        }

        if (majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
        {
            throw new ParseException($"range error in version '{str}'");
        }

        major = (byte)majVersion;
        minor = (byte)minVersion;
    }

    private static string majorMinorToString(byte major, byte minor) => $"{major}.{minor}";

    public static void registerPluginFactory(string name, PluginFactory factory, bool loadOnInit) => PluginManagerI.registerPluginFactory(name, factory, loadOnInit);

    public static readonly ProtocolVersion currentProtocol =
        new ProtocolVersion(Ice.Internal.Protocol.protocolMajor, Ice.Internal.Protocol.protocolMinor);

    public static readonly EncodingVersion currentProtocolEncoding =
        new EncodingVersion(
            Ice.Internal.Protocol.protocolEncodingMajor,
            Ice.Internal.Protocol.protocolEncodingMinor);

    public static readonly EncodingVersion currentEncoding =
        new EncodingVersion(Ice.Internal.Protocol.encodingMajor, Ice.Internal.Protocol.encodingMinor);

    public static readonly ProtocolVersion Protocol_1_0 = new ProtocolVersion(1, 0);

    public static readonly EncodingVersion Encoding_1_0 = new EncodingVersion(1, 0);
    public static readonly EncodingVersion Encoding_1_1 = new EncodingVersion(1, 1);

    private static readonly object _processLoggerMutex = new object();
    private static Logger? _processLogger;
}
