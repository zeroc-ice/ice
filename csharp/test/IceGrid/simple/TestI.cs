//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public sealed class TestI : Test.TestIntf
{
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.GetCommunicator().shutdown();
    }
}
