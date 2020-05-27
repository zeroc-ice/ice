//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    public readonly partial struct Identity
    {
        public static readonly Identity Empty = new Identity("", "");
        public static Identity Parse(string s)
        {
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
                    throw new FormatException($"invalid name in identity `{s}", ex);
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
                    throw new FormatException($"invalid category in identity `{s}", ex);
                }

                if (slash + 1 < s.Length)
                {
                    try
                    {
                        name = StringUtil.UnescapeString(s, slash + 1, s.Length, "/");
                    }
                    catch (ArgumentException ex)
                    {
                        throw new FormatException($"invalid name in identity `{s}", ex);
                    }
                }
            }

            if (string.IsNullOrEmpty(name))
            {
                throw new FormatException($"empty name in identity `{s}'");
            }

            return new Identity(name, category);
        }

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

        public override string ToString() => ToString(ToStringMode.Unicode);

        /// <summary>
        /// Converts an object identity to a string.
        /// </summary>
        /// <param name="mode">Specifies if and how non-printable ASCII characters are escaped in the result.</param>
        /// <returns>The string representation of the object identity.</returns>
        public string ToString(ToStringMode mode)
        {
            if (string.IsNullOrEmpty(Category))
            {
                return StringUtil.EscapeString(Name, "/", mode);
            }
            else
            {
                return StringUtil.EscapeString(Category, "/", mode) + '/' +
                       StringUtil.EscapeString(Name, "/", mode);
            }
        }
    }
}
