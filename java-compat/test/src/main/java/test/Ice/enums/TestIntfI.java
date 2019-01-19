//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.enums;

import test.Ice.enums.Test.*;

public final class TestIntfI extends _TestIntfDisp
{
    @Override
    public ByteEnum
    opByte(ByteEnum b1, ByteEnumHolder b2, Ice.Current current)
    {
        b2.value = b1;
        return b1;
    }

    @Override
    public ShortEnum
    opShort(ShortEnum s1, ShortEnumHolder s2, Ice.Current current)
    {
        s2.value = s1;
        return s1;
    }

    @Override
    public IntEnum
    opInt(IntEnum i1, IntEnumHolder i2, Ice.Current current)
    {
        i2.value = i1;
        return i1;
    }

    @Override
    public SimpleEnum
    opSimple(SimpleEnum s1, SimpleEnumHolder s2, Ice.Current current)
    {
        s2.value = s1;
        return s1;
    }

    @Override
    public ByteEnum[]
    opByteSeq(ByteEnum[] b1, ByteEnumSeqHolder b2, Ice.Current current)
    {
        b2.value = b1;
        return b1;
    }

    @Override
    public ShortEnum[]
    opShortSeq(ShortEnum[] s1, ShortEnumSeqHolder s2, Ice.Current current)
    {
        s2.value = s1;
        return s1;
    }

    @Override
    public IntEnum[]
    opIntSeq(IntEnum[] i1, IntEnumSeqHolder i2, Ice.Current current)
    {
        i2.value = i1;
        return i1;
    }

    @Override
    public SimpleEnum[]
    opSimpleSeq(SimpleEnum[] s1, SimpleEnumSeqHolder s2, Ice.Current current)
    {
        s2.value = s1;
        return s1;
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
