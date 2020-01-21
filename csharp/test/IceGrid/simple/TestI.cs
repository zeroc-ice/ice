//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public sealed class TestIntf : Test.ITestIntf
{
    public void
    shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();
}
