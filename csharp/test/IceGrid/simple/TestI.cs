//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public sealed class TestI : Test.TestIntfDisp_
{
    public override void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
