// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Globalization;

namespace ZeroC.Ice
{
    // Extends the Slice-defined Encoding struct
    public readonly partial struct Encoding
    {
        // The encodings known to the Ice runtime.

        /// <summary>Version 1.0 of the Ice encoding, supported by Ice 1.0 to Ice 3.7.</summary>
        public static readonly Encoding V10 = new Encoding(1, 0);

        /// <summary>Version 1.1 of the Ice encoding, supported since Ice 3.5.</summary>
        public static readonly Encoding V11 = new Encoding(1, 1);

        /// <summary>Version 2.0 of the Ice encoding, supported since Ice 4.0.</summary>
        public static readonly Encoding V20 = new Encoding(2, 0);

        internal bool IsSupported => this == V11 || this == V20;

        /// <summary>Parses a string into an Encoding.</summary>
        /// <param name="str">The string to parse.</param>
        /// <returns>A new encoding.</returns>
        public static Encoding Parse(string str)
        {
            int pos = str.IndexOf('.');
            if (pos == -1)
            {
                throw new FormatException($"malformed encoding string `{str}'");
            }

            string majStr = str[..pos];
            string minStr = str[(pos + 1)..];
            try
            {
                byte major = byte.Parse(majStr, CultureInfo.InvariantCulture);
                byte minor = byte.Parse(minStr, CultureInfo.InvariantCulture);
                return new Encoding(major, minor);
            }
            catch (FormatException)
            {
                throw new FormatException($"malformed encoding string `{str}'");
            }
        }

        /// <summary>Attempts to parse a string into an Encoding.</summary>
        /// <param name="str">The string to parse.</param>
        /// <param name="encoding">The resulting encoding.</param>
        /// <returns>True if the parsing succeeded and encoding contains the result; otherwise, false.</returns>
        public static bool TryParse(string str, out Encoding encoding)
        {
            try
            {
                encoding = Parse(str);
                return true;
            }
            catch (FormatException)
            {
                encoding = default;
                return false;
            }
        }

        /// <inheritdoc/>
        public override string ToString() => $"{Major}.{Minor}";

        internal void CheckSupported()
        {
            if (!IsSupported)
            {
                throw new NotSupportedException(
                    $"Ice encoding `{this}' is not supported by this Ice runtime ({Runtime.StringVersion})");
            }
        }
    }
}
