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

        public  Task<Ice.OutputStream>?
        Dispatch(IceInternal.Incoming incoming, Current current)
        {
            try
            {
                incoming.StartOver();
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
                            var t = _servant.Dispatch(incoming, current);
                            if (t != null && t.IsFaulted)
                            {
                                throw t.Exception.InnerException;
                            }
                            else
                            {
                                test(false);
                            }
                        }
                        catch (RetryException)
                        {
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
                    _servant.Dispatch(incoming, current);
                    _servant.Dispatch(incoming, current);
                }

                var task = _servant.Dispatch(incoming, current);
                _lastStatus = task != null;

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

                return task;
            }
            catch (Exception)
            {
                //
                // If the input parameters weren't read, make sure we skip them here. It's needed to read the
                // encoding version used by the client to eventually marshal the user exception. It's also needed
                // if we dispatch a batch oneway request to read the next batch request.
                //
                if (current.Encoding == null || (current.Encoding.Major == 0 && current.Encoding.Minor == 0))
                {
                    incoming.SkipReadParams();
                }
                throw;
            }
        }

        internal bool getLastStatus() => _lastStatus;

        internal string getLastOperation() => _lastOperation;

        internal void
        clear()
        {
            _lastOperation = null;
            _lastStatus = false;
        }

        private readonly IObject _servant;
        private string _lastOperation;
        private bool _lastStatus = false;
    }
}
