//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice.interceptor
{
    internal sealed class Interceptor : Ice.IObject
    {
        internal Interceptor(IObject servant)
        {
            _servant = servant;
        }

        private static void
        test(bool b)
        {
            if (!b)
            {
                System.Diagnostics.Debug.Assert(false);
                throw new System.Exception();
            }
        }

        public async ValueTask<Ice.OutgoingResponseFrame> DispatchAsync(Ice.IncomingRequestFrame request, Current current)
        {
            if (current.Context.TryGetValue("raiseBeforeDispatch", out var context))
            {
                if (context.Equals("invalidInput"))
                {
                    throw new Test.InvalidInputException();
                }
                else if (context.Equals("notExist"))
                {
                    throw new Ice.ObjectNotExistException();
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
                        test(false);
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
                var vt1 = _servant.DispatchAsync(request, current);
                var vt2 = _servant.DispatchAsync(request, current);
                await vt1.ConfigureAwait(false);
                await vt2.ConfigureAwait(false);
            }

            var vt = _servant.DispatchAsync(request, current);

            AsyncCompletion = !vt.IsCompleted;

            if (current.Context.TryGetValue("raiseAfterDispatch", out context))
            {
                if (context.Equals("invalidInput"))
                {
                    throw new Test.InvalidInputException();
                }
                else if (context.Equals("notExist"))
                {
                    throw new Ice.ObjectNotExistException();
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
