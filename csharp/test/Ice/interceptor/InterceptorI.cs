//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace Ice.interceptor
{
    internal sealed class Interceptor : IObject
    {
        internal Interceptor(IObject servant) => _servant = servant;

        public async ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
        {
            if (current.Context.TryGetValue("raiseBeforeDispatch", out string? context))
            {
                if (context.Equals("invalidInput"))
                {
                    throw new Test.InvalidInputException("intercept");
                }
                else if (context.Equals("notExist"))
                {
                    throw new ObjectNotExistException(current);
                }
            }

            _lastOperation = current.Operation;

            if (_lastOperation.Equals("addWithRetry") || _lastOperation.Equals("amdAddWithRetry"))
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
                    throw new Test.InvalidInputException("raiseAfterDispatch");
                }
                else if (context.Equals("notExist"))
                {
                    throw new ObjectNotExistException(current);
                }
            }

            return await vt.ConfigureAwait(false);
        }

        internal bool AsyncCompletion { get ; private set; } = false;

        internal string? getLastOperation() => _lastOperation;

        internal void
        clear()
        {
            _lastOperation = null;
            AsyncCompletion = false;
        }

        private readonly IObject _servant;
        private string? _lastOperation;
    }
}
