// Copyright (c) ZeroC, Inc.

using System.Collections;
using System.Globalization;
using System.Net.Security;

namespace IceInternal;

public sealed class HashUtil
{
    public static void hashAdd(ref int hashCode, bool value)
    {
        hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
    }

    public static void hashAdd(ref int hashCode, short value)
    {
        hashCode = unchecked(((hashCode << 5) + hashCode) ^ (int)(2654435761 * value));
    }

    public static void hashAdd(ref int hashCode, byte value)
    {
        hashCode = unchecked(((hashCode << 5) + hashCode) ^ (int)(2654435761 * value));
    }

    public static void hashAdd(ref int hashCode, int value)
    {
        hashCode = unchecked(((hashCode << 5) + hashCode) ^ (int)(2654435761 * value));
    }

    public static void hashAdd(ref int hashCode, long value)
    {
        hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
    }

    public static void hashAdd(ref int hashCode, float value)
    {
        hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
    }

    public static void hashAdd(ref int hashCode, double value)
    {
        hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
    }

    public static void hashAdd(ref int hashCode, object value)
    {
        if (value != null)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ value.GetHashCode());
        }
    }

    public static void hashAdd(ref int hashCode, object[] arr)
    {
        if (arr != null)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Arrays.GetHashCode(arr));
        }
    }

    public static void hashAdd(ref int hashCode, Array arr)
    {
        if (arr != null)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Arrays.GetHashCode(arr));
        }
    }

    public static void hashAdd(ref int hashCode, IEnumerable s)
    {
        if (s != null)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Collections.SequenceGetHashCode(s));
        }
    }

    public static void hashAdd(ref int hashCode, IDictionary d)
    {
        if (d != null)
        {
            hashCode = unchecked(((hashCode << 5) + hashCode) ^ IceUtilInternal.Collections.DictionaryGetHashCode(d));
        }
    }
}

public sealed class Util
{
    public static Instance getInstance(Ice.Communicator communicator)
    {
        Ice.CommunicatorI p = (Ice.CommunicatorI)communicator;
        return p.getInstance();
    }

    public static ProtocolPluginFacade getProtocolPluginFacade(Ice.Communicator communicator)
    {
        return new ProtocolPluginFacadeI(communicator);
    }

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
