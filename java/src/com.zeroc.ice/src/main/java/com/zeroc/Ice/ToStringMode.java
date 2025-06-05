// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The output mode for xxxToString method such as identityToString and proxyToString. The actual
 * encoding format for the string is the same for all modes: you don't need to specify an encoding
 * format or mode when reading such a string.
 */
public enum ToStringMode {
    /**
     * Characters with ordinal values greater than 127 are kept as-is in the resulting string.
     * Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n
     * (etc.) or \\unnnn.
     */
    Unicode(0),
    /**
     * Characters with ordinal values greater than 127 are encoded as universal character names in
     * the resulting string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
     * Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n
     * (etc.) or \\unnnn.
     */
    ASCII(1),
    /**
     * Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes
     * using octal escapes. Characters with ordinal values 127 and below are encoded as \\t, \\n
     * (etc.) or an octal escape. Use this mode to generate strings compatible with Ice 3.6 and
     * earlier.
     */
    Compat(2);

    /**
     * Returns the integer value of this enumerator.
     *
     * @return the integer value of this enumerator
     */
    public int value() {
        return _value;
    }

    /**
     * Returns the enumerator corresponding to the given integer value.
     *
     * @param v the integer value of the enumerator
     * @return the enumerator corresponding to the given integer value, or {@code null} if no such enumerator exists
     */
    public static ToStringMode valueOf(int v) {
        switch (v) {
            case 0:
                return Unicode;
            case 1:
                return ASCII;
            case 2:
                return Compat;
        }
        return null;
    }

    private ToStringMode(int v) {
        _value = v;
    }

    private final int _value;
}
