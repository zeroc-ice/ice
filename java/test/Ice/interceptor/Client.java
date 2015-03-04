// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        test(interceptor.getLastStatus() == null);
        prx.ice_ping();
        test(interceptor.getLastOperation().equals("ice_ping"));
        test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchOK);
        String typeId = prx.ice_id();
        test(interceptor.getLastOperation().equals("ice_id"));
        test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchOK);
        test(prx.ice_isA(typeId));
        test(interceptor.getLastOperation().equals("ice_isA"));
        test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchOK);
        test(prx.add(33, 12) == 45);
        test(interceptor.getLastOperation().equals("add"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchOK));
        out.println("ok");
        out.print("testing retry... ");
        out.flush();
        test(prx.addWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("addWithRetry"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchOK));
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
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchUserException));
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
        test(interceptor.getLastStatus() == null);
        out.println("ok");
        out.print("testing system exception... ");
        out.flush();
        interceptor.clear();
        try
        {
            prx.badSystemAdd(33, 12);
            test(false);
        }
        catch(Ice.UnknownLocalException e)
        {
        }
        catch(Throwable ex)
        {
            test(false);
        }
        test(interceptor.getLastOperation().equals("badSystemAdd"));
        test(interceptor.getLastStatus() == null);
        out.println("ok");
        if(!prx.ice_isCollocationOptimized())
        {
            out.print("testing simple AMD... ");
            out.flush();
            test(prx.amdAdd(33, 12) == 45);
            test(interceptor.getLastOperation().equals("amdAdd"));
            test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
            out.println("ok");
        }
        return 0;
    }
    
    private int
    runAmd(MyObjectPrx prx, AMDInterceptorI interceptor, PrintWriter out)
    {
                out.print("testing simple interceptor... ");
        out.flush();
        test(interceptor.getLastOperation() == null);
        test(interceptor.getLastStatus() == null);
        test(prx.amdAdd(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAdd"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus().equals(Ice.DispatchStatus.DispatchOK));
        out.println("ok");
        out.print("testing retry... ");
        out.flush();
        test(prx.amdAddWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAddWithRetry"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus().equals(Ice.DispatchStatus.DispatchOK));
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
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus().equals(Ice.DispatchStatus.DispatchUserException));
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
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus() == null);
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
        catch(Ice.UnknownLocalException e)
        {
            test(!prx.ice_isCollocationOptimized());
        }
        test(interceptor.getLastOperation().equals("amdBadSystemAdd"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus() == null);
        test(interceptor.getException() instanceof Ice.InitializationException);
        out.println("ok");
        return 0;
    }

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
        
        MyObjectPrx prx = MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(interceptor));
        
        oa.activate();
       
        PrintWriter out = getWriter();
                out.println("Collocation optimization on");
        int rs = run(prx, interceptor);
        if(rs == 0)
        {
            out.println("Collocation optimization off");
            interceptor.clear();
            prx = MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));
            rs = run(prx, interceptor);

            if(rs == 0)
            {
                out.println("Now with AMD");
                AMDInterceptorI amdInterceptor = new AMDInterceptorI(servant);
                prx = MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(amdInterceptor));
                prx = MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));

                rs = runAmd(prx, amdInterceptor, out);
            }
        }
        return rs;
    }


    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
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


    
