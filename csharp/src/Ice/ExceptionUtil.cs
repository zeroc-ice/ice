// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Runtime.ExceptionServices;

namespace ZeroC.Ice
{
    internal static class ExceptionUtil
    {
        /// <summary>Helper function to re throw an exception preserving the stack trace, use as:
        /// <code>throw ExceptionUtil.Throw(ex.AggregateException);</code>
        /// This prevent the compiler error of not all code path returning a value.</summary>
        internal static Exception Throw(Exception ex)
        {
            ExceptionDispatchInfo.Throw(ex);
            Debug.Assert(false);
            return ex;
        }
    }
}
