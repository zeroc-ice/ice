// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Runtime.InteropServices;

    public sealed class Time
    {
#if MANAGED
        public static long currentMonotonicTimeMillis()
        {
            return System.DateTime.Now.Ticks / 10000;
        }
#else
        [DllImport("Kernel32.dll")]
        private static extern bool QueryPerformanceCounter(out long lpPerformanceCount);

        [DllImport("Kernel32.dll")]
        private static extern bool QueryPerformanceFrequency(out long lpFrequency);

        public static long currentMonotonicTimeMillis()
        {
            if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
            {
                if(_frequency == -1)
                {
                    QueryPerformanceFrequency(out _frequency);
                }
                long current;
                QueryPerformanceCounter(out current);
                return (long)(1000.0 / _frequency * current);
            }
            else
            {
                return System.DateTime.Now.Ticks / 10000;
            }
        }

        private static long _frequency = -1;
#endif
    }
}
