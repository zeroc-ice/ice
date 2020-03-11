//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;

using Ice.interceptor.Test;

namespace Ice.interceptor
{
    public class RetryException : System.Exception
    {
    }

    public class Client : TestHelper
    {
        private void runTest(Test.IMyObjectPrx prx, Interceptor interceptor)
        {
            var output = getWriter();
            output.Write("testing simple interceptor... ");
            output.Flush();
            test(interceptor.getLastOperation() == null);
            test(!interceptor.AsyncCompletion);
            prx.IcePing();
            test(interceptor.getLastOperation()!.Equals("ice_ping"));
            test(!interceptor.AsyncCompletion);
            string typeId = prx.IceId();
            test(interceptor.getLastOperation()!.Equals("ice_id"));
            test(!interceptor.AsyncCompletion);
            test(prx.IceIsA(typeId));
            test(interceptor.getLastOperation()!.Equals("ice_isA"));
            test(!interceptor.AsyncCompletion);
            test(prx.add(33, 12) == 45);
            test(interceptor.getLastOperation()!.Equals("add"));
            test(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing retry... ");
            output.Flush();
            test(prx.addWithRetry(33, 12) == 45);
            test(interceptor.getLastOperation()!.Equals("addWithRetry"));
            test(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing remote exception... ");
            output.Flush();
            try
            {
                prx.badAdd(33, 12);
                test(false);
            }
            catch (InvalidInputException)
            {
                // expected
            }
            test(interceptor.getLastOperation()!.Equals("badAdd"));
            test(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing ONE... ");
            output.Flush();
            interceptor.clear();
            try
            {
                prx.notExistAdd(33, 12);
                test(false);
            }
            catch (ObjectNotExistException)
            {
                // expected
            }
            test(interceptor.getLastOperation()!.Equals("notExistAdd"));
            test(!interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing exceptions raised by the interceptor... ");
            output.Flush();
            testInterceptorExceptions(prx);
            output.WriteLine("ok");
        }

        private void runAmdTest(IMyObjectPrx prx, Interceptor interceptor)
        {
            var output = getWriter();
            output.Write("testing simple interceptor... ");
            output.Flush();
            test(interceptor.getLastOperation() == null);
            test(!interceptor.AsyncCompletion);
            test(prx.amdAdd(33, 12) == 45);
            test(interceptor.getLastOperation()!.Equals("amdAdd"));
            test(interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing retry... ");
            output.Flush();
            test(prx.amdAddWithRetry(33, 12) == 45);
            test(interceptor.getLastOperation()!.Equals("amdAddWithRetry"));
            test(interceptor.AsyncCompletion);

            {
                var ctx = new Dictionary<string, string>();
                ctx.Add("retry", "yes");
                for (int i = 0; i < 10; ++i)
                {
                    test(prx.amdAdd(33, 12, ctx) == 45);
                    test(interceptor.getLastOperation()!.Equals("amdAdd"));
                    test(interceptor.AsyncCompletion);
                }
            }

            output.WriteLine("ok");

            output.Write("testing user exception... ");
            try
            {
                prx.amdBadAdd(33, 12);
                test(false);
            }
            catch (InvalidInputException)
            {
                // expected
            }
            test(interceptor.getLastOperation()!.Equals("amdBadAdd"));
            test(interceptor.AsyncCompletion);
            Console.WriteLine("ok");

            output.Write("testing ONE... ");
            output.Flush();
            interceptor.clear();
            try
            {
                prx.amdNotExistAdd(33, 12);
                test(false);
            }
            catch (ObjectNotExistException)
            {
                // expected
            }
            test(interceptor.getLastOperation()!.Equals("amdNotExistAdd"));
            test(interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing exceptions raised by the interceptor... ");
            output.Flush();
            testInterceptorExceptions(prx);
            output.WriteLine("ok");
        }

        public override void run(string[] args)
        {
            var communicator = initialize(ref args);
            //
            // Create OA and servants
            //
            communicator.SetProperty("MyOA.AdapterId", "myOA");

            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA2", "tcp -h localhost");

            var myObject = new MyObject();
            var interceptor = new Interceptor(myObject);

            var prx = oa.AddWithUUID(interceptor, IMyObjectPrx.Factory);

            var output = getWriter();

            output.WriteLine("Collocation optimization on");
            runTest(prx, interceptor);
            output.WriteLine("Now with AMD");
            interceptor.clear();
            runAmdTest(prx, interceptor);

            oa.Activate(); // Only necessary for non-collocation optimized tests

            output.WriteLine("Collocation optimization off");
            interceptor.clear();
            prx = prx.Clone(collocationOptimized: false);
            runTest(prx, interceptor);

            output.WriteLine("Now with AMD");
            interceptor.clear();
            runAmdTest(prx, interceptor);
        }

        public static int Main(string[] args) => TestDriver.runTest<Client>(args);

        private void testInterceptorExceptions(Test.IMyObjectPrx prx)
        {
            var exceptions = new List<(string operation, string kind)>();
            exceptions.Add(("raiseBeforeDispatch", "invalidInput"));
            exceptions.Add(("raiseBeforeDispatch", "notExist"));
            exceptions.Add(("raiseAfterDispatch", "invalidInput"));
            exceptions.Add(("raiseAfterDispatch", "notExist"));
            foreach (var e in exceptions)
            {
                var ctx = new Dictionary<string, string>();
                ctx.Add(e.operation, e.kind);
                try
                {
                    prx.IcePing(ctx);
                    test(false);
                }
                catch (Test.InvalidInputException) when (e.kind.Equals("invalidInput"))
                {
                }
                catch (ObjectNotExistException) when (e.kind.Equals("notExist"))
                {
                }
            }
        }
    }
}
