// Copyright (c) ZeroC, Inc.


using System.Diagnostics;

namespace IceInternal;

public static class Time
{
    static Time()
    {
        _stopwatch.Start();
    }

    public static long currentMonotonicTimeMillis()
    {
        return _stopwatch.ElapsedMilliseconds;
    }

    private static readonly Stopwatch _stopwatch = new();
}
