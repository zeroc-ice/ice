//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    internal sealed class Interceptor : IObject
    {
        private string? _lastOperation;
        private readonly IObject _servant;

        internal Interceptor(IObject servant) => _servant = servant;

        public async ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
        {
            if (current.Operation == "op1")
            {
                TestHelper.Assert(request.Context["interceptor-1"] == "interceptor-1");
                TestHelper.Assert(request.Context["interceptor-2"] == "interceptor-2");

                TestHelper.Assert(request.BinaryContext.ContainsKey(110));
                TestHelper.Assert(request.BinaryContext[110].Read(istr => istr.ReadInt()) == 110);
                TestHelper.Assert(request.BinaryContext.ContainsKey(120));
                TestHelper.Assert(request.BinaryContext[120].Read(istr => istr.ReadInt()) == 120);

                var response = OutgoingResponseFrame.WithVoidReturnValue(current);
                response.AddBinaryContextEntry(110, 110, (ostr, value) => ostr.WriteInt(value));
                response.AddBinaryContextEntry(120, 120, (ostr, value) => ostr.WriteInt(value));
                return response;
            }

            if (current.Context.TryGetValue("raiseBeforeDispatch", out string? context))
            {
                if (context.Equals("invalidInput"))
                {
                    throw new InvalidInputException("intercept");
                }
                else if (context.Equals("notExist"))
                {
                    throw new ObjectNotExistException(current);
                }
            }

            _lastOperation = current.Operation;

            if (_lastOperation == "opWithBinaryContext")
            {
                if (request.Protocol == Protocol.Ice2)
                {
                    Debug.Assert(request.BinaryContext.ContainsKey(3));
                    short size = request.BinaryContext[3].Read(istr => istr.ReadShort());
                    var t2 = new Token(1, "mytoken", Enumerable.Range(0, size).Select(i => (byte)2).ToArray());
                    Debug.Assert(request.BinaryContext.ContainsKey(1));
                    Token t1 = request.BinaryContext[1].Read(Token.IceReader);
                    TestHelper.Assert(t1.Hash == t2.Hash);
                    TestHelper.Assert(t1.Expiration == t2.Expiration);
                    TestHelper.Assert(t1.Payload.SequenceEqual(t2.Payload));
                    Debug.Assert(request.BinaryContext.ContainsKey(2));
                    string[] s2 = request.BinaryContext[2].Read(Ice.StringSeqHelper.IceReader);
                    Enumerable.Range(0, 10).Select(i => $"string-{i}").SequenceEqual(s2);

                    if (request.HasCompressedPayload)
                    {
                        request.DecompressPayload();

                        Debug.Assert(request.BinaryContext.ContainsKey(3));
                        size = request.BinaryContext[3].Read(istr => istr.ReadShort());

                        Debug.Assert(request.BinaryContext.ContainsKey(1));
                        t1 = request.BinaryContext[1].Read(Token.IceReader);
                        t2 = request.ReadParamList(current.Communicator, Token.IceReader);
                        TestHelper.Assert(t1.Hash == t2.Hash);
                        TestHelper.Assert(t1.Expiration == t2.Expiration);
                        TestHelper.Assert(t1.Payload.SequenceEqual(t2.Payload));
                        Debug.Assert(request.BinaryContext.ContainsKey(2));
                        s2 = request.BinaryContext[2].Read(Ice.StringSeqHelper.IceReader);
                        Enumerable.Range(0, 10).Select(i => $"string-{i}").SequenceEqual(s2);
                    }
                }
                else
                {
                    try
                    {
                        _ = request.BinaryContext;
                        TestHelper.Assert(false);
                    }
                    catch (NotSupportedException)
                    {
                    }
                }
            }
            else if (_lastOperation.Equals("addWithRetry") || _lastOperation.Equals("amdAddWithRetry"))
            {
                for (int i = 0; i < 10; ++i)
                {
                    try
                    {
                        await _servant.DispatchAsync(request, current).ConfigureAwait(false);
                        TestHelper.Assert(false);
                    }
                    catch (RetryException)
                    {
                        // Expected, retry
                    }
                }
                current.Context["retry"] = "no";
            }
            else if (current.Context.TryGetValue("retry", out context) && context.Equals("yes"))
            {
                // Retry the dispatch to ensure that abandoning the result of the dispatch
                // works fine and is thread-safe
                ValueTask<OutgoingResponseFrame> vt1 = _servant.DispatchAsync(request, current);
                ValueTask<OutgoingResponseFrame> vt2 = _servant.DispatchAsync(request, current);
                await vt1.ConfigureAwait(false);
                await vt2.ConfigureAwait(false);
            }

            ValueTask<OutgoingResponseFrame> vt = _servant.DispatchAsync(request, current);

            AsyncCompletion = !vt.IsCompleted;

            if (current.Context.TryGetValue("raiseAfterDispatch", out context))
            {
                if (context.Equals("invalidInput"))
                {
                    throw new InvalidInputException("raiseAfterDispatch");
                }
                else if (context.Equals("notExist"))
                {
                    throw new ObjectNotExistException(current);
                }
            }

            return await vt.ConfigureAwait(false);
        }

        internal bool AsyncCompletion { get ; private set; }

        internal string? GetLastOperation() => _lastOperation;

        internal void Clear()
        {
            _lastOperation = null;
            AsyncCompletion = false;
        }
    }
}
