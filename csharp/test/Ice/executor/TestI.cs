// Copyright (c) ZeroC, Inc.

using Test;

public class TestI : TestIntfDisp_
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    public TestI()
    {
    }

    public override void
    op(Ice.Current current)
    {
        test(Executor.isExecutorThread());
    }

    public override void
    opWithPayload(byte[] seq, Ice.Current current)
    {
        test(Executor.isExecutorThread());
    }

    public override void sleep(int to, Ice.Current current)
    {
        Thread.Sleep(to);
    }

    public override void
    shutdown(Ice.Current current)
    {
        test(Executor.isExecutorThread());
        current.adapter.getCommunicator().shutdown();
    }
}

public class TestControllerI : TestIntfControllerDisp_
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    public override void
    holdAdapter(Ice.Current current)
    {
        test(Executor.isExecutorThread());
        _adapter.hold();
    }

    public override void
    resumeAdapter(Ice.Current current)
    {
        test(Executor.isExecutorThread());
        _adapter.activate();
    }

    public
    TestControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    private Ice.ObjectAdapter _adapter;
};
