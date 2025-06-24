// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The optional type.
 *
 * <p>An optional value is encoded with a specific optional format. This optional format describes
 * how the data is encoded and how it can be skipped by the unmarshaling code if the optional is not
 * known to the receiver.
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
    /** "Size encoding" using 1 to 5 bytes, e.g., enum, class identifier. */
    Size(4),
    /** "Size encoding" using 1 to 5 bytes followed by data, e.g., string, fixed size struct, or containers whose
     * size can be computed prior to marshaling. */
    VSize(5),
    /** Fixed size using 4 bytes followed by data, e.g., variable-size struct, container. */
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
        switch (v) {
            case 0:
                return F1;
            case 1:
                return F2;
            case 2:
                return F4;
            case 3:
                return F8;
            case 4:
                return Size;
            case 5:
                return VSize;
            case 6:
                return FSize;
            case 7:
                return Class;
            default:
                throw new IllegalArgumentException();
        }
    }

    private final int _value;
}
