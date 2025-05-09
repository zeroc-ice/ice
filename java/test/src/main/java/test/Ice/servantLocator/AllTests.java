// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationNotExistException;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UnknownLocalException;
import com.zeroc.Ice.UnknownUserException;

import test.Ice.servantLocator.Test.TestActivationPrx;
import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntfPrx;
import test.Ice.servantLocator.Test.TestIntfUserException;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void testExceptions(TestIntfPrx obj, TestHelper helper) {
        try {
            obj.requestFailedException();
            test(false);
        } catch (ObjectNotExistException ex) {
            test(ex.id.equals(obj.ice_getIdentity()));
            test(ex.facet.equals(obj.ice_getFacet()));
            test("requestFailedException".equals(ex.operation));
        }

        try {
            obj.unknownUserException();
            test(false);
        } catch (UnknownUserException ex) {
            helper.getWriter().flush();
            test("reason".equals(ex.getMessage()));
        }

        try {
            obj.unknownLocalException();
            test(false);
        } catch (UnknownLocalException ex) {
            test("reason".equals(ex.getMessage()));
        }

        try {
            obj.unknownException();
            test(false);
        } catch (UnknownException ex) {
            test("reason".equals(ex.getMessage()));
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
        //          // // TODO remove this print after figuring out why this block is commented out.
        //          //System.err.println(ex.getMessage());
        //          test(!collocated);
        //          test(ex.getMessage().equals("Test::TestIntfUserException"));
        //      }
        //      catch(TestIntfUserException ex)
        //      {
        //          test(collocated);
        //      }

        try {
            obj.localException();
            test(false);
        } catch (UnknownLocalException ex) {
            var message = ex.getMessage();
            test(
                message.contains("Ice::SocketException")
                    || message.contains("Ice.SocketException"));
        } catch (Throwable ex) {
            test(false);
        }

        try {
            obj.javaException();
            test(false);
        } catch (UnknownException ex) {
            test(ex.getMessage().contains("java.lang.RuntimeException: message"));
        } catch (OperationNotExistException ex) {} catch (Throwable ex) {
            // System.err.println(ex);
            test(false);
        }

        try {
            obj.unknownExceptionWithServantException();
            test(false);
        } catch (UnknownException ex) {
            test("reason".equals(ex.getMessage()));
        } catch (Throwable ex) {
            test(false);
        }

        try {
            obj.impossibleException(false);
            test(false);
        } catch (UnknownUserException ex) {
            // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
        } catch (Throwable ex) {
            // System.err.println(ex);
            test(false);
        }

        try {
            obj.impossibleException(true);
            test(false);
        } catch (UnknownUserException ex) {
            // Operation throws TestImpossibleException, but locate() and finished() throw
            // TestIntfUserException.
        } catch (Throwable ex) {
            // System.err.println(ex);
            test(false);
        }

        try {
            obj.intfUserException(false);
            test(false);
        } catch (TestImpossibleException ex) {
            // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
        } catch (Throwable ex) {
            // System.err.println(ex);
            test(false);
        }

        try {
            obj.intfUserException(true);
            test(false);
        } catch (TestImpossibleException ex) {
            // Operation throws TestIntfUserException, but locate() and finished() throw
            // TestImpossibleException.
        } catch (Throwable ex) {
            // System.err.println(ex);
            test(false);
        }
    }

    public static TestIntfPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "asm:" + helper.getTestEndpoint(0);
        ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrx.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        out.print("testing ice_ids... ");
        out.flush();
        try {
            ObjectPrx o =
                communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
            o.ice_ids();
            test(false);
        } catch (UnknownUserException ex) {
            var expected =
                "The reply carries a user exception that does not conform to the operation's exception specification: "
                    + "::Test::TestIntfUserException";
            test(ex.getMessage().equals(expected));
        } catch (Throwable ex) {
            test(false);
        }

        try {
            ObjectPrx o =
                communicator.stringToProxy("category/finished:" + helper.getTestEndpoint(0));
            o.ice_ids();
            test(false);
        } catch (UnknownUserException ex) {
            var expected =
                "The reply carries a user exception that does not conform to the operation's exception specification: "
                    + "::Test::TestIntfUserException";
            test(ex.getMessage().equals(expected));
        } catch (Throwable ex) {
            test(false);
        }
        out.println("ok");

        out.print("testing servant locator... ");
        out.flush();
        base = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        try {
            TestIntfPrx.checkedCast(
                communicator.stringToProxy("category/unknown:" + helper.getTestEndpoint(0)));
        } catch (ObjectNotExistException ex) {}
        out.println("ok");

        out.print("testing default servant locator... ");
        out.flush();
        base = communicator.stringToProxy("anothercat/locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        base = communicator.stringToProxy("locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        try {
            TestIntfPrx.checkedCast(
                communicator.stringToProxy("anothercat/unknown:" + helper.getTestEndpoint(0)));
        } catch (ObjectNotExistException ex) {}
        try {
            TestIntfPrx.checkedCast(
                communicator.stringToProxy("unknown:" + helper.getTestEndpoint(0)));
        } catch (ObjectNotExistException ex) {}
        out.println("ok");

        out.print("testing locate exceptions... ");
        out.flush();
        base = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        testExceptions(obj, helper);
        out.println("ok");

        out.print("testing finished exceptions... ");
        out.flush();
        base = communicator.stringToProxy("category/finished:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        testExceptions(obj, helper);

        //
        // Only call these for category/finished.
        //
        try {
            obj.asyncResponse();
        } catch (TestIntfUserException ex) {
            test(false);
        } catch (TestImpossibleException ex) {
            //
            // Called by finished().
            //
        }

        //
        // Only call these for category/finished.
        //
        try {
            obj.asyncException();
        } catch (TestIntfUserException ex) {
            test(false);
        } catch (TestImpossibleException ex) {
            //
            // Called by finished().
            //
        }

        out.println("ok");

        out.print("testing servant locator removal... ");
        out.flush();
        base = communicator.stringToProxy("test/activation:" + helper.getTestEndpoint(0));
        TestActivationPrx activation = TestActivationPrx.checkedCast(base);
        activation.activateServantLocator(false);
        try {
            obj.ice_ping();
            test(false);
        } catch (ObjectNotExistException ex) {
            out.println("ok");
        }
        out.print("testing servant locator addition... ");
        out.flush();
        activation.activateServantLocator(true);
        try {
            obj.ice_ping();
            out.println("ok");
        } catch (Exception ex) {
            test(false);
        }

        return obj;
    }

    private AllTests() {}
}
