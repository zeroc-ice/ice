// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxDispatches;

internal sealed class TestIntfI : Test.TestIntfDisp_
{
    private readonly object _mutex = new();
    private int _dispatchCount;
    private int _maxDispatchCount;

    public override async Task opAsync(Current current)
    {
        lock (_mutex)
        {
            if (++_dispatchCount > _maxDispatchCount)
            {
                _maxDispatchCount = _dispatchCount;
            }
        }
        await Task.Delay(TimeSpan.FromMilliseconds(50));
        lock (_mutex)
        {
            --_dispatchCount;
        }
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
}
