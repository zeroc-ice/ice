// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxDispatches;

internal sealed class ResponderI : Test.ResponderDisp_
{
    private readonly object _mutex = new();
    private readonly List<Action> _responses = new();
    private bool _started;

    public override void start(Current current)
    {
        List<Action> responses;

        lock (_mutex)
        {
            _started = true;
            responses = [.. _responses];
            _responses.Clear();
        }

        foreach (Action response in responses)
        {
            response();
        }
    }

    public override void stop(Current current)
    {
        lock (_mutex)
        {
            _started = false;
        }
    }

    public override int pendingResponseCount(Current current)
    {
        lock (_mutex)
        {
            return _responses.Count;
        }
    }

    internal void queueResponse(Action response)
    {
        bool queued = false;

        lock (_mutex)
        {
            if (!_started)
            {
                _responses.Add(response);
                queued = true;
            }
        }

        if (!queued)
        {
            response();
        }
    }
}
