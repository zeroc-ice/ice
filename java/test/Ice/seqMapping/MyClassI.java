// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class MyClassI extends Test.MyClass
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public Serialize.Small opSerialSmallJava(Serialize.Small i, Ice.Holder<Serialize.Small> o, Ice.Current current)
    {
        o.value = i;
        return i;
    }

    public Serialize.Large opSerialLargeJava(Serialize.Large i, Ice.Holder<Serialize.Large> o, Ice.Current current)
    {
        o.value = i;
        return i;
    }

    public Serialize.Struct opSerialStructJava(Serialize.Struct i, Ice.Holder<Serialize.Struct> o, Ice.Current current)
    {
        o.value = i;
        return i;
    }
}
