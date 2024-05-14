// Copyright (c) ZeroC, Inc.

using Test;

public class TestI : TestIntfDisp_
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }

    public TestI()
    {
    }

    override public void
    op(Ice.Current current)
    {
        test(Executor.isExecutorThread());
    }

    override public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
        test(Executor.isExecutorThread());
    }

    public override void sleep(int to, Ice.Current current)
    {
        Thread.Sleep(to);
    }

    override public void
    shutdown(Ice.Current current)
    {
        test(Executor.isExecutorThread());
        current.adapter.getCommunicator().shutdown();
    }
}

public class TestControllerI : TestIntfControllerDisp_
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }

    override public void
    holdAdapter(Ice.Current current)
    {
        test(Executor.isExecutorThread());
        _adapter.hold();
    }

    override public void
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
