// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;
import Ice.*;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    testExceptions(TestIntfPrx obj, boolean collocated)
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
                test(ex.id.equals(obj.ice_getIdentity()));
                test(ex.facet.equals(obj.ice_getFacet()));
                test(ex.operation.equals("requestFailedException"));
            }
        }

        try
        {
            obj.unknownUserException();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            test(ex.unknown.equals("reason"));
        }

        try
        {
            obj.unknownLocalException();
            test(false);
        }
        catch(UnknownLocalException ex)
        {
            test(ex.unknown.equals("reason"));
        }

        try
        {
            obj.unknownException();
            test(false);
        }
        catch(UnknownException ex)
        {
            test(ex.unknown.equals("reason"));
        }

        //
        // User exceptions are checked exceptions
        //
//      try
//      {
//          obj.userException();
//          test(false);
//      }
//      catch(UnknownUserException ex)
//      {
//          //System.err.println(ex.unknown);
//          test(!collocated);
//          test(ex.unknown.equals("Test::TestIntfUserException"));
//      }
//      catch(TestIntfUserException ex)
//      {
//          test(collocated);
//      }

        try
        {
            obj.localException();
            test(false);
        }
        catch(UnknownLocalException ex)
        {
            //System.err.println(ex.unknown);
            test(!collocated);
            test(ex.unknown.indexOf("Ice.SocketException") >= 0);
        }
        catch(SocketException ex)
        {
            test(collocated);
        }

        try
        {
            obj.javaException();
            test(false);
        }
        catch(UnknownException ex)
        {
            //System.err.println(ex.unknown);
            test(!collocated);
            test(ex.unknown.indexOf("java.lang.RuntimeException: message") >= 0);
        }
        catch(java.lang.RuntimeException ex)
        {
            test(collocated);
        }
    }

    public static TestIntfPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "asm:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        System.out.println("ok");

        System.out.print("testing servant locator... ");
        System.out.flush();
        base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        try
        {
            TestIntfPrxHelper.checkedCast(communicator.stringToProxy("category/unknown:default -p 12010 -t 10000"));
        }
        catch(ObjectNotExistException ex)
        {
        }
        System.out.println("ok");

        System.out.print("testing default servant locator... ");
        System.out.flush();
        base = communicator.stringToProxy("anothercat/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        base = communicator.stringToProxy("locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        try
        {
            TestIntfPrxHelper.checkedCast(communicator.stringToProxy("anothercat/unknown:default -p 12010 -t 10000"));
        }
        catch(ObjectNotExistException ex)
        {
        }
        try
        {
            TestIntfPrxHelper.checkedCast(communicator.stringToProxy("unknown:default -p 12010 -t 10000"));
        }
        catch(ObjectNotExistException ex)
        {
        }
        System.out.println("ok");

        System.out.print("testing locate exceptions... ");
        System.out.flush();
        base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        testExceptions(obj, collocated);
        System.out.println("ok");

        System.out.print("testing finished exceptions... ");
        System.out.flush();
        base = communicator.stringToProxy("category/finished:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        testExceptions(obj, collocated);
        System.out.println("ok");

        return obj;
    }
}
