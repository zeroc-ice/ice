// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.servantLocator;

import java.io.PrintWriter;

import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntfPrx;
import test.Ice.servantLocator.Test.TestIntfPrxHelper;
import Ice.ObjectNotExistException;
import Ice.ObjectPrx;
import Ice.UnknownException;
import Ice.UnknownLocalException;
import Ice.UnknownUserException;

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
            test(ex.unknown.indexOf("Ice::SocketException") >= 0);
        }
        catch(Throwable ex)
        {
            test(false);
        }

        try
        {
            obj.javaException();
            test(false);
        }
        catch(UnknownException ex)
        {
            test(ex.unknown.indexOf("java.lang.RuntimeException: message") >= 0);
        }
        catch(Throwable ex)
        {
            test(false);
        }

        try
        {
            obj.impossibleException(false);
            test(false);
        }
        catch(UnknownUserException ex)
        {
            // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }

        try
        {
            obj.impossibleException(true);
            test(false);
        }
        catch(UnknownUserException ex)
        {
            // Operation throws TestImpossibleException, but locate() and finished() throw TestIntfUserException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }

        try
        {
            obj.intfUserException(false);
            test(false);
        }
        catch(TestImpossibleException ex)
        {
            // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }

        try
        {
            obj.intfUserException(true);
            test(false);
        }
        catch(TestImpossibleException ex)
        {
            // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }
    }

    public static TestIntfPrx
    allTests(Ice.Communicator communicator, boolean collocated, PrintWriter out)
    {
		out.print("testing stringToProxy... ");
        out.flush();
        String ref = "asm:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        out.print("testing ice_ids... ");
        out.flush();
        try
        {
            ObjectPrx o = communicator.stringToProxy("category/locate:default -p 12010 -t 10000");
            o.ice_ids();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            test(ex.unknown.equals("Test::TestIntfUserException"));
        }
        catch(Throwable ex)
        {
            test(false);
        }

        try
        {
            ObjectPrx o = communicator.stringToProxy("category/finished:default -p 12010 -t 10000");
            o.ice_ids();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            test(ex.unknown.equals("Test::TestIntfUserException"));
        }
        catch(Throwable ex)
        {
            test(false);
        }
        out.println("ok");

        out.print("testing servant locator... ");
        out.flush();
        base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        try
        {
            TestIntfPrxHelper.checkedCast(communicator.stringToProxy("category/unknown:default -p 12010 -t 10000"));
        }
        catch(ObjectNotExistException ex)
        {
        }
        out.println("ok");

        out.print("testing default servant locator... ");
        out.flush();
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
        out.println("ok");

        out.print("testing locate exceptions... ");
        out.flush();
        base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        testExceptions(obj, collocated);
        out.println("ok");

        out.print("testing finished exceptions... ");
        out.flush();
        base = communicator.stringToProxy("category/finished:default -p 12010 -t 10000");
        obj = TestIntfPrxHelper.checkedCast(base);
        testExceptions(obj, collocated);
        out.println("ok");

        return obj;
    }
}
