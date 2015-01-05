// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
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

    public test.TestPB.Message opMessage(test.TestPB.Message i, Ice.Holder<test.TestPB.Message> o, Ice.Current current)
    {
        o.value = i;
        return i;
    }

    public void opMessageAMD_async(Test.AMD_MyClass_opMessageAMD amdCB, test.TestPB.Message i, Ice.Current current)
    {
        amdCB.ice_response(i, i);
    }
}
