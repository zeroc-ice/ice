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
    public void
    shutdown_async(Test.AMD_MyClass_shutdown cb,
                   Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public void
    opSerialSmallJava_async(Test.AMD_MyClass_opSerialSmallJava cb,
                            Serialize.Small i,
                            Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public void
    opSerialLargeJava_async(Test.AMD_MyClass_opSerialLargeJava cb,
                            Serialize.Large i,
                            Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public void opSerialStructJava_async(Test.AMD_MyClass_opSerialStructJava cb,
                                         Serialize.Struct i,
                                         Ice.Current current)
    {
        cb.ice_response(i, i);
    }
}
