//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;

namespace Ice
{
    namespace interceptor
    {
        public class Client : TestHelper
        {
            private void runTest(Test.MyObjectPrx prx, InterceptorI interceptor)
            {
                var output = getWriter();
                output.Write("testing simple interceptor... ");
                output.Flush();
                test(interceptor.getLastOperation() == null);
                test(!interceptor.getLastStatus());
                prx.ice_ping();
                test(interceptor.getLastOperation().Equals("ice_ping"));
                test(!interceptor.getLastStatus());
                String typeId = prx.ice_id();
                test(interceptor.getLastOperation().Equals("ice_id"));
                test(!interceptor.getLastStatus());
                test(prx.ice_isA(typeId));
                test(interceptor.getLastOperation().Equals("ice_isA"));
                test(!interceptor.getLastStatus());
                test(prx.add(33, 12) == 45);
                test(interceptor.getLastOperation().Equals("add"));
                test(!interceptor.getLastStatus());
                output.WriteLine("ok");
                output.Write("testing retry... ");
                output.Flush();
                test(prx.addWithRetry(33, 12) == 45);
                test(interceptor.getLastOperation().Equals("addWithRetry"));
                test(!interceptor.getLastStatus());
                output.WriteLine("ok");
                output.Write("testing user exception... ");
                output.Flush();
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
                test(!interceptor.getLastStatus());
                output.WriteLine("ok");
                output.Write("testing ONE... ");
                output.Flush();
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
                test(!interceptor.getLastStatus());
                output.WriteLine("ok");
                output.Write("testing system exception... ");
                output.Flush();
                interceptor.clear();
                try
                {
                    prx.badSystemAdd(33, 12);
                    test(false);
                }
                catch(Ice.UnknownException)
                {
                    test(!prx.ice_isCollocationOptimized());
                }
                catch(MySystemException)
                {
                    test(prx.ice_isCollocationOptimized());
                }
                catch(Exception)
                {
                    test(false);
                }
                test(interceptor.getLastOperation().Equals("badSystemAdd"));
                test(!interceptor.getLastStatus());
                output.WriteLine("ok");

                output.Write("testing exceptions raised by the interceptor... ");
                output.Flush();
                testInterceptorExceptions(prx);
                output.WriteLine("ok");
            }

            private void runAmdTest(Test.MyObjectPrx prx, InterceptorI interceptor)
            {
                var output = getWriter();
                output.Write("testing simple interceptor... ");
                output.Flush();
                test(interceptor.getLastOperation() == null);
                test(!interceptor.getLastStatus());
                test(prx.amdAdd(33, 12) == 45);
                test(interceptor.getLastOperation().Equals("amdAdd"));
                test(interceptor.getLastStatus());
                output.WriteLine("ok");

                output.Write("testing retry... ");
                output.Flush();
                test(prx.amdAddWithRetry(33, 12) == 45);
                test(interceptor.getLastOperation().Equals("amdAddWithRetry"));
                test(interceptor.getLastStatus());

                {
                    var ctx = new Dictionary<string, string>();
                    ctx.Add("retry", "yes");
                    for(int i = 0; i < 10; ++i)
                    {
                        test(prx.amdAdd(33, 12, ctx) == 45);
                        test(interceptor.getLastOperation().Equals("amdAdd"));
                        test(interceptor.getLastStatus());
                    }
                }

                output.WriteLine("ok");

                output.Write("testing user exception... ");
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
                test(interceptor.getLastStatus());
                Console.WriteLine("ok");

                output.Write("testing ONE... ");
                output.Flush();
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
                test(interceptor.getLastStatus());
                output.WriteLine("ok");

                output.Write("testing system exception... ");
                output.Flush();
                interceptor.clear();
                try
                {
                    prx.amdBadSystemAdd(33, 12);
                    test(false);
                }
                catch(Ice.UnknownException)
                {
                    test(!prx.ice_isCollocationOptimized());
                }
                catch(MySystemException)
                {
                    test(prx.ice_isCollocationOptimized());
                }
                catch(Exception)
                {
                    test(false);
                }
                test(interceptor.getLastOperation().Equals("amdBadSystemAdd"));
                test(interceptor.getLastStatus());
                output.WriteLine("ok");

                output.Write("testing exceptions raised by the interceptor... ");
                output.Flush();
                testInterceptorExceptions(prx);
                output.WriteLine("ok");
            }

            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    //
                    // Create OA and servants
                    //
                    communicator.getProperties().setProperty("MyOA.AdapterId", "myOA");

                    Ice.ObjectAdapter oa = communicator.createObjectAdapterWithEndpoints("MyOA2", "tcp -h localhost");

                    Ice.Object servant = new MyObjectI();
                    InterceptorI interceptor = new InterceptorI(servant);

                    Test.MyObjectPrx prx = Test.MyObjectPrxHelper.uncheckedCast(oa.addWithUUID(interceptor));

                    var output = getWriter();

                    output.WriteLine("Collocation optimization on");
                    runTest(prx, interceptor);
                    output.WriteLine("Now with AMD");
                    interceptor.clear();
                    runAmdTest(prx, interceptor);

                    oa.activate(); // Only necessary for non-collocation optimized tests

                    output.WriteLine("Collocation optimization off");
                    interceptor.clear();
                    prx = Test.MyObjectPrxHelper.uncheckedCast(prx.ice_collocationOptimized(false));
                    runTest(prx, interceptor);

                    output.WriteLine("Now with AMD");
                    interceptor.clear();
                    runAmdTest(prx, interceptor);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }

            private void testInterceptorExceptions(Test.MyObjectPrx prx)
            {
                var exceptions = new List<Tuple<string, string>>();
                exceptions.Add(new Tuple<string, string>("raiseBeforeDispatch", "user"));
                exceptions.Add(new Tuple<string, string>("raiseBeforeDispatch", "notExist"));
                exceptions.Add(new Tuple<string, string>("raiseBeforeDispatch", "system"));
                exceptions.Add(new Tuple<string, string>("raiseAfterDispatch", "user"));
                exceptions.Add(new Tuple<string, string>("raiseAfterDispatch", "notExist"));
                exceptions.Add(new Tuple<string, string>("raiseAfterDispatch", "system"));
                foreach(var e in exceptions)
                {
                    var ctx = new Dictionary<string, string>();
                    ctx.Add(e.Item1, e.Item2);
                    try
                    {
                        prx.ice_ping(ctx);
                        test(false);
                    }
                    catch(Ice.UnknownUserException)
                    {
                        test(e.Item2.Equals("user"));
                    }
                    catch(Ice.ObjectNotExistException)
                    {
                        test(e.Item2.Equals("notExist"));
                    }
                    catch(Ice.UnknownException)
                    {
                        test(e.Item2.Equals("system")); // non-collocated
                    }
                    catch(MySystemException)
                    {
                        test(e.Item2.Equals("system")); // collocated
                    }
                    {
                        Ice.ObjectPrx batch = prx.ice_batchOneway();
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
    }
}
