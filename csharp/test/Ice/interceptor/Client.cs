//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
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
            Assert(prx.Add(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("add"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing retry... ");
            output.Flush();
            Assert(prx.AddWithRetry(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("addWithRetry"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing remote exception... ");
            output.Flush();
            try
            {
                prx.BadAdd(33, 12);
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
                prx.NotExistAdd(33, 12);
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
            Assert(prx.AmdAdd(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("amdAdd"));
            Assert(interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing retry... ");
            output.Flush();
            Assert(prx.AmdAddWithRetry(33, 12) == 45);
            Assert(interceptor.getLastOperation()!.Equals("amdAddWithRetry"));
            Assert(interceptor.AsyncCompletion);

            {
                var ctx = new Dictionary<string, string>
                {
                    { "retry", "yes" }
                };
                for (int i = 0; i < 10; ++i)
                {
                    Assert(prx.AmdAdd(33, 12, ctx) == 45);
                    Assert(interceptor.getLastOperation()!.Equals("amdAdd"));
                    Assert(interceptor.AsyncCompletion);
                }
            }

            output.WriteLine("ok");

            output.Write("testing user exception... ");
            try
            {
                prx.AmdBadAdd(33, 12);
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
                prx.AmdNotExistAdd(33, 12);
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

        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            //
            // Create OA and servants
            //
            communicator.SetProperty("MyOA.AdapterId", "myOA");

            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA2", "tcp -h localhost");

            var myObject = new MyObject();
            var interceptor = new Interceptor(myObject);

            IMyObjectPrx prx = oa.AddWithUUID(interceptor, IMyObjectPrx.Factory);

            System.IO.TextWriter output = GetWriter();

            output.WriteLine("With sync dispatch");
            runTest(prx, interceptor);
            output.WriteLine("Now with AMD");
            interceptor.clear();
            runAmdAssert(prx, interceptor);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);

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
