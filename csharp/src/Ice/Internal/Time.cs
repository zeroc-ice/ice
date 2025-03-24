// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

public static class Time
{
    static Time() => _stopwatch.Start();

    public static long currentMonotonicTimeMillis() => _stopwatch.ElapsedMilliseconds;

    private static readonly Stopwatch _stopwatch = new();
}
