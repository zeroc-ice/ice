// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class Ex
{
    public static <T extends com.zeroc.Ice.Value> void throwUOE(Class<T> expectedType, com.zeroc.Ice.Value v)
    {
        //
        // If the object is an unknown sliced object, we didn't find an
        // value factory, in this case raise a NoValueFactoryException
        // instead.
        //
        if(v instanceof com.zeroc.Ice.UnknownSlicedValue)
        {
            com.zeroc.Ice.UnknownSlicedValue usv = (com.zeroc.Ice.UnknownSlicedValue)v;
            throw new com.zeroc.Ice.NoValueFactoryException("", usv.ice_id());
        }

        String type = v.ice_id();
        String expected;
        try
        {
            expected = (String)expectedType.getMethod("ice_staticId").invoke(null);
        }
        catch(Exception ex)
        {
            expected = "";
            assert(false);
        }
        throw new com.zeroc.Ice.UnexpectedObjectException(
            "expected element of type `" + expected + "' but received '" + type, type, expected);
    }
    public static void throwUOE(String expectedType, com.zeroc.Ice.Value v)
    {
        //
        // If the object is an unknown sliced object, we didn't find an
        // value factory, in this case raise a NoValueFactoryException
        // instead.
        //
        if(v instanceof com.zeroc.Ice.UnknownSlicedValue)
        {
            com.zeroc.Ice.UnknownSlicedValue usv = (com.zeroc.Ice.UnknownSlicedValue)v;
            throw new com.zeroc.Ice.NoValueFactoryException("", usv.ice_id());
        }

        String type = v.ice_id();
        throw new com.zeroc.Ice.UnexpectedObjectException(
            "expected element of type `" + expectedType + "' but received '" + type, type, expectedType);
    }

    public static void throwMemoryLimitException(int requested, int maximum)
    {
        throw new com.zeroc.Ice.MemoryLimitException(
            "requested " + requested + " bytes, maximum allowed is " + maximum + " bytes (see Ice.MessageSizeMax)");
    }

    //
    // A small utility to get the stack trace of the exception (which also includes toString()).
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
