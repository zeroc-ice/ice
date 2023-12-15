//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Diagnostics;

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

        private static Stopwatch _stopwatch = new Stopwatch();
    }
}
