// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The optional format, used for marshaling optional fields and arguments.
 * It describes how the data is marshaled and how it can be skipped by the unmarshaling code
 * if the optional isn't known to the receiver.
 */
public enum OptionalFormat {
    /** Fixed 1-byte encoding. */
    F1(0),
    /** Fixed 2-byte encoding. */
    F2(1),
    /** Fixed 4-byte encoding. */
    F4(2),
    /** Fixed 8-byte encoding. */
    F8(3),
    /** "Size encoding" using either 1 or 5 bytes. Used by enums, class identifiers, etc. */
    Size(4),
    /** Variable "size encoding" using either 1 or 5 bytes followed by data.
     *  Used by strings, fixed size structs, and containers whose size can be computed prior to marshaling. */
    VSize(5),
    /** Fixed "size encoding" using 4 bytes followed by data.
     *  Used by variable-size structs, and containers whose sizes can't be computed prior to unmarshaling. */
    FSize(6),
    /** Class instance. No longer supported. */
    Class(7);

    private OptionalFormat(int value) {
        _value = value;
    }

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
    public static OptionalFormat valueOf(int v) {
        return switch (v) {
            case 0 -> F1;
            case 1 -> F2;
            case 2 -> F4;
            case 3 -> F8;
            case 4 -> Size;
            case 5 -> VSize;
            case 6 -> FSize;
            case 7 -> Class;
            default -> throw new IllegalArgumentException();
        };
    }

    private final int _value;
}
