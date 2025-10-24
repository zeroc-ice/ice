// Copyright (c) ZeroC, Inc.

namespace Ice.ami;

public class TestI : Test.TestIntfDisp_
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    public TestI()
    {
    }

    public override void
    op(Ice.Current current)
    {
    }

    public override int
    opWithResult(Ice.Current current) => 15;

    public override void
    opWithUE(Ice.Current current) => throw new Test.TestIntfException();

    public override void
    opWithPayload(byte[] seq, Ice.Current current)
    {
    }

    public override void
    opBatch(Current current)
    {
        lock (_mutex)
        {
            ++_batchCount;
            Monitor.Pulse(_mutex);
        }
    }

    public override int
    opBatchCount(Ice.Current current)
    {
        lock (_mutex)
        {
            return _batchCount;
        }
    }

    public override bool
    waitForBatch(int count, Ice.Current current)
    {
        lock (_mutex)
        {
            while (_batchCount < count)
            {
                test(Monitor.Wait(_mutex, 10000));
            }
            bool result = count == _batchCount;
            _batchCount = 0;
            return result;
        }
    }

    public override void closeConnection(Ice.Current current)
    {
        // We can't wait for the connection to close - it would self-deadlock. So we just initiate the closure.
        _ = closeAsync();

        async Task closeAsync()
        {
            try
            {
                await current.con.closeAsync();
            }
            catch (System.Exception ex)
            {
                Console.WriteLine($"Connection close failed: {ex}");
                test(false);
            }
        }
    }

    public override void abortConnection(Ice.Current current) => current.con.abort();

    public override void
    sleep(int ms, Ice.Current current) => Thread.Sleep(ms);

    public override void
    shutdown(Ice.Current current)
    {
        lock (_mutex)
        {
            _shutdown = true;
            if (_pending != null)
            {
                _pending.SetResult(null);
                _pending = null;
            }
            current.adapter.getCommunicator().shutdown();
        }
    }

    public override bool
    supportsFunctionalTests(Ice.Current current) => false;

    public override bool
    supportsBackPressureTests(Ice.Current current) => true;

    public override Task opAsyncDispatchAsync(Ice.Current current) => Task.CompletedTask;

    public override async Task<int>
    opWithResultAsyncDispatchAsync(Ice.Current current)
    {
        test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server", StringComparison.Ordinal));
        await Task.Yield();
        test(!Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server", StringComparison.Ordinal));
        return await self(current).opWithResultAsync();
    }

    public override async Task
    opWithUEAsyncDispatchAsync(Ice.Current current)
    {
        test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server", StringComparison.Ordinal));
        await Task.Yield();
        test(!Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server", StringComparison.Ordinal));
        await self(current).opWithUEAsync();
    }

    public override async Task pingBiDirAsync(Test.PingReplyPrx reply, Ice.Current current)
    {
        reply = Test.PingReplyPrxHelper.uncheckedCast(reply.ice_fixed(current.con));
        bool expectSuccess = !current.ctx.ContainsKey("ONE");

        try
        {
            await reply.replyAsync();
            test(expectSuccess);
        }
        catch (ObjectNotExistException)
        {
            test(!expectSuccess);
        }
    }

    private Test.TestIntfPrx
    self(Ice.Current current) => Test.TestIntfPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));

    public override Task
    startDispatchAsync(Ice.Current current)
    {
        lock (_mutex)
        {
            if (_shutdown)
            {
                // Ignore, this can occur with the forceful connection close test, shutdown can be dispatch
                // before start dispatch.
                var v = new TaskCompletionSource<object>();
                v.SetResult(null);
                return v.Task;
            }
            else
            {
                _pending?.SetResult(null);
            }
            _pending = new TaskCompletionSource<object>();
            return _pending.Task;
        }
    }

    public override void
    finishDispatch(Ice.Current current)
    {
        lock (_mutex)
        {
            if (_shutdown)
            {
                return;
            }
            else if (_pending != null) // Pending might not be set yet if startDispatch is dispatch out-of-order
            {
                _pending.SetResult(null);
                _pending = null;
            }
        }
    }

    private int _batchCount;
    private bool _shutdown;
    private TaskCompletionSource<object> _pending;
    private readonly object _mutex = new();
}

public class TestII : Test.Outer.Inner.TestIntfDisp_
{
    public override int
    op(int i, out int j, Ice.Current current)
    {
        j = i;
        return i;
    }
}

public class TestControllerI : Test.TestIntfControllerDisp_
{
    public override void
    holdAdapter(Ice.Current current) => _adapter.hold();

    public override void
    resumeAdapter(Ice.Current current) => _adapter.activate();

    public
    TestControllerI(Ice.ObjectAdapter adapter) => _adapter = adapter;

    private readonly Ice.ObjectAdapter _adapter;
}
