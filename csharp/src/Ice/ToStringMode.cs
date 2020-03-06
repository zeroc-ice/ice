//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    /// <summary>The output mode or format for ToString on an Ice proxy or Ice identity.</summary>
    public enum ToStringMode
    {
        /// <summary>Characters with ordinal values greater than 127 are kept as-is in the resulting string.
        /// Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.).</summary>
        Unicode = 1,

        /// <summary>Characters with ordinal values greater than 127 are encoded as universal character names in
        /// the resulting string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
        /// Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.)
        /// or \\unnnn.</summary>
        ASCII = 2,

        /// <summary>Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using
        /// octal escapes. Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or
        /// an octal escape. Use this mode to generate strings compatible with Ice 3.7 and earlier Ice versions.
        /// </summary>
        Compat = 4,

        /// <summary>Generates Unicode-style stringified proxies and identities in a format compatible with Ice 3.7.
        /// </summary>
        CompatUnicode = Compat | Unicode,

        /// <summary>Generates ASCII-style stringified proxies and identities in a format compatible with Ice 3.7.
        /// </summary>
        CompatASCII = Compat | ASCII
    }
}
