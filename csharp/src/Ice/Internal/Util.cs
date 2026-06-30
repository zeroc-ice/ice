// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class Util
{
    public static Instance getInstance(Ice.Communicator communicator) => communicator.instance;

    public static ThreadPriority getThreadPriorityProperty(Ice.Properties properties, string prefix)
    {
        string propertyName = prefix + ".ThreadPriority";
        string s = properties.getProperty(propertyName);
        if (string.IsNullOrEmpty(s))
        {
            return ThreadPriority.Normal;
        }
        string value = s.StartsWith("ThreadPriority.", StringComparison.Ordinal) ?
            s["ThreadPriority.".Length..] : s;
        return value switch
        {
            "Lowest" => ThreadPriority.Lowest,
            "BelowNormal" => ThreadPriority.BelowNormal,
            "Normal" => ThreadPriority.Normal,
            "AboveNormal" => ThreadPriority.AboveNormal,
            "Highest" => ThreadPriority.Highest,
            _ => throw new PropertyException($"property '{propertyName}' has an invalid value: '{s}'"),
        };
    }
}
