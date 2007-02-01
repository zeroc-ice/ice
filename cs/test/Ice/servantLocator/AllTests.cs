// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;
using Ice;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }
    
    public static void testExceptions(TestIntfPrx obj, bool collocated)
    {
        try
        {
            obj.requestFailedException();
            test(false);
        }
        catch(ObjectNotExistException ex)
        {
            if(!collocated)
            {
                test(ex.id.Equals(obj.ice_getIdentity()));
                test(ex.facet.Equals(obj.ice_getFacet()));
                test(ex.operation.Equals("requestFailedException"));
            }
        }

        try
        {
            obj.unknownUserException();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            test(ex.unknown.Equals("reason"));
        }

        try
        {
            obj.unknownLocalException();
            test(false);
        }
        catch(UnknownLocalException ex)
        {
            test(ex.unknown.Equals("reason"));
        }

        try
        {
            obj.unknownException();
            test(false);
        }
        catch(UnknownException ex)
        {
            test(ex.unknown.Equals("reason"));
        }

        try
        {
            obj.userException();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            //Console.Error.WriteLine(ex.unknown);
            test(!collocated);
            test(ex.unknown.IndexOf("Test.TestIntfUserException") >= 0);
        }
        catch(TestIntfUserException)
        {
            test(collocated);
        }

        try
        {
            obj.localException();
            test(false);
        }
        catch(UnknownLocalException ex)
        {
            //Console.Error.WriteLine(ex.unknown);
            test(!collocated);
            test(ex.unknown.IndexOf("Ice.SocketException") >= 0);
        }
        catch(SocketException)
        {
            test(collocated);
        }

        try
        {
            obj.csException();
            test(false);
        }
        catch(UnknownException ex)
        {
            //Console.Error.WriteLine(ex.unknown);
            test(!collocated);
            test(ex.unknown.IndexOf("System.Exception: message") >= 0);
        }
        catch(System.Exception)
        {
            test(collocated);
        }
    }

    public static TestIntfPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string @ref = "asm:default -p 12010 -t 2000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing servant locator...");
        Console.Out.Flush();
        @base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(@base);
        try
        {
            TestIntfPrxHelper.checkedCast(communicator.stringToProxy("category/unknown:default -p 12010 -t 10000"));
        }
        catch(ObjectNotExistException)
        {
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing default servant locator...");
        Console.Out.Flush();
        @base = communicator.stringToProxy("anothercat/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(@base);
        @base = communicator.stringToProxy("locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(@base);
        try
        {
            TestIntfPrxHelper.checkedCast(communicator.stringToProxy("anothercat/unknown:default -p 12010 -t 10000"));
        }
        catch(ObjectNotExistException)
        {
        }
        try
        {
            TestIntfPrxHelper.checkedCast(communicator.stringToProxy("unknown:default -p 12010 -t 10000"));
        }
        catch(ObjectNotExistException)
        {
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing locate exceptions... ");
        Console.Out.Flush();
        @base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(@base);
        testExceptions(obj, collocated);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing finished exceptions... ");
        Console.Out.Flush();
        @base = communicator.stringToProxy("category/finished:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(@base);
        testExceptions(obj, collocated);
        Console.Out.WriteLine("ok");

        return obj;
    }
}
