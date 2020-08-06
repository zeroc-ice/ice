//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

            if (_lastOperation == "OpWithBinaryContext")
            {
                Debug.Assert(request.BinaryContext.ContainsKey(1));
                Token t1 = request.BinaryContext[1].Read(Token.IceReader);
                Token t2 = request.ReadParamList(current.Communicator, Token.IceReader);
                TestHelper.Assert(t1 == t2);
                Debug.Assert(request.BinaryContext.ContainsKey(2));
                string[] s2 = request.BinaryContext[2].Read(Ice.StringSeqHelper.IceReader);
                Enumerable.Range(0, 10).Select(i => $"string-{i}").SequenceEqual(s2);
                Debug.Assert(request.BinaryContext.ContainsKey(3));
                TestHelper.Assert(1024 == request.BinaryContext[3].Read(istr => istr.ReadShort()));
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
