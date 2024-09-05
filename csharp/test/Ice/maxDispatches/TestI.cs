// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxDispatches;

internal sealed class TestIntfI : Test.TestIntfDisp_
{
    private readonly object _mutex = new();
    private readonly ResponderI _responder;
    private int _dispatchCount;
    private int _maxDispatchCount;

    public override Task opAsync(Current current)
    {
        lock (_mutex)
        {
            if (++_dispatchCount > _maxDispatchCount)
            {
                _maxDispatchCount = _dispatchCount;
            }
        }

        var tcs = new TaskCompletionSource(); // continuation can run synchronously

        _responder.queueResponse(() =>
        {
            decDispatchCount();
            tcs.SetResult();
        });

        return tcs.Task;
    }

    public override int resetMaxConcurrentDispatches(Current current)
    {
        lock (_mutex)
        {
            int result = _maxDispatchCount;
            _maxDispatchCount = 0;
            return result;
        }
    }

    public override void shutdown(Current current) => current.adapter.getCommunicator().shutdown();

    internal TestIntfI(ResponderI responder) => _responder = responder;

    private void decDispatchCount()
    {
        lock (_mutex)
        {
            --_dispatchCount;
        }
    }
}
