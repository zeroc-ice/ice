// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Globalization;
using System.Threading;

namespace ZeroC.Ice
{
    /// <summary>Provides public extension methods for <see cref="TimeSpan"/> values.</summary>
    public static class TimeSpanExtensions
    {
        /// <summary>Gets the TimeSpan as an Ice property string. The largest possible unit
        /// which fully represents the TimeSpan will be used. e.g. A TimeSpan of 00:01:30 will be
        /// returned as "75s" </summary>
        /// <param name="ts">The TimeSpan.</param>
        /// <returns>The stringified TimeSpan.</returns>
        public static string ToPropertyString(this TimeSpan ts)
        {
            FormattableString message;
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
                message = $"{ts.TotalMilliseconds}ms";
            }
            else if (ts.Seconds != 0)
            {
                message = $"{ts.TotalSeconds}s";
            }
            else if (ts.Minutes != 0)
            {
                message = $"{ts.TotalMinutes}m";
            }
            else if (ts.Hours != 0)
            {
                message = $"{ts.TotalHours}h";
            }
            else if (ts.Days != 0)
            {
                message = $"{ts.TotalDays}d";
            }
            else
            {
                message = $"{ts.TotalMilliseconds}ms";
            }

            return message.ToString(CultureInfo.InvariantCulture);
        }
    }
}
