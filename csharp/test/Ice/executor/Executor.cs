// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

public class Executor
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    public Executor()
    {
        Debug.Assert(_instance == null);
        _instance = this;
        _thread = new Thread(run);
        _thread.Start();
    }

    public void run()
    {
        while (true)
        {
            System.Action call = null;
            lock (_m)
            {
                if (!_terminated && _calls.Count == 0)
                {
                    Monitor.Wait(_m);
                }

                if (_calls.Count > 0)
                {
                    call = _calls.Dequeue();
                }
                else if (_terminated)
                {
                    // Terminate only once all calls are dispatched.
                    return;
                }
            }

            if (call != null)
            {
                try
                {
                    call();
                }
                catch (System.Exception)
                {
                    // Exceptions should never propagate here.
                    test(false);
                }
            }
        }
    }

    public void execute(System.Action call, Ice.Connection con)
    {
        lock (_m)
        {
            _calls.Enqueue(call);
            if (_calls.Count == 1)
            {
                Monitor.Pulse(_m);
            }
        }
    }

    public static void terminate()
    {
        lock (_m)
        {
            _instance._terminated = true;
            Monitor.Pulse(_m);
        }

        _instance._thread.Join();
    }

    public static bool isExecutorThread() => Thread.CurrentThread == _instance._thread;

    private static Executor _instance;

    private readonly Queue<System.Action> _calls = new Queue<System.Action>();
    private readonly Thread _thread;
    private bool _terminated = false;
    private static readonly object _m = new object();
}
