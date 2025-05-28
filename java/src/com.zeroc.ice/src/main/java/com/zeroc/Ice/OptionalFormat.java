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
    F1(0),
    F2(1),
    F4(2),
    F8(3),
    Size(4),
    VSize(5),
    FSize(6),
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
