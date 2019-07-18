//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

namespace Ice
{
    namespace interceptor
    {
        class InterceptorI : Ice.DispatchInterceptor
        {
            internal InterceptorI(Ice.Object servant)
            {
                servant_ = servant;
            }

            protected static void
            test(bool b)
            {
                if(!b)
                {
                    throw new System.Exception();
                }
            }

            public override Task<Ice.OutputStream>
            dispatch(Ice.Request request)
            {
                Ice.Current current = request.getCurrent();

                string context;
                if(current.ctx.TryGetValue("raiseBeforeDispatch", out context))
                {
                    if(context.Equals("user"))
                    {
                        throw new Test.InvalidInputException();
                    }
                    else if(context.Equals("notExist"))
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    else if(context.Equals("system"))
                    {
                        throw new MySystemException();
                    }
                }

                lastOperation_ = current.operation;

                if(lastOperation_.Equals("addWithRetry") || lastOperation_.Equals("amdAddWithRetry"))
                {
                    for(int i = 0; i < 10; ++i)
                    {
                        try
                        {
                            var t = servant_.ice_dispatch(request);
                            if(t != null && t.IsFaulted)
                            {
                                throw t.Exception.InnerException;
                            }
                            else
                            {
                                test(false);
                            }
                        }
                        catch(Test.RetryException)
                        {
                            //
                            // Expected, retry
                            //
                        }
                    }

                    current.ctx["retry"] = "no";
                }
                else if(current.ctx.TryGetValue("retry", out context) && context.Equals("yes"))
                {
                    //
                    // Retry the dispatch to ensure that abandoning the result of the dispatch
                    // works fine and is thread-safe
                    //
                    servant_.ice_dispatch(request);
                    servant_.ice_dispatch(request);
                }

                var task = servant_.ice_dispatch(request);
                lastStatus_ = task != null;

                if(current.ctx.TryGetValue("raiseAfterDispatch", out context))
                {
                    if(context.Equals("user"))
                    {
                        throw new Test.InvalidInputException();
                    }
                    else if(context.Equals("notExist"))
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    else if(context.Equals("system"))
                    {
                        throw new MySystemException();
                    }
                }

                return task;
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

            protected readonly Ice.Object servant_;
            protected string lastOperation_;
            protected bool lastStatus_ = false;
        }
    }
}
