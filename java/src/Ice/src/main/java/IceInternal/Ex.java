// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class Ex
{
    public static void throwUOE(String expectedType, Ice.Object v)
    {
        //
        // If the object is an unknown sliced object, we didn't find an
        // object factory, in this case raise a NoObjectFactoryException
        // instead.
        //
        if(v instanceof Ice.UnknownSlicedObject)
        {
            Ice.UnknownSlicedObject uso = (Ice.UnknownSlicedObject)v;
            throw new Ice.NoObjectFactoryException("", uso.getUnknownTypeId());
        }

        String type = v.ice_id();
        throw new Ice.UnexpectedObjectException("expected element of type `" + expectedType + "' but received '" +
                                                type, type, expectedType);
    }

    public static void throwMemoryLimitException(int requested, int maximum)
    {
        throw new Ice.MemoryLimitException("requested " + requested + " bytes, maximum allowed is " + maximum +
                                           " bytes (see Ice.MessageSizeMax)");
    }

    //
    // A small utility to get the strack trace of the exception (which also includes toString()).
    //
    public static String toString(java.lang.Throwable ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        return sw.toString();
    }
}
