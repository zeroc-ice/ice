// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client extends Ice.Application
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
    run(Test.MyObjectPrx prx, InterceptorI interceptor)
    {
        System.out.print("testing simple interceptor...");
        System.out.flush();
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
        System.out.println("ok");
        System.out.print("testing retry...");
        System.out.flush();
        test(prx.addWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("addWithRetry"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchOK));
        System.out.println("ok");
        System.out.print("testing user exception...");
        System.out.flush();
        try
        {
            prx.badAdd(33, 12);
            test(false);
        }
        catch(Test.InvalidInputException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("badAdd"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchUserException));
        System.out.println("ok");
        System.out.print("testing ONE...");
        System.out.flush();
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
        System.out.println("ok");
        System.out.print("testing system exception...");
        System.out.flush();
        interceptor.clear();
        try
        {
            prx.badSystemAdd(33, 12);
            test(false);
        }
        catch(Ice.InitializationException e)
        {
            test(prx.ice_isCollocationOptimized());
        }
        catch(Ice.UnknownLocalException e)
        {
            test(!prx.ice_isCollocationOptimized());
        }
        test(interceptor.getLastOperation().equals("badSystemAdd"));
        test(interceptor.getLastStatus() == null);
        System.out.println("ok");
        if(!prx.ice_isCollocationOptimized())
        {
            System.out.print("testing simple AMD...");
            System.out.flush();
            test(prx.amdAdd(33, 12) == 45);
            test(interceptor.getLastOperation().equals("amdAdd"));
            test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
            System.out.println("ok");
        }
        return 0;
    }
    
    private int
    runAmd(Test.MyObjectPrx prx, AMDInterceptorI interceptor)
    {
        System.out.print("testing simple interceptor...");
        System.out.flush();
        test(interceptor.getLastOperation() == null);
        test(interceptor.getLastStatus() == null);
        test(prx.amdAdd(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAdd"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus().equals(Ice.DispatchStatus.DispatchOK));
        System.out.println("ok");
        System.out.print("testing retry...");
        System.out.flush();
        test(prx.amdAddWithRetry(33, 12) == 45);
        test(interceptor.getLastOperation().equals("amdAddWithRetry"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus().equals(Ice.DispatchStatus.DispatchOK));
        System.out.println("ok");
        System.out.print("testing user exception...");
        System.out.flush();
        try
        {
            prx.amdBadAdd(33, 12);
            test(false);
        }
        catch(Test.InvalidInputException e)
        {
            // expected
        }
        test(interceptor.getLastOperation().equals("amdBadAdd"));
        test(interceptor.getLastStatus().equals(Ice.DispatchStatus.DispatchAsync));
        test(interceptor.getActualStatus().equals(Ice.DispatchStatus.DispatchUserException));
        System.out.println("ok");
        System.out.print("testing ONE...");
        System.out.flush();
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
        System.out.println("ok");
        System.out.print("testing system exception...");
        System.out.flush();
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
        System.out.println("ok");
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
        
        Test.MyObjectPrx prx = Test.MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(interceptor));
        
        oa.activate();
       
        System.out.println("Collocation optimization on");
        int rs = run(prx, interceptor);
        if(rs == 0)
        {
            System.out.println("Collocation optimization off");
            interceptor.clear();
            prx = Test.MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));
            rs = run(prx, interceptor);

            if(rs == 0)
            {
                System.out.println("Now with AMD");
                AMDInterceptorI amdInterceptor = new AMDInterceptorI(servant);
                prx = Test.MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(amdInterceptor));
                prx = Test.MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));

                rs = runAmd(prx, amdInterceptor);
            }
        }
        return rs;
    }


    public static void
    main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Interceptor", args);
        System.exit(status);
    }
}


    
