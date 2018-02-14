// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
#if !SILVERLIGHT
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

#else

    public class Stopwatch
    {
        public void Start()
        {
            if(!_running)
            {
                _startTick = System.DateTime.Now.Ticks;
                _running = true;
            }
        }

        public void Stop()
        {
            if(_running)
            {
                _elapsedTicks += System.DateTime.Now.Ticks - _startTick;
                _running = false;
            }

        }

        public void Reset()
        {
            _startTick = 0;
            _elapsedTicks = 0;
            _running = false;
        }

        public long ElapsedTicks
        {
            get
            {
                if(!_running)
                {
                    return _elapsedTicks;
                }
                else
                {
                    return _elapsedTicks + (System.DateTime.Now.Ticks - _startTick);
                }
            }
        }

        public long Frequency
        {
            get
            {
                return System.TimeSpan.TicksPerMillisecond * 1000;
            }
        }

        private long _startTick = 0;
        private long _elapsedTicks = 0;
        private bool _running = false;
    }

    public sealed class Time
    {
        static Time()
        {
            _begin = System.DateTime.Now.Ticks;
        }

        public static long currentMonotonicTimeMillis()
        {
            return (System.DateTime.Now.Ticks - _begin) / 10000;
        }

        private static long _begin;
    }
#endif
}
