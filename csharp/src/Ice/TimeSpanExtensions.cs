//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;

namespace ZeroC.Ice
{
    public static class TimeSpanExtensions
    {
        /// <summary>Gets the TimeSpan as an Ice property string. The largest possible unit
        /// which fully represents the TimeSpan will be used. e.g. A TimeSpan of 00:01:30 will be
        /// returned as "75s" </summary>
        /// <param name="ts">The TimeSpan.</param>
        /// <returns>The stringified TimeSpan.</returns>
        public static string ToPropertyString(this TimeSpan ts)
        {
            if (ts == TimeSpan.Zero)
            {
                return "0ms";
            }
            else if (ts == Timeout.InfiniteTimeSpan)
            {
                return "infinite";
            }
            else if (ts.Milliseconds != 0)
            {
                return $"{ts.TotalMilliseconds}ms";
            }
            else if (ts.Seconds != 0)
            {
                return $"{ts.TotalSeconds}s";
            }
            else if (ts.Minutes != 0)
            {
                return $"{ts.TotalMinutes}m";
            }
            else if (ts.Hours != 0)
            {
                return $"{ts.TotalHours}h";
            }
            else if (ts.Days != 0)
            {
                return $"{ts.TotalDays}d";
            }
            else
            {
                return $"{ts.TotalMilliseconds}ms";
            }
        }
    }
}
