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
import test.Ice.interceptor.Test.MyObjectPrxHelper;

public class Client extends test.Util.Application
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private int
    run(MyObjectPrx prx, InterceptorI interceptor)
    {
        PrintWriter out = getWriter();
        out.print("testing simple interceptor... ");
        out.flush();
        test(interceptor.getLastOperation() == null);
        test(!interceptor.getLastStatus());
        prx.ice_ping();
        test(interceptor.getLastOperation().equals("ice_ping"));
        test(interceptor.getLastStatus());
        String typeId = prx.ice_id();
        test(interceptor.getLastOperation().equals("ice_id"));
        test(interceptor.getLastStatus());
        test(prx.ice_isA(typeId));
        test(interceptor.getLastOperation().equals("ice_isA"));
        test(interceptor.getLastStatus());
        test(prx.add(33, 12) == 45);
        test(interceptor.getLastOperation().equals("add"));
        test(interceptor.getLastStatus());
        out.println("ok");
        out.print("testing retry... ");
        out.flush();
        test(prx.addWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("addWithRetry"));
        test(interceptor.getLastStatus());
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
        test(interceptor.getLastStatus());
        out.println("ok");
        out.print("testing ONE... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.notExistAdd(33, 12);
            test(false);
        }
        catch(Ice.ObjectNotExistException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("notExistAdd"));
        out.println("ok");
        out.print("testing system exception... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.badSystemAdd(33, 12);
            test(false);
        }
        catch(Ice.UnknownException e)
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
        out.println("ok");

        out.print("testing simple AMD... ");
        out.flush();
        test(prx.amdAdd(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAdd"));
        test(!interceptor.getLastStatus());
        out.println("ok");

        return 0;
    }

    private int
    runAmd(MyObjectPrx prx, AMDInterceptorI interceptor, PrintWriter out)
    {
        out.print("testing simple interceptor... ");
        out.flush();
        test(interceptor.getLastOperation() == null);
        test(!interceptor.getLastStatus());
        test(prx.amdAdd(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAdd"));
        test(!interceptor.getLastStatus());
        out.println("ok");
        out.print("testing retry... ");
        out.flush();
        test(prx.amdAddWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAddWithRetry"));
        test(!interceptor.getLastStatus());
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
        test(!interceptor.getLastStatus());
        out.println("ok");
        out.print("testing ONE... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.amdNotExistAdd(33, 12);
            test(false);
        }
        catch(Ice.ObjectNotExistException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("amdNotExistAdd"));
        test(!interceptor.getLastStatus());
        test(interceptor.getException() instanceof Ice.ObjectNotExistException);
        out.println("ok");
        out.print("testing system exception... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.amdBadSystemAdd(33, 12);
            test(false);
        }
        catch(Ice.UnknownException e)
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
        test(!interceptor.getLastStatus());
        test(interceptor.getException() instanceof MySystemException);
        out.println("ok");
        return 0;
    }

    @Override
    public int
    run(String[] args)
    {
        //
        // Create OA and servants
        //
        communicator().getProperties().setProperty("MyOA.AdapterId", "myOA");

        Ice.ObjectAdapter oa = communicator().createObjectAdapterWithEndpoints("MyOA2", "tcp -h localhost");

        Ice.Object servant = new MyObjectI();
        InterceptorI interceptor = new InterceptorI(servant);
        AMDInterceptorI amdInterceptor = new AMDInterceptorI(servant);

        MyObjectPrx prx = MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(interceptor));
        MyObjectPrx prxForAMD = MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(amdInterceptor));

        PrintWriter out = getWriter();
        out.println("Collocation optimization on");
        int rs = run(prx, interceptor);
        if(rs != 0)
        {
            return rs;
        }

        out.println("Now with AMD");
        rs = runAmd(prxForAMD, amdInterceptor, out);
        if(rs != 0)
        {
            return rs;
        }

        oa.activate(); // Only necessary for non-collocation optimized tests

        out.println("Collocation optimization off");
        interceptor.clear();
        prx = MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));
        rs = run(prx, interceptor);
        if(rs != 0)
        {
            return rs;
        }

        out.println("Now with AMD");
        amdInterceptor.clear();
        prxForAMD = MyObjectPrxHelper.uncheckedCast(prxForAMD.ice_collocationOptimized(false));
        rs = runAmd(prxForAMD, amdInterceptor, out);

        return rs;
    }


    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.interceptor");
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
