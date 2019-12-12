//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    public partial struct Identity
    {
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
                        throw new ArgumentException($"unescaped backslash in identity `{s}'");
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
                    name = IceUtilInternal.StringUtil.unescapeString(s, 0, s.Length, "/");
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
                    category = IceUtilInternal.StringUtil.unescapeString(s, 0, slash, "/");
                }
                catch (ArgumentException ex)
                {
                    throw new FormatException($"invalid category in identity `{s}", ex);
                }

                if (slash + 1 < s.Length)
                {
                    try
                    {
                        name = IceUtilInternal.StringUtil.unescapeString(s, slash + 1, s.Length, "/");
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

        public static bool TryParse(string s, out Identity? identity)
        {
            try
            {
                identity = Parse(s);
                return true;
            }
            catch (Exception)
            {
                identity = null;
                return false;
            }
        }

        public override string ToString()
        {
            return ToString(ToStringMode.Unicode);
        }

        public string ToString(Communicator communicator)
        {
            return ToString(communicator.ToStringMode);
        }

        /// <summary>
        /// Converts an object identity to a string.
        /// </summary>
        /// <param name="mode">Specifies if and how non-printable ASCII characters are escaped in the result.</param>
        /// <returns>The string representation of the object identity.</returns>
        public string ToString(ToStringMode mode)
        {
            if (string.IsNullOrEmpty(category))
            {
                return IceUtilInternal.StringUtil.escapeString(name, "/", mode);
            }
            else
            {
                return IceUtilInternal.StringUtil.escapeString(category, "/", mode) + '/' +
                       IceUtilInternal.StringUtil.escapeString(name, "/", mode);
            }
        }
    }
}
