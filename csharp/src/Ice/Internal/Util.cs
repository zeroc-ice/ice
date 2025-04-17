// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class Util
{
    public static Instance getInstance(Ice.Communicator communicator) => communicator.instance;

    public static ThreadPriority stringToThreadPriority(string s)
    {
        if (string.IsNullOrEmpty(s))
        {
            return ThreadPriority.Normal;
        }
        if (s.StartsWith("ThreadPriority.", StringComparison.Ordinal))
        {
            s = s.Substring("ThreadPriority.".Length, s.Length);
        }
        if (s == "Lowest")
        {
            return ThreadPriority.Lowest;
        }
        else if (s == "BelowNormal")
        {
            return ThreadPriority.BelowNormal;
        }
        else if (s == "Normal")
        {
            return ThreadPriority.Normal;
        }
        else if (s == "AboveNormal")
        {
            return ThreadPriority.AboveNormal;
        }
        else if (s == "Highest")
        {
            return ThreadPriority.Highest;
        }
        return ThreadPriority.Normal;
    }
}
