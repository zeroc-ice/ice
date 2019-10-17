//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interceptor;

import java.io.PrintWriter;

import test.Ice.interceptor.Test.InvalidInputException;
import test.Ice.interceptor.Test.MyObjectPrx;

public class Client extends test.TestHelper
{
    private void runTest(MyObjectPrx prx, InterceptorI interceptor)
    {
        PrintWriter out = getWriter();
        out.print("testing simple interceptor... ");
        out.flush();
        test(interceptor.getLastOperation() == null);
        test(!interceptor.getLastStatus());
        prx.ice_ping();
        test(interceptor.getLastOperation().equals("ice_ping"));
        test(!interceptor.getLastStatus());
        String typeId = prx.ice_id();
        test(interceptor.getLastOperation().equals("ice_id"));
        test(!interceptor.getLastStatus());
        test(prx.ice_isA(typeId));
        test(interceptor.getLastOperation().equals("ice_isA"));
        test(!interceptor.getLastStatus());
        test(prx.add(33, 12) == 45);
        test(interceptor.getLastOperation().equals("add"));
        test(!interceptor.getLastStatus());
        out.println("ok");
        out.print("testing retry... ");
        out.flush();
        test(prx.addWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("addWithRetry"));
        test(!interceptor.getLastStatus());
        out.println("ok");
        out.print("testing user exception... ");
        out.flush();
        try
        {
            prx.badAdd(33, 12);
            test(false);
        }
        catch(InvalidInputException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("badAdd"));
        test(!interceptor.getLastStatus());
        out.println("ok");
        out.print("testing ONE... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.notExistAdd(33, 12);
            test(false);
        }
        catch(com.zeroc.Ice.ObjectNotExistException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("notExistAdd"));
        test(!interceptor.getLastStatus());
        out.println("ok");
        out.print("testing system exception... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.badSystemAdd(33, 12);
            test(false);
        }
        catch(com.zeroc.Ice.UnknownException e)
        {
            test(!prx.ice_isCollocationOptimized());
        }
        catch(MySystemException e)
        {
            test(prx.ice_isCollocationOptimized());
        }
        catch(Throwable ex)
        {
            test(false);
        }
        test(interceptor.getLastOperation().equals("badSystemAdd"));
        test(!interceptor.getLastStatus());
        out.println("ok");

        out.print("testing exceptions raised by the interceptor... ");
        out.flush();
        testInterceptorExceptions(prx);
        out.println("ok");
    }

    private void runAmdTest(MyObjectPrx prx, InterceptorI interceptor, PrintWriter out)
    {
        out.print("testing simple interceptor... ");
        out.flush();
        test(interceptor.getLastOperation() == null);
        test(!interceptor.getLastStatus());
        test(prx.amdAdd(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAdd"));
        test(interceptor.getLastStatus());
        out.println("ok");
        out.print("testing retry... ");
        out.flush();
        test(prx.amdAddWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAddWithRetry"));
        test(interceptor.getLastStatus());
        {
            java.util.Map<String, String> ctx = new java.util.HashMap<>();
            ctx.put("retry", "yes");
            for(int i = 0; i < 10; ++i)
            {
                test(prx.amdAdd(33, 12, ctx) == 45);
                test(interceptor.getLastOperation().equals("amdAdd"));
                test(interceptor.getLastStatus());
            }
        }
        out.println("ok");
        out.print("testing user exception... ");
        out.flush();
        try
        {
            prx.amdBadAdd(33, 12);
            test(false);
        }
        catch(InvalidInputException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("amdBadAdd"));
        test(interceptor.getLastStatus());
        out.println("ok");
        out.print("testing ONE... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.amdNotExistAdd(33, 12);
            test(false);
        }
        catch(com.zeroc.Ice.ObjectNotExistException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("amdNotExistAdd"));
        test(interceptor.getLastStatus());
        out.println("ok");
        out.print("testing system exception... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.amdBadSystemAdd(33, 12);
            test(false);
        }
        catch(com.zeroc.Ice.UnknownException e)
        {
            test(!prx.ice_isCollocationOptimized());
        }
        catch(MySystemException e)
        {
            test(prx.ice_isCollocationOptimized());
        }
        catch(Throwable ex)
        {
            test(false);
        }
        test(interceptor.getLastOperation().equals("amdBadSystemAdd"));
        test(interceptor.getLastStatus());
        out.println("ok");

        out.print("testing exceptions raised by the interceptor... ");
        out.flush();
        testInterceptorExceptions(prx);
        out.println("ok");
    }

    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.interceptor");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            //
            // Create OA and servants
            //
            communicator.getProperties().setProperty("MyOA.AdapterId", "myOA");

            com.zeroc.Ice.ObjectAdapter oa = communicator.createObjectAdapterWithEndpoints("MyOA2", "tcp -h localhost");

            com.zeroc.Ice.Object servant = new MyObjectI();
            InterceptorI interceptor = new InterceptorI(servant);

            MyObjectPrx prx = MyObjectPrx.uncheckedCast(oa.addWithUUID(interceptor));

            PrintWriter out = getWriter();
            out.println("Collocation optimization on");
            runTest(prx, interceptor);
            out.println("Now with AMD");
            interceptor.clear();
            runAmdTest(prx, interceptor, out);
            oa.activate(); // Only necessary for non-collocation optimized tests

            out.println("Collocation optimization off");
            interceptor.clear();
            prx = MyObjectPrx.uncheckedCast(prx.ice_collocationOptimized(false));
            runTest(prx, interceptor);
            out.println("Now with AMD");
            interceptor.clear();
            runAmdTest(prx, interceptor, out);
        }
    }

    private class ExceptionPoint
    {
        public ExceptionPoint(String point, String exception)
        {
            this.point = point;
            this.exception = exception;
        }
        public String point;
        public String exception;
    };

    private void testInterceptorExceptions(MyObjectPrx prx)
    {
        java.util.List<ExceptionPoint> exceptions = new java.util.ArrayList<>();
        exceptions.add(new ExceptionPoint("raiseBeforeDispatch", "user"));
        exceptions.add(new ExceptionPoint("raiseBeforeDispatch", "notExist"));
        exceptions.add(new ExceptionPoint("raiseBeforeDispatch", "system"));
        exceptions.add(new ExceptionPoint("raiseAfterDispatch", "user"));
        exceptions.add(new ExceptionPoint("raiseAfterDispatch", "notExist"));
        exceptions.add(new ExceptionPoint("raiseAfterDispatch", "system"));
        for(ExceptionPoint e : exceptions)
        {
            java.util.Map<String, String> ctx = new java.util.HashMap<>();
            ctx.put(e.point, e.exception);
            try
            {
                prx.ice_ping(ctx);
                test(false);
            }
            catch(com.zeroc.Ice.UnknownUserException ex)
            {
                test(e.exception.equals("user"));
            }
            catch(com.zeroc.Ice.ObjectNotExistException ex)
            {
                test(e.exception.equals("notExist"));
            }
            catch(com.zeroc.Ice.UnknownException ex)
            {
                test(e.exception.equals("system")); // non-collocated
            }
            catch(MySystemException ex)
            {
                test(e.exception.equals("system")); // collocated
            }
            {
                com.zeroc.Ice.ObjectPrx batch = prx.ice_batchOneway();
                batch.ice_ping(ctx);
                batch.ice_ping();
                batch.ice_flushBatchRequests();

                // Force the last batch request to be dispatched by the server thread using invocation timeouts
                // This is required to preven threading issue with the test interceptor implementation which
                // isn't thread safe
                prx.ice_invocationTimeout(10000).ice_ping();
            }
        }
    }
}
