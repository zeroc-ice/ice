// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interceptor;

import java.io.PrintWriter;

import test.Ice.interceptor.Test.InvalidInputException;
import test.Ice.interceptor.Test.MyObjectPrx;

public class Client extends test.Util.Application
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private int run(MyObjectPrx prx, InterceptorI interceptor)
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

        return 0;
    }

    private int runAmd(MyObjectPrx prx, InterceptorI interceptor, PrintWriter out)
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
        return 0;
    }

    @Override
    public int run(String[] args)
    {
        //
        // Create OA and servants
        //
        communicator().getProperties().setProperty("MyOA.AdapterId", "myOA");

        com.zeroc.Ice.ObjectAdapter oa = communicator().createObjectAdapterWithEndpoints("MyOA2", "tcp -h localhost");

        com.zeroc.Ice.Object servant = new MyObjectI();
        InterceptorI interceptor = new InterceptorI(servant);

        MyObjectPrx prx = MyObjectPrx.uncheckedCast(oa.addWithUUID(interceptor));

        PrintWriter out = getWriter();
        out.println("Collocation optimization on");
        int rs = run(prx, interceptor);
        if(rs != 0)
        {
            return rs;
        }

        out.println("Now with AMD");
        interceptor.clear();
        rs = runAmd(prx, interceptor, out);
        if(rs != 0)
        {
            return rs;
        }

        oa.activate(); // Only necessary for non-collocation optimized tests

        out.println("Collocation optimization off");
        interceptor.clear();
        prx = MyObjectPrx.uncheckedCast(prx.ice_collocationOptimized(false));
        rs = run(prx, interceptor);
        if(rs != 0)
        {
            return rs;
        }

        out.println("Now with AMD");
        interceptor.clear();
        rs = runAmd(prx, interceptor, out);

        return rs;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.interceptor");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        return initData;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Interceptor", args);
        System.gc();
        System.exit(status);
    }
}
