// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        private static void test(bool b)
        {
            if(!b)
            {
                throw new Exception();
            }
        }

        private int run(Test.MyObjectPrx prx, InterceptorI interceptor)
        {
            Console.Out.Write("testing simple interceptor... ");
            Console.Out.Flush();
            test(interceptor.getLastOperation() == null);
            test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchAsync);
            prx.ice_ping();
            test(interceptor.getLastOperation().Equals("ice_ping"));
            test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchOK);
            String typeId = prx.ice_id();
            test(interceptor.getLastOperation().Equals("ice_id"));
            test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchOK);
            test(prx.ice_isA(typeId));
            test(interceptor.getLastOperation().Equals("ice_isA"));
            test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchOK);
            test(prx.add(33, 12) == 45);
            test(interceptor.getLastOperation().Equals("add"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchOK));
            Console.WriteLine("ok");
            Console.Out.Write("testing retry... ");
            Console.Out.Flush();
            test(prx.addWithRetry(33, 12) == 45);
            test(interceptor.getLastOperation().Equals("addWithRetry"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchOK));
            Console.WriteLine("ok");
            Console.Out.Write("testing user exception... ");
            Console.Out.Flush();
            try
            {
                prx.badAdd(33, 12);
                test(false);
            }
            catch(Test.InvalidInputException)
            {
                // expected
            }
            test(interceptor.getLastOperation().Equals("badAdd"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchUserException));
            Console.WriteLine("ok");
            Console.Out.Write("testing ONE... ");
            Console.Out.Flush();
            interceptor.clear();
            try
            {
                prx.notExistAdd(33, 12);
                test(false);
            }
            catch(Ice.ObjectNotExistException)
            {
                // expected
            }
            test(interceptor.getLastOperation().Equals("notExistAdd"));
            test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchAsync);
            Console.WriteLine("ok");
            Console.Out.Write("testing system exception... ");
            Console.Out.Flush();
            interceptor.clear();
            try
            {
                prx.badSystemAdd(33, 12);
                test(false);
            }
            catch(Ice.UnknownLocalException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            test(interceptor.getLastOperation().Equals("badSystemAdd"));
            test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchAsync);
            Console.WriteLine("ok");
            if(!prx.ice_isCollocationOptimized())
            {
                Console.Out.Write("testing simple AMD... ");
                Console.Out.Flush();
                test(prx.amdAdd(33, 12) == 45);
                test(interceptor.getLastOperation().Equals("amdAdd"));
                test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchAsync));
                Console.WriteLine("ok");
            }
            return 0;
        }

        private int runAmd(Test.MyObjectPrx prx, AMDInterceptorI interceptor)
        {
            Console.Out.Write("testing simple interceptor... ");
            Console.Out.Flush();
            test(interceptor.getLastOperation() == null);
            test(interceptor.getLastStatus() == Ice.DispatchStatus.DispatchAsync);
            test(prx.amdAdd(33, 12) == 45);
            test(interceptor.getLastOperation().Equals("amdAdd"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchAsync));
            test(interceptor.getActualStatus().Equals(Ice.DispatchStatus.DispatchOK));
            Console.WriteLine("ok");
            Console.Out.Write("testing retry... ");
            Console.Out.Flush();
            test(prx.amdAddWithRetry(33, 12) == 45);
            test(interceptor.getLastOperation().Equals("amdAddWithRetry"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchAsync));
            test(interceptor.getActualStatus().Equals(Ice.DispatchStatus.DispatchOK));
            Console.WriteLine("ok");
            Console.Out.Write("testing user exception... ");
            Console.Out.Flush();
            try
            {
                prx.amdBadAdd(33, 12);
                test(false);
            }
            catch(Test.InvalidInputException)
            {
                // expected
            }
            test(interceptor.getLastOperation().Equals("amdBadAdd"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchAsync));
            test(interceptor.getActualStatus().Equals(Ice.DispatchStatus.DispatchUserException));
            Console.WriteLine("ok");
            Console.Out.Write("testing ONE... ");
            Console.Out.Flush();
            interceptor.clear();
            try
            {
                prx.amdNotExistAdd(33, 12);
                test(false);
            }
            catch(Ice.ObjectNotExistException)
            {
                // expected
            }
            test(interceptor.getLastOperation().Equals("amdNotExistAdd"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchAsync));
            test(interceptor.getActualStatus() == Ice.DispatchStatus.DispatchAsync);
            test(interceptor.getException() is Ice.ObjectNotExistException);
            Console.WriteLine("ok");
            Console.Out.Write("testing system exception... ");
            Console.Out.Flush();
            interceptor.clear();
            try
            {
                prx.amdBadSystemAdd(33, 12);
                test(false);
            }
            catch(Ice.UnknownLocalException)
            {
                test(!prx.ice_isCollocationOptimized());
            }
            test(interceptor.getLastOperation().Equals("amdBadSystemAdd"));
            test(interceptor.getLastStatus().Equals(Ice.DispatchStatus.DispatchAsync));
            test(interceptor.getActualStatus() == Ice.DispatchStatus.DispatchAsync);
            test(interceptor.getException() is Ice.InitializationException);
            Console.WriteLine("ok");
            return 0;
        }

        public override int run(string[] args)
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

            Console.WriteLine("Collocation optimization on");
            int rs = run(prx, interceptor);
            if(rs == 0)
            {
                Console.WriteLine("Collocation optimization off");
                interceptor.clear();
                prx = Test.MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));
                rs = run(prx, interceptor);

                if(rs == 0)
                {
                    Console.WriteLine("Now with AMD");
                    AMDInterceptorI amdInterceptor = new AMDInterceptorI(servant);
                    prx = Test.MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(amdInterceptor));
                    prx = Test.MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));

                    rs = runAmd(prx, amdInterceptor);
                }
            }
            return rs;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        Ice.InitializationData data = new Ice.InitializationData();
#if COMPACT
        //
        // When using Ice for .NET Compact Framework, we need to specify
        // the assembly so that Ice can locate classes and exceptions.
        //
        data.properties = Ice.Util.createProperties();
        data.properties.setProperty("Ice.FactoryAssemblies", "client");
#endif
        return app.main(args, data);
    }
}
