// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * The optional type.
 *
 * An optional value is encoded with a specific optional format. This optional
 * format describes how the data is encoded and how it can be skipped by the
 * unmarshaling code if the optional is not known to the receiver.
 *
 **/
public enum OptionalFormat
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
    OptionalFormat(int value)
    {
        _value = value;
    }

    public int
    value()
    {
        return _value;
    }

    public static OptionalFormat
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
