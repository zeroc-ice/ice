// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class TestI : TestIntfDisp_
{
    public TestI()
    {
    }

    public override Test.ByteEnum opByte(Test.ByteEnum b1, out Test.ByteEnum b2, Ice.Current current)
    {
        b2 = b1;
        return b1;
    }

    public override Test.ShortEnum opShort(Test.ShortEnum s1, out Test.ShortEnum s2, Ice.Current current)
    {
        s2 = s1;
        return s1;
    }

    public override Test.IntEnum opInt(Test.IntEnum i1, out Test.IntEnum i2, Ice.Current current)
    {
        i2 = i1;
        return i1;
    }

    public override Test.SimpleEnum opSimple(Test.SimpleEnum s1, out Test.SimpleEnum s2, Ice.Current current)
    {
        s2 = s1;
        return s1;
    }

    public override Test.ByteEnum[] opByteSeq(Test.ByteEnum[] b1, out Test.ByteEnum[] b2, Ice.Current current)
    {
        b2 = b1;
        return b1;
    }

    public override Test.ShortEnum[] opShortSeq(Test.ShortEnum[] s1, out Test.ShortEnum[] s2, Ice.Current current)
    {
        s2 = s1;
        return s1;
    }

    public override Test.IntEnum[] opIntSeq(Test.IntEnum[] i1, out Test.IntEnum[] i2, Ice.Current current)
    {
        i2 = i1;
        return i1;
    }

    public override Test.SimpleEnum[] opSimpleSeq(Test.SimpleEnum[] s1, out Test.SimpleEnum[] s2, Ice.Current current)
    {
        s2 = s1;
        return s1;
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
