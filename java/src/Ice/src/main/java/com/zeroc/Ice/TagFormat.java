//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * The tag type.
 *
 * A tagged value is encoded with a specific tag format. This tag format
 * describes how the data is encoded and how it can be skipped by the
 * unmarshaling code if the value is not known to the receiver.
 *
 **/
public enum TagFormat
{
    F1(0),
    F2(1),
    F4(2),
    F8(3),
    Size(4),
    VSize(5),
    FSize(6),
    Class(7);

    private
    TagFormat(int value)
    {
        _value = value;
    }

    public int
    value()
    {
        return _value;
    }

    public static TagFormat
    valueOf(int v)
    {
        switch(v)
        {
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

    private int _value;
}
