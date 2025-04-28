// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

public static class Ex
{
    public static void throwUOE(Type expectedType, Ice.Value v)
    {
        if (v is UnknownSlicedValue usv)
        {
            throw new MarshalException($"The Slice loader did not find a class for type ID '{usv.ice_id()}'.");
        }

        string type = v.ice_id();
        string expected;
        try
        {
            expected = (string)expectedType.GetMethod("ice_staticId").Invoke(null, null);
        }
        catch (System.Exception)
        {
            expected = "";
            Debug.Assert(false);
        }

        throw new MarshalException(
            $"Failed to unmarshal class with type ID '{expected}': the Slice loader returned class with type ID '{type}'.");
    }

    public static void throwMemoryLimitException(int requested, int maximum)
    {
        throw new MarshalException(
            $"Cannot unmarshal Ice message: the message size of {requested} bytes exceeds the maximum allowed of {maximum} bytes (see Ice.MessageSizeMax).");
    }
}

public class RetryException : System.Exception
{
    private readonly Ice.LocalException _ex;

    public RetryException(Ice.LocalException ex) => _ex = ex;

    public Ice.LocalException get() => _ex;
}
