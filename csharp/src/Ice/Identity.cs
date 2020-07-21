//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

namespace ZeroC.Ice
{
    public readonly partial struct Identity
    {
        public static readonly Identity Empty = new Identity("", "");

        /// <summary>Converts the string representation of an identity to its equivalent Identity struct.</summary>
        /// <param name="s">A string [escapedCategory/]escapedName.</param>
        /// <param name="uriFormat">When true (the default), the string is a relative URI. When false, the string
        /// uses the ice1 format. See also <see cref="ToStringMode"/>.</param>
        /// <exception cref="FormatException">s is not in the correct format.</exception>
        /// <returns>An Identity equivalent to the identity contained in s.</returns>
        public static Identity Parse(string s, bool uriFormat = true)
        {
            if (!uriFormat) // TODO: reverse order in post-review cleanup
            {
                // Find unescaped separator; note that the string may contain an escaped backslash before the separator.
                int slash = -1, pos = 0;
                while ((pos = s.IndexOf('/', pos)) != -1)
                {
                    int escapes = 0;
                    while (pos - escapes > 0 && s[pos - escapes - 1] == '\\')
                    {
                        escapes++;
                    }

                    // We ignore escaped escapes
                    if (escapes % 2 == 0)
                    {
                        if (slash == -1)
                        {
                            slash = pos;
                        }
                        else
                        {
                            // Extra unescaped slash found.
                            throw new FormatException($"unescaped backslash in identity `{s}'");
                        }
                    }
                    pos++;
                }

                string category;
                string? name = null;
                if (slash == -1)
                {
                    try
                    {
                        name = StringUtil.UnescapeString(s, 0, s.Length, "/");
                    }
                    catch (ArgumentException ex)
                    {
                        throw new FormatException($"invalid name in identity `{s}'", ex);
                    }
                    category = "";
                }
                else
                {
                    try
                    {
                        category = StringUtil.UnescapeString(s, 0, slash, "/");
                    }
                    catch (ArgumentException ex)
                    {
                        throw new FormatException($"invalid category in identity `{s}'", ex);
                    }

                    if (slash + 1 < s.Length)
                    {
                        try
                        {
                            name = StringUtil.UnescapeString(s, slash + 1, s.Length, "/");
                        }
                        catch (ArgumentException ex)
                        {
                            throw new FormatException($"invalid name in identity `{s}'", ex);
                        }
                    }
                }

                return name?.Length > 0 ? new Identity(name, category) :
                    throw new FormatException($"invalid empty name in identity `{s}'");
            }
            else
            {
                string[] segments = s.Split('/');
                Debug.Assert(segments.Length > 0);
                (string name, string category) = segments.Length switch
                {
                    1 => (Uri.UnescapeDataString(segments[0]), ""),
                    2 => (Uri.UnescapeDataString(segments[1]), Uri.UnescapeDataString(segments[0])),
                    _ => throw new FormatException($"too many path segments in identity `{s}'"),
                };

                return name.Length > 0 ? new Identity(name, category) :
                    throw new FormatException($"invalid empty name in identity `{s}'");
            }
        }

        /// <summary>Converts the string representation of an identity to its equivalent Identity struct.</summary>
        /// <param name="s">A string containing an identity to convert.</param>
        /// <param name="identity">When this method returns, contains an Identity struct equivalent to the identity
        /// contained in s, if the conversion succeeded, or a default identity if failed. </param>
        /// <returns>true if s was converted successfully; otherwise, false.</returns>
        public static bool TryParse(string s, out Identity identity)
        {
            try
            {
                identity = Parse(s);
                return true;
            }
            catch (Exception)
            {
                identity = default;
                return false;
            }
        }

        /// <summary>Converts an object identity to a relative URI string. All characters in category and name are
        /// percent escaped, except unreserved characters.</summary>
        /// <returns>A relative URI string [escapedCategory/]escapedName for this identity.</returns>
        public override string ToString()
        {
            if (string.IsNullOrEmpty(Name))
            {
                // This struct was default initialized (null) or poorly initialized (empty name).
                return "";
            }
            Debug.Assert(Category != null);

            if (Category.Length == 0)
            {
                return Uri.EscapeDataString(Name);
            }
            else
            {
                return $"{Uri.EscapeDataString(Category)}/{Uri.EscapeDataString(Name)}";
            }
        }

        /// <summary>Converts an object identity to a string, using the format specified by ToStringMode.</summary>
        /// <param name="mode">Specifies if and how non-printable ASCII characters are escaped in the result. See
        /// <see cref="ToStringMode"/>.</param>
        /// <returns>The string representation of the object identity.</returns>
        public string ToString(ToStringMode mode)
        {
            if (string.IsNullOrEmpty(Name))
            {
                return "";
            }
            Debug.Assert(Category != null);

            string escapedName = StringUtil.EscapeString(Name, mode, '/');

            if (Category.Length == 0)
            {
                return escapedName;
            }
            else
            {
                string escapedCategory = StringUtil.EscapeString(Category, mode, '/');
                return $"{escapedCategory}/{escapedName}";
            }
        }
    }

    /// <summary>The output mode or format for <see cref="Identity.ToString(ToStringMode)"/>.</summary>
    public enum ToStringMode : byte
    {
        /// <summary>Characters with ordinal values greater than 127 are kept as-is in the resulting string.
        /// Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.). This
        /// corresponds to the default mode with Ice 3.7.</summary>
        Unicode,

        /// <summary>Characters with ordinal values greater than 127 are encoded as universal character names in
        /// the resulting string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
        /// Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.)
        /// or \\unnnn. This is an optional mode provided by Ice 3.7.</summary>
        ASCII,

        /// <summary>Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using
        /// octal escapes. Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or
        /// an octal escape. This is the format used by Ice 3.6 and earlier Ice versions.</summary>
        Compat
    }
}
