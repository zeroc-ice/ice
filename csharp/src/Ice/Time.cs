// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Diagnostics;

    public sealed class Time
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
