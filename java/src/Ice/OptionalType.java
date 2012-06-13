// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * The optional type.
 *
 * An optional value is encoded with a specific optional type. This optional
 * type describes how the data is encoded and how it can be skipped by the
 * unmarshaling code if the optional is not known to the receiver.
 *
 **/
public enum OptionalType
{
    F1(0),
    F2(1),
    F4(2),
    F8(3),
    Size(4),
    VSize(5),
    FSize(6),
    EndMarker(7);

    private
    OptionalType(int value)
    {
        _value = value;
    }

    public int
    value()
    {
        return _value;
    }

    public static OptionalType
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
            return EndMarker;
        default:
            throw new IllegalArgumentException();
        }
    }

    private int _value;
}
