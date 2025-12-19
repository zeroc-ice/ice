// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Utility methods for the Ice runtime.
/// </summary>
public sealed class Util
{
    /// <summary>
    /// Creates a new empty property set.
    /// </summary>
    /// <returns>A new empty property set.</returns>
    /// <remarks>This method is provided for backwards compatibility. New code should call the
    /// <see cref="Properties()" /> constructor directly.</remarks>
    public static Properties createProperties() => new();

    /// <summary>
    /// Creates a property set initialized from command-line arguments and a default property set.
    /// </summary>
    /// <param name="args">The command-line arguments.</param>
    /// <param name="defaults">Default values for the new property set.</param>
    /// <returns>A new property set.</returns>
    /// <remarks>This method is provided for backwards compatibility. New code should call the
    /// <see cref="Properties(ref string[], Properties?)" /> constructor directly.</remarks>
    public static Properties createProperties(ref string[] args, Properties? defaults = null) =>
        new(ref args, defaults);

    /// <summary>
    /// Creates a new communicator.
    /// </summary>
    /// <param name="initData">Options for the new communicator.</param>
    /// <returns>The new communicator.</returns>
    /// <remarks>This method is provided for backwards compatibility. New code should call the
    /// <see cref="Communicator(InitializationData?)" /> constructor directly.</remarks>
    public static Communicator initialize(InitializationData? initData = null) => new(initData);

    /// <summary>
    /// Creates a new communicator, using Ice properties parsed from command-line arguments.
    /// </summary>
    /// <param name="args">Command-line arguments.</param>
    /// <returns>The new communicator.</returns>
    /// <remarks>This method is provided for backwards compatibility. New code should call the
    /// <see cref="Communicator(ref string[])" /> constructor directly.</remarks>
    public static Communicator initialize(ref string[] args) => new(ref args);

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
    public static string generateUUID() =>
        Guid.NewGuid().ToString().ToUpper(System.Globalization.CultureInfo.InvariantCulture);

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
            if (_processLogger is null)
            {
                _processLogger = new ConsoleLoggerI(AppDomain.CurrentDomain.FriendlyName);
                _ownProcessLogger = true;
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
            if (_ownProcessLogger)
            {
                _processLogger!.Dispose();
            }
            _processLogger = logger;
            _ownProcessLogger = false;
        }
    }

    /// <summary>
    /// Returns the Ice version in the form A.B.C, where A indicates the
    /// major version, B indicates the minor version, and C indicates the
    /// patch level.
    /// </summary>
    /// <returns>The Ice version.</returns>
    public static string stringVersion() => "3.9.0-alpha.0"; // "A.B.C", with A=major, B=minor, C=patch

    /// <summary>
    /// Returns the Ice version as an integer in the form AABBCC, where AA
    /// indicates the major version, BB indicates the minor version, and CC
    /// indicates the patch level. For example, for Ice 3.8.1, the returned value is 30801.
    /// </summary>
    /// <returns>The Ice version.</returns>
    public static int intVersion() => 30950; // AABBCC, with AA=major, BB=minor, CC=patch

    public static readonly EncodingVersion Encoding_1_0 = new(1, 0);
    public static readonly EncodingVersion Encoding_1_1 = new(1, 1);

    private static readonly object _processLoggerMutex = new object();
    private static Logger? _processLogger;
    private static bool _ownProcessLogger;
}
