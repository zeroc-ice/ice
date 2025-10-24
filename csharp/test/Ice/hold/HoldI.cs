// Copyright (c) ZeroC, Inc.

namespace Ice.hold;

public sealed class HoldI : Test.HoldDisp_
{
    private readonly Ice.ObjectAdapter _adapter;
    private int _last;
    private readonly object _mutex = new();
    private readonly object _taskMutex = new();

    private static void test(bool b) => global::Test.TestHelper.test(b);

    public HoldI(Ice.ObjectAdapter adapter) => _adapter = adapter;

    public override void putOnHold(int delay, Ice.Current current)
    {
        if (delay < 0)
        {
            _adapter.hold();
        }
        else if (delay == 0)
        {
            _adapter.hold();
            _adapter.activate();
        }
        else
        {
            _ = Task.Run(async () =>
            {
                await Task.Delay(delay);
                lock (_taskMutex) // serialize task execution
                {
                    try
                    {
                        _adapter.hold();
                        _adapter.activate();
                    }
                    catch
                    {
                        test(false);
                    }
                }
            });
        }
    }

    public override void waitForHold(Ice.Current current)
    {
        ObjectAdapter adapter = current.adapter;

        _ = Task.Run(() =>
        {
            lock (_taskMutex) // serialize task execution
            {
                try
                {
                    adapter.waitForHold();
                    adapter.activate();
                }
                catch
                {
                    test(false);
                }
            }
        });
    }

    public override int set(int value, int delay, Ice.Current current)
    {
        System.Threading.Thread.Sleep(delay);

        lock (_mutex)
        {
            int tmp = _last;
            _last = value;
            return tmp;
        }
    }

    public override void setOneway(int value, int expected, Ice.Current current)
    {
        lock (_mutex)
        {
            test(_last == expected);
            _last = value;
        }
    }

    public override void shutdown(Ice.Current current)
    {
        _adapter.hold();
        _adapter.getCommunicator().shutdown();
    }
}
