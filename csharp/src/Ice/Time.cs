//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System;
using System.Diagnostics;

namespace ZeroC.Ice
{
    public sealed class Time
    {
        static Time() => _stopwatch.Start();

        public static long CurrentMonotonicTimeMillis() => _stopwatch.ElapsedMilliseconds;

        public static TimeSpan CurrentMonotonicTime() => _stopwatch.Elapsed;

        private static readonly Stopwatch _stopwatch = new Stopwatch();
    }
}
