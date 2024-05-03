// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;

namespace Ice.Internal;

public static class Ex
{
    public static void throwUOE(Type expectedType, Ice.Value v)
    {
        //
        // If the object is an unknown sliced object, we didn't find an
        // value factory, in this case raise a NoValueFactoryException
        // instead.
        //
        if (v is Ice.UnknownSlicedValue)
        {
            Ice.UnknownSlicedValue usv = (Ice.UnknownSlicedValue)v;
            throw new Ice.NoValueFactoryException("", usv.ice_id());
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

        throw new Ice.UnexpectedObjectException("expected element of type `" + expected + "' but received `" +
                                                type + "'", type, expected);
    }

    public static void throwMemoryLimitException(int requested, int maximum)
    {
        throw new Ice.MemoryLimitException("requested " + requested + " bytes, maximum allowed is " + maximum +
                                           " bytes (see Ice.MessageSizeMax)");
    }
}

public class RetryException : System.Exception
{
    public RetryException(Ice.LocalException ex)
    {
        _ex = ex;
    }

    public Ice.LocalException get()
    {
        return _ex;
    }

    private Ice.LocalException _ex;
}
