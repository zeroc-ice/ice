// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
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
                await using var communicator = new Communicator(prx.Communicator.GetProperties());

                communicator.DefaultInvocationInterceptors = ImmutableList.Create<InvocationInterceptor>(
                    (target, request, next, cancel) =>
                    {
                        request.WritableContext["interceptor-1"] = "interceptor-1";
                        if (ice2)
                        {
                            request.BinaryContextOverride.Add(110, ostr => ostr.WriteInt(110));
                        }
                        return next(target, request, cancel);
                    },
                    async (target, request, next, cancel) =>
                    {
                        TestHelper.Assert(request.Context["interceptor-1"] == "interceptor-1");
                        request.WritableContext["interceptor-2"] = "interceptor-2";
                        if (ice2)
                        {
                            request.BinaryContextOverride.Add(120, ostr => ostr.WriteInt(120));
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
                    });

                await communicator.ActivateAsync();

                for (int i = 0; i < 10; ++i)
                {
                    invocationContext.Value = i;
                    var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                    Task t = prx1.Op1Async(new Dictionary<string, string> { { "local-user", $"{i}" } });
                    tasks.Add(t);
                }
                Task.WaitAll(tasks.ToArray());
            }

            {
                IncomingResponseFrame? response = null;
                int invocations = 0;
                // An interceptor can stop the chain and directly return a response without calling next,
                // the first invocation calls next and subsequent invocations reuse the first response.
                await using var communicator = new Communicator(prx.Communicator.GetProperties());

                communicator.DefaultInvocationInterceptors = ImmutableList.Create<InvocationInterceptor>(
                    (target, request, next, cancel) =>
                    {
                        request.WritableContext["interceptor-1"] = "interceptor-1";
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
                    });

                TestHelper.Assert(communicator.DefaultInvocationInterceptors.Count == 3);
                await communicator.ActivateAsync();
                TestHelper.Assert(communicator.DefaultInvocationInterceptors.Count == 4);

                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);

                // Truly reference equal
                TestHelper.Assert(prx1.InvocationInterceptors == communicator.DefaultInvocationInterceptors);

                prx1.Op1(new Dictionary<string, string> { { "local-user", "10" } });
                prx1.Op1(new Dictionary<string, string> { { "local-user", "11" } });
                prx1.Op1(new Dictionary<string, string> { { "local-user", "12" } });
                TestHelper.Assert(invocations == 1);
            }

            {
                // throwing from an interceptor stops the interceptor chain
                await using var communicator = new Communicator(prx.Communicator.GetProperties());
                communicator.DefaultInvocationInterceptors = ImmutableList.Create<InvocationInterceptor>(
                    (target, request, next, cancel) =>
                    {
                        request.WritableContext["interceptor-1"] = "interceptor-1";
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
                    });
                await communicator.ActivateAsync();

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

                    {
                        using var request = OutgoingRequestFrame.WithArgs(prx,
                                                                        "opWithBinaryContext",
                                                                        idempotent: false,
                                                                        compress: false,
                                                                        format: default,
                                                                        context: null,
                                                                        token,
                                                                        Token.IceWriter);
                        request.BinaryContextOverride.Add(1, ostr => ostr.WriteStruct(token));
                        request.BinaryContextOverride.Add(3, ostr => ostr.WriteShort((short)size));
                        request.BinaryContextOverride.Add(
                            2,
                            ostr => ostr.WriteSequence(
                                Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                                (ostr, s) => ostr.WriteString(s)));

                        using IncomingResponseFrame response = prx.InvokeAsync(request).Result;
                    }

                    {
                        // repeat with compressed frame
                        using OutgoingRequestFrame request = OutgoingRequestFrame.WithArgs(prx,
                                                                                           "opWithBinaryContext",
                                                                                           idempotent: false,
                                                                                           compress: false,
                                                                                           format: default,
                                                                                           context: null,
                                                                                           token,
                                                                                           Token.IceWriter);
                        request.BinaryContextOverride.Add(1, ostr => ostr.WriteStruct(token));
                        request.BinaryContextOverride.Add(3, ostr => ostr.WriteShort((short)size));
                        request.BinaryContextOverride.Add(
                            2,
                            ostr => ostr.WriteSequence(
                                Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                                (ostr, s) => ostr.WriteString(s)));

                        TestHelper.Assert(request.CompressPayload() == CompressionResult.Success);
                        using IncomingResponseFrame response = prx.InvokeAsync(request).Result;
                    }

                    {
                        // repeat compressed the frame before writing the context
                        using OutgoingRequestFrame request = OutgoingRequestFrame.WithArgs(prx,
                                                                                           "opWithBinaryContext",
                                                                                           idempotent: false,
                                                                                           compress: false,
                                                                                           format: default,
                                                                                           context: null,
                                                                                           token,
                                                                                           Token.IceWriter);

                        TestHelper.Assert(request.CompressPayload() == CompressionResult.Success);

                        request.BinaryContextOverride.Add(1, ostr => ostr.WriteStruct(token));
                        request.BinaryContextOverride.Add(3, ostr => ostr.WriteShort((short)size));
                        request.BinaryContextOverride.Add(
                            2,
                            ostr => ostr.WriteSequence(
                                Enumerable.Range(0, 10).Select(i => $"string-{i}").ToArray(),
                                (ostr, s) => ostr.WriteString(s)));

                        using IncomingResponseFrame response = prx.InvokeAsync(request).Result;
                    }
                }
            }
            else
            {
                var token = new Token(1, "mytoken", Enumerable.Range(0, 256).Select(i => (byte)2).ToArray());
                using var request = OutgoingRequestFrame.WithArgs(prx,
                                                                  "opWithBinaryContext",
                                                                  idempotent: false,
                                                                  compress: false,
                                                                  format: default,
                                                                  context: null,
                                                                  token,
                                                                  Token.IceWriter);
                try
                {
                    request.BinaryContextOverride.Add(1, ostr => ostr.WriteStruct(token));
                    TestHelper.Assert(false);
                }
                catch (NotSupportedException)
                {
                }
                using IncomingResponseFrame response = prx.InvokeAsync(request).Result;
            }
            output.WriteLine("ok");

            output.Write("testing per proxy invocation interceptors... ");
            output.Flush();
            {
                var communicator = helper.Communicator!;

                SortedDictionary<string, string>? context = prx.Op2();
                TestHelper.Assert(context["context1"] == "plug-in");
                TestHelper.Assert(context["context2"] == "plug-in");
                TestHelper.Assert(!context.ContainsKey("context3"));
                prx = prx.Clone(invocationInterceptors: ImmutableList.Create<InvocationInterceptor>(
                    (target, request, next, cancel) =>
                    {
                        request.WritableContext["context2"] = "proxy";
                        request.WritableContext["context3"] = "proxy";
                        return next(target, request, cancel);
                    }).AddRange(communicator.DefaultInvocationInterceptors));
                context = prx.Op2();
                TestHelper.Assert(context["context1"] == "plug-in");
                TestHelper.Assert(context["context2"] == "plug-in");
                TestHelper.Assert(context["context3"] == "proxy");

                // Calling next twice doesn't change the result
                prx = prx.Clone(invocationInterceptors: ImmutableList.Create<InvocationInterceptor>(
                    (target, request, next, cancel) =>
                    {
                        request.WritableContext["context2"] = "proxy";
                        request.WritableContext["context3"] = "proxy";
                        _ = next(target, request, cancel);
                        return next(target, request, cancel);
                    }).AddRange(communicator.DefaultInvocationInterceptors));
                context = prx.Op2();
                TestHelper.Assert(context["context1"] == "plug-in");
                TestHelper.Assert(context["context2"] == "plug-in");
                TestHelper.Assert(context["context3"] == "proxy");

                // Cloning the proxy preserve its interceptors
                prx = prx.Clone(invocationTimeout: TimeSpan.FromSeconds(10));
                context = prx.Op2();
                TestHelper.Assert(context["context1"] == "plug-in");
                TestHelper.Assert(context["context2"] == "plug-in");
                TestHelper.Assert(context["context3"] == "proxy");

                // The server increments the result with each call when using the invocation interceptor we
                // return a cached response, and we will see the same result with each call.
                IncomingResponseFrame? response = null;
                prx = prx.Clone(invocationInterceptors: ImmutableList.Create<InvocationInterceptor>(
                   async (target, request, next, cancel) =>
                   {
                       response ??= await next(target, request, cancel);
                       return response;
                   }).AddRange(communicator.DefaultInvocationInterceptors));
                TestHelper.Assert(prx.Op3() == 0);
                TestHelper.Assert(prx.Op3() == 0);

                // After clearing the invocation interceptors we should see the result increase with each call
                prx = prx.Clone(invocationInterceptors: communicator.DefaultInvocationInterceptors);
                TestHelper.Assert(prx.Op3() == 1);
                TestHelper.Assert(prx.Op3() == 2);
            }
            output.WriteLine("ok");
            await prx.ShutdownAsync();
        }
    }
}
