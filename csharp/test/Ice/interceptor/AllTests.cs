//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Linq;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class AllTests
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
                using var communicator = new Communicator(
                    prx.Communicator.GetProperties(),
                    invocationInterceptors: new InvocationInterceptor[]
                    {
                        (target, request, next) =>
                        {
                            if (ice2)
                            {
                                request.ContextOverride["interceptor-1"] = "interceptor-1";
                                request.AddBinaryContextEntry(110, 110, (ostr, v) => ostr.WriteInt(v));
                            }
                            return next(target, request);
                        },
                        async (target, request, next) =>
                        {
                            if (ice2)
                            {
                                TestHelper.Assert(request.Context["interceptor-1"] == "interceptor-1");
                                request.ContextOverride["interceptor-2"] = "interceptor-2";
                                request.AddBinaryContextEntry(120, 120, (ostr, v) => ostr.WriteInt(v));
                            }
                            IncomingResponseFrame response = await next(target, request);
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
                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                prx1.Op1();
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
                        (target, request, next) =>
                        {
                            if (ice2)
                            {
                                request.ContextOverride["interceptor-1"] = "interceptor-1";
                            }
                            return next(target, request);
                        },
                        async (target, request, next) =>
                        {
                            if (response == null)
                            {
                                response = await next(target, request);
                            }
                            return response;
                        },
                        (target, request, next) =>
                        {
                            invocations++;
                            TestHelper.Assert(response == null);
                            return next(target, request);
                        }
                    });

                var prx1 = IMyObjectPrx.Parse(prx.ToString()!, communicator);
                prx1.Op1();
                prx1.Op1();
                prx1.Op1();
                TestHelper.Assert(invocations == 1);
            }

            {
                // throwing from an interceptor stops the interceptor chain
                using var communicator = new Communicator(
                    prx.Communicator.GetProperties(),
                    invocationInterceptors: new InvocationInterceptor[]
                    {
                        (target, request, next) =>
                        {
                            request.ContextOverride["interceptor-1"] = "interceptor-1";
                            return next(target, request);
                        },
                        (target, request, next) =>
                        {
                            TestHelper.Assert(request.Context["interceptor-1"] == "interceptor-1");
                            throw new InvalidOperationException("stop interceptor chain");
                        },
                        (target, request, next) =>
                        {
                            TestHelper.Assert(false);
                            return next(target, request);
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
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                    prx.Invoke(request);

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
                    TestHelper.Assert(request.CompressPayload() == CompressionResult.Success);
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

                    TestHelper.Assert(request.CompressPayload() == CompressionResult.Success);
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
                    TestHelper.Assert(false);
                }
                catch (NotSupportedException)
                {
                }
                prx.Invoke(request);
                TestHelper.Assert(request.IsSealed);
            }
            output.WriteLine("ok");
            return prx;
        }
    }
}
