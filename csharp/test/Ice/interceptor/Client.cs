//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class RetryException : Exception
    {
    }

    public class Client : TestHelper
    {
        private void runTest(IMyObjectPrx prx, Interceptor interceptor)
        {
            System.IO.TextWriter output = GetWriter();
            output.Write("testing simple interceptor... ");
            output.Flush();
            Assert(interceptor.getLastOperation() == null);
            Assert(!interceptor.AsyncCompletion);
            prx.IcePing();
            Assert(interceptor.getLastOperation()!.Equals("ice_ping"));
            Assert(!interceptor.AsyncCompletion);
            string typeId = prx.IceId();
            Assert(interceptor.getLastOperation()!.Equals("ice_id"));
            Assert(!interceptor.AsyncCompletion);
            Assert(prx.IceIsA(typeId));
            Assert(interceptor.getLastOperation()!.Equals("ice_isA"));
            Assert(!interceptor.AsyncCompletion);
            Assert(prx.add(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("add"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing retry... ");
            output.Flush();
            Assert(prx.addWithRetry(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("addWithRetry"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing remote exception... ");
            output.Flush();
            try
            {
                prx.badAdd(33, 12);
                Assert(false);
            }
            catch (InvalidInputException)
            {
                // expected
            }
            Assert(interceptor.getLastOperation()!.Equals("badAdd"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing ONE... ");
            output.Flush();
            interceptor.clear();
            try
            {
                prx.notExistAdd(33, 12);
                Assert(false);
            }
            catch (ObjectNotExistException)
            {
                // expected
            }
            Assert(interceptor.getLastOperation()!.Equals("notExistAdd"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing exceptions raised by the interceptor... ");
            output.Flush();
            TestInterceptorExceptions(prx);
            output.WriteLine("ok");
        }

        private void runAmdAssert(IMyObjectPrx prx, Interceptor interceptor)
        {
            System.IO.TextWriter output = GetWriter();
            output.Write("testing simple interceptor... ");
            output.Flush();
            Assert(interceptor.getLastOperation() == null);
            Assert(!interceptor.AsyncCompletion);
            Assert(prx.amdAdd(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("amdAdd"));
            Assert(interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing retry... ");
            output.Flush();
            Assert(prx.amdAddWithRetry(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("amdAddWithRetry"));
            Assert(interceptor.AsyncCompletion);

            {
                var ctx = new Dictionary<string, string>
                {
                    { "retry", "yes" }
                };
                for (int i = 0; i < 10; ++i)
                {
                    Assert(prx.amdAdd(33, 12, ctx) == 45);
                    Assert(interceptor.getLastOperation()!.Equals("amdAdd"));
                    Assert(interceptor.AsyncCompletion);
                }
            }

            output.WriteLine("ok");

            output.Write("testing user exception... ");
            try
            {
                prx.amdBadAdd(33, 12);
                Assert(false);
            }
            catch (InvalidInputException)
            {
                // expected
            }
            Assert(interceptor.getLastOperation()!.Equals("amdBadAdd"));
            Assert(interceptor.AsyncCompletion);
            Console.WriteLine("ok");

            output.Write("testing ONE... ");
            output.Flush();
            interceptor.clear();
            try
            {
                prx.amdNotExistAdd(33, 12);
                Assert(false);
            }
            catch (ObjectNotExistException)
            {
                // expected
            }
            Assert(interceptor.getLastOperation()!.Equals("amdNotExistAdd"));
            Assert(interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing exceptions raised by the interceptor... ");
            output.Flush();
            TestInterceptorExceptions(prx);
            output.WriteLine("ok");
        }

        public override void Run(string[] args)
        {
            Communicator communicator = Initialize(ref args);
            //
            // Create OA and servants
            //
            communicator.SetProperty("MyOA.AdapterId", "myOA");

            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA2", "tcp -h localhost");

            var myObject = new MyObject();
            var interceptor = new Interceptor(myObject);

            IMyObjectPrx prx = oa.AddWithUUID(interceptor, IMyObjectPrx.Factory);

            System.IO.TextWriter output = GetWriter();

            output.WriteLine("Collocation optimization on");
            runTest(prx, interceptor);
            output.WriteLine("Now with AMD");
            interceptor.clear();
            runAmdAssert(prx, interceptor);

            oa.Activate(); // Only necessary for non-collocation optimized tests

            output.WriteLine("Collocation optimization off");
            interceptor.clear();
            prx = prx.Clone(collocationOptimized: false);
            runTest(prx, interceptor);

            output.WriteLine("Now with AMD");
            interceptor.clear();
            runAmdAssert(prx, interceptor);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);

        private void TestInterceptorExceptions(IMyObjectPrx prx)
        {
            var exceptions = new List<(string operation, string kind)>
            {
                ("raiseBeforeDispatch", "invalidInput"),
                ("raiseBeforeDispatch", "notExist"),
                ("raiseAfterDispatch", "invalidInput"),
                ("raiseAfterDispatch", "notExist")
            };
            foreach ((string operation, string kind) in exceptions)
            {
                var ctx = new Dictionary<string, string>
                {
                    { operation, kind }
                };
                try
                {
                    prx.IcePing(ctx);
                    Assert(false);
                }
                catch (InvalidInputException) when (kind.Equals("invalidInput"))
                {
                }
                catch (ObjectNotExistException) when (kind.Equals("notExist"))
                {
                }
            }
        }
    }
}
