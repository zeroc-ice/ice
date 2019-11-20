//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

namespace Ice
{
    namespace interceptor
    {
        class InterceptorI<T, Traits> where Traits : struct, IInterfaceTraits<T>
        {
            internal InterceptorI(T servant)
            {
                servant_ = servant;
                traits_ = default;
            }

            protected static void
            test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

            public Task<Ice.OutputStream>
            Dispatch(IceInternal.Incoming incoming, Current current)
            {
                try
                {
                    incoming.startOver();
                    string context;
                    if (current.ctx.TryGetValue("raiseBeforeDispatch", out context))
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

                    lastOperation_ = current.operation;

                    if (lastOperation_.Equals("addWithRetry") || lastOperation_.Equals("amdAddWithRetry"))
                    {
                        for (int i = 0; i < 10; ++i)
                        {
                            try
                            {
                                var t = traits_.Dispatch(servant_, incoming, current);
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

                        current.ctx["retry"] = "no";
                    }
                    else if (current.ctx.TryGetValue("retry", out context) && context.Equals("yes"))
                    {
                        //
                        // Retry the dispatch to ensure that abandoning the result of the dispatch
                        // works fine and is thread-safe
                        //
                        traits_.Dispatch(servant_, incoming, current);
                        traits_.Dispatch(servant_, incoming, current);
                    }

                    var task = traits_.Dispatch(servant_, incoming, current);
                    lastStatus_ = task != null;

                    if (current.ctx.TryGetValue("raiseAfterDispatch", out context))
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
                    if (current.encoding == null || (current.encoding.major == 0 && current.encoding.minor == 0))
                    {
                        incoming.skipReadParams();
                    }
                    throw;
                }
            }

            internal bool
            getLastStatus()
            {
                return lastStatus_;
            }

            internal String
            getLastOperation()
            {
                return lastOperation_;
            }

            internal virtual void
            clear()
            {
                lastOperation_ = null;
                lastStatus_ = false;
            }

            protected readonly T servant_;
            protected readonly Traits traits_;
            protected string lastOperation_;
            protected bool lastStatus_ = false;
        }
    }
}
