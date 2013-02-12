// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.enums;

import test.Ice.enums.Test.*;

public final class TestIntfI extends _TestIntfDisp
{
    public ByteEnum
    opByte(ByteEnum b1, ByteEnumHolder b2, Ice.Current current)
    {
        b2.value = b1;
        return b1;
    }

    public ShortEnum
    opShort(ShortEnum s1, ShortEnumHolder s2, Ice.Current current)
    {
        s2.value = s1;
        return s1;
    }

    public IntEnum
    opInt(IntEnum i1, IntEnumHolder i2, Ice.Current current)
    {
        i2.value = i1;
        return i1;
    }

    public SimpleEnum
    opSimple(SimpleEnum s1, SimpleEnumHolder s2, Ice.Current current)
    {
        s2.value = s1;
        return s1;
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
