//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

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
                throw new System.Exception();
            }
        }

        public async ValueTask<Ice.OutputStream> DispatchAsync(Ice.InputStream istr, Current current)
        {
            string context;
            if (current.Context.TryGetValue("raiseBeforeDispatch", out context))
            {
                if (context.Equals("user"))
                {
                    throw new Test.InvalidInputException();
                }
                else if (context.Equals("notExist"))
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if (context.Equals("system"))
                {
                    throw new MySystemException();
                }
            }

            _lastOperation = current.Operation;

            if (_lastOperation.Equals("addWithRetry") || _lastOperation.Equals("amdAddWithRetry"))
            {
                for (int i = 0; i < 10; ++i)
                {
                    try
                    {
                        var ostr = await _servant.DispatchAsync(istr, current).ConfigureAwait(false);
                        test(false);
                    }
                    catch (RetryException)
                    {
                        istr.RestartEncapsulation();
                        //
                        // Expected, retry
                        //
                    }
                }

                current.Context["retry"] = "no";
            }
            else if (current.Context.TryGetValue("retry", out context) && context.Equals("yes"))
            {
                //
                // Retry the dispatch to ensure that abandoning the result of the dispatch
                // works fine and is thread-safe
                //
                _servant.DispatchAsync(istr, current);
                istr.RestartEncapsulation();
                _servant.DispatchAsync(istr, current);
            }

            istr.RestartEncapsulation();
            var vt = _servant.DispatchAsync(istr, current);

            AsyncCompletion = !vt.IsCompleted;

            if (current.Context.TryGetValue("raiseAfterDispatch", out context))
            {
                if (context.Equals("user"))
                {
                    throw new Test.InvalidInputException();
                }
                else if (context.Equals("notExist"))
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if (context.Equals("system"))
                {
                    throw new MySystemException();
                }
            }

            return await vt.ConfigureAwait(false);
        }

        internal bool AsyncCompletion { get ; private set; } = false;

        internal string getLastOperation() => _lastOperation;

        internal void
        clear()
        {
            _lastOperation = null;
            AsyncCompletion = false;
        }

        private readonly IObject _servant;
        private string _lastOperation;
    }
}
