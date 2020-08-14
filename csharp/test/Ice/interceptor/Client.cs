//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class RetryException : Exception
    {
    }

    public class Client : TestHelper
    {
        private void Run(IMyObjectPrx prx, Interceptor interceptor)
        {
            System.IO.TextWriter output = GetWriter();
            output.Write("testing simple interceptor... ");
            output.Flush();
            Assert(interceptor.GetLastOperation() == null);
            Assert(!interceptor.AsyncCompletion);
            prx.IcePing();
            Assert(interceptor.GetLastOperation()!.Equals("ice_ping"));
            Assert(!interceptor.AsyncCompletion);
            string typeId = prx.IceId();
            Assert(interceptor.GetLastOperation()!.Equals("ice_id"));
            Assert(!interceptor.AsyncCompletion);
            Assert(prx.IceIsA(typeId));
            Assert(interceptor.GetLastOperation()!.Equals("ice_isA"));
            Assert(!interceptor.AsyncCompletion);
            Assert(prx.Add(33, 12) == 45);
            Assert(interceptor.GetLastOperation()!.Equals("add"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing retry... ");
            output.Flush();
            Assert(prx.AddWithRetry(33, 12) == 45);
            Assert(interceptor.GetLastOperation()!.Equals("addWithRetry"));
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
            Assert(interceptor.GetLastOperation()!.Equals("badAdd"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");
            output.Write("testing ONE... ");
            output.Flush();
            interceptor.Clear();
            try
            {
                prx.NotExistAdd(33, 12);
                Assert(false);
            }
            catch (ObjectNotExistException)
            {
                // expected
            }
            Assert(interceptor.GetLastOperation()!.Equals("notExistAdd"));
            Assert(!interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing exceptions raised by the interceptor... ");
            output.Flush();
            TestInterceptorExceptions(prx);
            output.WriteLine("ok");

            output.Write("testing invocation interceptors... ");
            output.Flush();
            {
                using var communicator = new Communicator(prx.Communicator.GetProperties());
                communicator.InterceptInvocation((target, request, next) =>
                {
                    request.Context["interceptor-1"] = "interceptor-1";
                    request.AddBinaryContextEntry(110, 110, (ostr, v) => ostr.WriteInt(v));

                    return next(target, request);
                });

                communicator.InterceptInvocation(async (target, request, next) =>
                {
                    Assert(request.Context["interceptor-1"] == "interceptor-1");
                    request.Context["interceptor-2"] = "interceptor-2";
                    request.AddBinaryContextEntry(120, 120, (ostr, v) => ostr.WriteInt(v));
                    IncomingResponseFrame response = await next(target, request);
                    Assert(response.BinaryContext.ContainsKey(110));
                    Assert(response.BinaryContext[110].Read(istr => istr.ReadInt()) == 110);
                    Assert(response.BinaryContext.ContainsKey(120));
                    Assert(response.BinaryContext[120].Read(istr => istr.ReadInt()) == 120);
                    return response;
                });
                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                prx1.Op1();
            }

            {
                // An interceptor can stop the chain and directly return a response without calling next
                using var communicator = new Communicator(prx.Communicator.GetProperties());
                communicator.InterceptInvocation((target, request, next) =>
                {
                    request.Context["interceptor-1"] = "interceptor-1";
                    return next(target, request);
                });

                communicator.InterceptInvocation((target, request, next) =>
                {
                    Assert(request.Context["interceptor-1"] == "interceptor-1");
                    return new ValueTask<IncomingResponseFrame>(
                        IncomingResponseFrame.WithVoidReturnValue(target.Protocol, target.Encoding));
                });

                communicator.InterceptInvocation((target, request, next) =>
                {
                    Assert(false);
                    return next(target, request);
                });
                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                prx1.Op1();
            }

            {
                // throwing from an interceptor stops the interceptor chain
                using var communicator = new Communicator(prx.Communicator.GetProperties());
                communicator.InterceptInvocation((target, request, next) =>
                {
                    request.Context["interceptor-1"] = "interceptor-1";
                    return next(target, request);
                });

                communicator.InterceptInvocation((target, request, next) =>
                {
                    Assert(request.Context["interceptor-1"] == "interceptor-1");
                    throw new InvalidOperationException("stop interceptor chain");
                });

                communicator.InterceptInvocation((target, request, next) =>
                {
                    Assert(false);
                    return next(target, request);
                });
                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                try
                {
                    prx1.Op1();
                    Assert(false);
                }
                catch (InvalidOperationException)
                {
                }

            }
            output.WriteLine("ok");

            output.Write("testing binary context... ");
            output.Flush();
            bool ice2 = Communicator()!.DefaultProtocol != Protocol.Ice1;

            if (ice2)
            {
                for (int size = 128; size < 4096; size *= 2)
                {
                    var token = new Token(1, "mytoken", Enumerable.Range(0, size).Select(i => (byte)2).ToArray());
                    var request = OutgoingRequestFrame.WithParamList(prx,
                                                                     "opWithBinaryContext",
                                                                     idempotent: false,
                                                                     compress: false,
                                                                     format: default,
                                                                     context: null,
                                                                     token,
                                                                     Token.IceWriter);
                    request.AddBinaryContextEntry(1, token, Token.IceWriter);
                    request.AddBinaryContextEntry(3, (short)size, (ostr, value) => ostr.WriteShort(value));
                    request.AddBinaryContextEntry(2,
                                                  Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                                                  Ice.StringSeqHelper.IceWriter);

                    // Adding the same key twice throws ArgumentException
                    try
                    {
                        request.AddBinaryContextEntry(1, token, Token.IceWriter);
                        Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                    prx.Invoke(request);

                    Assert(request.IsSealed);
                    // Adding to a sealed frame throws InvalidOperationException
                    try
                    {
                        request.AddBinaryContextEntry(10, token, Token.IceWriter);
                        Assert(false);
                    }
                    catch (InvalidOperationException)
                    {
                    }

                    // repeat with compressed frame
                    request = OutgoingRequestFrame.WithParamList(prx,
                                                                 "opWithBinaryContext",
                                                                 idempotent: false,
                                                                 compress: false,
                                                                 format: default,
                                                                 context: null,
                                                                 token,
                                                                 Token.IceWriter);
                    request.AddBinaryContextEntry(1, token, Token.IceWriter);
                    request.AddBinaryContextEntry(3, (short)size, (ostr, value) => ostr.WriteShort(value));
                    request.AddBinaryContextEntry(2,
                                                  Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                                                  Ice.StringSeqHelper.IceWriter);
                    Assert(request.CompressPayload() == CompressionResult.Success);
                    prx.Invoke(request);

                    // repeat compressed the frame before writing the context
                    request = OutgoingRequestFrame.WithParamList(prx,
                                                                 "opWithBinaryContext",
                                                                 idempotent: false,
                                                                 compress: false,
                                                                 format: default,
                                                                 context: null,
                                                                 token,
                                                                 Token.IceWriter);

                    Assert(request.CompressPayload() == CompressionResult.Success);
                    request.AddBinaryContextEntry(1, token, Token.IceWriter);
                    request.AddBinaryContextEntry(3, (short)size, (ostr, value) => ostr.WriteShort(value));
                    request.AddBinaryContextEntry(2,
                                                  Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                                                  Ice.StringSeqHelper.IceWriter);
                    prx.Invoke(request);
                }
            }
            else
            {
                var token = new Token(1, "mytoken", Enumerable.Range(0, 256).Select(i => (byte)2).ToArray());
                var request = OutgoingRequestFrame.WithParamList(prx,
                                                                 "opWithBinaryContext",
                                                                 idempotent: false,
                                                                 compress: false,
                                                                 format: default,
                                                                 context: null,
                                                                 token,
                                                                 Token.IceWriter);
                try
                {
                    request.AddBinaryContextEntry(1, token, Token.IceWriter);
                    Assert(false);
                }
                catch (NotSupportedException)
                {
                }
                prx.Invoke(request);
                Assert(request.IsSealed);
            }
            output.WriteLine("ok");
        }

        private void RunAmd(IMyObjectPrx prx, Interceptor interceptor)
        {
            System.IO.TextWriter output = GetWriter();
            output.Write("testing simple interceptor... ");
            output.Flush();
            Assert(interceptor.GetLastOperation() == null);
            Assert(!interceptor.AsyncCompletion);
            Assert(prx.AmdAdd(33, 12) == 45);
            Assert(interceptor.GetLastOperation()!.Equals("amdAdd"));
            Assert(interceptor.AsyncCompletion);
            output.WriteLine("ok");

            output.Write("testing retry... ");
            output.Flush();
            Assert(prx.AmdAddWithRetry(33, 12) == 45);
            Assert(interceptor.GetLastOperation()!.Equals("amdAddWithRetry"));
            Assert(interceptor.AsyncCompletion);

            {
                var ctx = new Dictionary<string, string>
                {
                    { "retry", "yes" }
                };
                for (int i = 0; i < 10; ++i)
                {
                    Assert(prx.AmdAdd(33, 12, ctx) == 45);
                    Assert(interceptor.GetLastOperation()!.Equals("amdAdd"));
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
            Assert(interceptor.GetLastOperation()!.Equals("amdBadAdd"));
            Assert(interceptor.AsyncCompletion);
            Console.WriteLine("ok");

            output.Write("testing ONE... ");
            output.Flush();
            interceptor.Clear();
            try
            {
                prx.AmdNotExistAdd(33, 12);
                Assert(false);
            }
            catch (ObjectNotExistException)
            {
                // expected
            }
            Assert(interceptor.GetLastOperation()!.Equals("amdNotExistAdd"));
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

            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA2", GetTestEndpoint(0));
            oa.Activate();
            var myObject = new MyObject();
            var interceptor = new Interceptor(myObject);

            IMyObjectPrx prx = oa.AddWithUUID(interceptor, IMyObjectPrx.Factory);

            System.IO.TextWriter output = GetWriter();

            output.WriteLine("With sync dispatch");
            Run(prx, interceptor);
            output.WriteLine("Now with AMD");
            interceptor.Clear();
            RunAmd(prx, interceptor);
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
