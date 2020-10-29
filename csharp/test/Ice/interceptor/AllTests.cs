// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public static class AllTests
    {
        public static IMyObjectPrx Run(TestHelper helper)
        {
            bool ice2 = helper.Protocol == Protocol.Ice2;
            var prx = IMyObjectPrx.Parse(helper.GetTestProxy("test"), helper.Communicator!);
            System.IO.TextWriter output = helper.Output;

            output.Write("testing retry... ");
            output.Flush();
            TestHelper.Assert(prx.AddWithRetry(33, 12) == 45);
            output.WriteLine("ok");

            output.Write("testing remote exception... ");
            output.Flush();
            try
            {
                prx.BadAdd(33, 12);
                TestHelper.Assert(false);
            }
            catch (InvalidInputException)
            {
                // expected
            }
            output.WriteLine("ok");

            output.Write("testing ONE... ");
            output.Flush();
            try
            {
                prx.NotExistAdd(33, 12);
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
                // expected
            }
            output.WriteLine("ok");

            output.Write("testing exceptions raised by the interceptor... ");
            output.Flush();
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
                    TestHelper.Assert(false);
                }
                catch (InvalidInputException) when (kind == "invalidInput")
                {
                }
                catch (ObjectNotExistException) when (kind == "notExist")
                {
                }
            }
            output.WriteLine("ok");

            output.Write("testing invocation interceptors... ");
            output.Flush();
            {
                var tasks = new List<Task>();
                var invocationContext = new AsyncLocal<int>();
                using var communicator = new Communicator(
                    prx.Communicator.GetProperties(),
                    invocationInterceptors: new InvocationInterceptor[]
                    {
                        (target, request, next, cancel) =>
                        {
                            if (ice2)
                            {
                                request.ContextOverride["interceptor-1"] = "interceptor-1";
                                request.AddBinaryContextEntry(110, 110, (ostr, v) => ostr.WriteInt(v));
                            }
                            return next(target, request, cancel);
                        },
                        async (target, request, next, cancel) =>
                        {
                            if (ice2)
                            {
                                TestHelper.Assert(request.Context["interceptor-1"] == "interceptor-1");
                                request.ContextOverride["interceptor-2"] = "interceptor-2";
                                request.AddBinaryContextEntry(120, 120, (ostr, v) => ostr.WriteInt(v));
                            }
                            IncomingResponseFrame response = await next(target, request, cancel);
                            TestHelper.Assert(invocationContext.Value == int.Parse(request.Context["local-user"]));
                            if (ice2)
                            {
                                TestHelper.Assert(response.BinaryContext.ContainsKey(110));
                                TestHelper.Assert(response.BinaryContext[110].Read(istr => istr.ReadInt()) == 110);
                                TestHelper.Assert(response.BinaryContext.ContainsKey(120));
                                TestHelper.Assert(response.BinaryContext[120].Read(istr => istr.ReadInt()) == 120);
                            }
                            return response;
                        }
                    });

                for (int i = 0; i < 10; ++i)
                {
                    invocationContext.Value = i;
                    var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                    Task t = prx1.Op1Async(new Dictionary<string, string> { { "local-user", $"{i}"} });
                    tasks.Add(t);
                }
                Task.WaitAll(tasks.ToArray());
            }

            {
                IncomingResponseFrame? response = null;
                int invocations = 0;
                // An interceptor can stop the chain and directly return a response without calling next,
                // the first invocation calls next and subsequent invocations reuse the first response.
                using var communicator = new Communicator(
                    prx.Communicator.GetProperties(),
                    invocationInterceptors: new InvocationInterceptor[]
                    {
                        (target, request, next, cancel) =>
                        {
                            if (ice2)
                            {
                                request.ContextOverride["interceptor-1"] = "interceptor-1";
                            }
                            return next(target, request, cancel);
                        },
                        async (target, request, next, cancel) =>
                        {
                            if (response == null)
                            {
                                response = await next(target, request, cancel);
                            }
                            return response;
                        },
                        (target, request, next, cancel) =>
                        {
                            invocations++;
                            TestHelper.Assert(response == null);
                            return next(target, request, cancel);
                        }
                    });

                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                prx1.Op1(new Dictionary<string, string> { { "local-user", "10" } });
                prx1.Op1(new Dictionary<string, string> { { "local-user", "11" } });
                prx1.Op1(new Dictionary<string, string> { { "local-user", "12" } });
                TestHelper.Assert(invocations == 1);
            }

            {
                // throwing from an interceptor stops the interceptor chain
                using var communicator = new Communicator(
                    prx.Communicator.GetProperties(),
                    invocationInterceptors: new InvocationInterceptor[]
                    {
                        (target, request, next, cancel) =>
                        {
                            request.ContextOverride["interceptor-1"] = "interceptor-1";
                            return next(target, request, cancel);
                        },
                        (target, request, next, cancel) =>
                        {
                            TestHelper.Assert(request.Context["interceptor-1"] == "interceptor-1");
                            throw new InvalidOperationException("stop interceptor chain");
                        },
                        (target, request, next, cancel) =>
                        {
                            TestHelper.Assert(false);
                            return next(target, request, cancel);
                        }
                    });

                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                try
                {
                    prx1.Op1();
                    TestHelper.Assert(false);
                }
                catch (InvalidOperationException)
                {
                }
            }
            output.WriteLine("ok");

            output.Write("testing binary context... ");
            output.Flush();

            if (ice2)
            {
                for (int size = 128; size < 4096; size *= 2)
                {
                    var token = new Token(1, "mytoken", Enumerable.Range(0, size).Select(i => (byte)2).ToArray());
                    var request = OutgoingRequestFrame.WithArgs(prx,
                                                                "opWithBinaryContext",
                                                                idempotent: false,
                                                                compress: false,
                                                                format: default,
                                                                context: null,
                                                                token,
                                                                Token.IceWriter);
                    request.AddBinaryContextEntry(1, token, Token.IceWriter);
                    request.AddBinaryContextEntry(3, (short)size, (ostr, value) => ostr.WriteShort(value));
                    request.AddBinaryContextEntry(
                        2,
                        Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                        (ostr, seq) => ostr.WriteSequence(seq, (ostr, s) => ostr.WriteString(s)));

                    // Adding the same key twice throws ArgumentException
                    try
                    {
                        request.AddBinaryContextEntry(1, token, Token.IceWriter);
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                    prx.InvokeAsync(request).Wait();

                    TestHelper.Assert(request.IsSealed);
                    // Adding to a sealed frame throws InvalidOperationException
                    try
                    {
                        request.AddBinaryContextEntry(10, token, Token.IceWriter);
                        TestHelper.Assert(false);
                    }
                    catch (InvalidOperationException)
                    {
                    }

                    // repeat with compressed frame
                    request = OutgoingRequestFrame.WithArgs(prx,
                                                            "opWithBinaryContext",
                                                            idempotent: false,
                                                            compress: false,
                                                            format: default,
                                                            context: null,
                                                            token,
                                                            Token.IceWriter);
                    request.AddBinaryContextEntry(1, token, Token.IceWriter);
                    request.AddBinaryContextEntry(3, (short)size, (ostr, value) => ostr.WriteShort(value));
                    request.AddBinaryContextEntry(
                        2,
                        Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                        (ostr, seq) => ostr.WriteSequence(seq, (ostr, s) => ostr.WriteString(s)));
                    TestHelper.Assert(request.CompressPayload() == CompressionResult.Success);
                    prx.InvokeAsync(request).Wait();

                    // repeat compressed the frame before writing the context
                    request = OutgoingRequestFrame.WithArgs(prx,
                                                            "opWithBinaryContext",
                                                            idempotent: false,
                                                            compress: false,
                                                            format: default,
                                                            context: null,
                                                            token,
                                                            Token.IceWriter);

                    TestHelper.Assert(request.CompressPayload() == CompressionResult.Success);
                    request.AddBinaryContextEntry(1, token, Token.IceWriter);
                    request.AddBinaryContextEntry(3, (short)size, (ostr, value) => ostr.WriteShort(value));
                    request.AddBinaryContextEntry(
                        2,
                        Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                        (ostr, seq) => ostr.WriteSequence(seq, (ostr, s) => ostr.WriteString(s)));
                    prx.InvokeAsync(request).Wait();
                }
            }
            else
            {
                var token = new Token(1, "mytoken", Enumerable.Range(0, 256).Select(i => (byte)2).ToArray());
                var request = OutgoingRequestFrame.WithArgs(prx,
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
                    TestHelper.Assert(false);
                }
                catch (NotSupportedException)
                {
                }
                prx.InvokeAsync(request).Wait();
                TestHelper.Assert(request.IsSealed);
            }
            output.WriteLine("ok");
            return prx;
        }
    }
}
