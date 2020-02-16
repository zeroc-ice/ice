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

        public ValueTask<Ice.OutputStream>? Dispatch(Ice.InputStream istr, Current current)
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
                        var vt = _servant.Dispatch(istr, current);
                        if (vt.HasValue && vt.Value.IsFaulted)
                        {
                            throw vt.Value.AsTask().Exception.InnerException;
                        }
                        else
                        {
                            test(false);
                        }
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
                _servant.Dispatch(istr, current);
                istr.RestartEncapsulation();
                _servant.Dispatch(istr, current);
            }

            istr.RestartEncapsulation();
            var valueTask = _servant.Dispatch(istr, current);
            if (valueTask == null)
            {
                AsyncCompletion = false;
            }
            else
            {
                AsyncCompletion = !valueTask.Value.IsCompleted;
            }

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

            return valueTask;
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
