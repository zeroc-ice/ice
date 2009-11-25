// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

import java.io.PrintWriter;

import test.Ice.ami.Test.TestIntfPrx;
import test.Ice.ami.Test.TestIntfPrxHelper;
import test.Ice.ami.Test.TestIntfControllerPrx;
import test.Ice.ami.Test.TestIntfControllerPrxHelper;
import test.Ice.ami.Test.TestIntfException;
import test.Ice.ami.Test.Callback_TestIntf_op;
import test.Ice.ami.Test.Callback_TestIntf_opWithResult;
import test.Ice.ami.Test.Callback_TestIntf_opWithUE;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class CallbackBase
    {
        CallbackBase()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    static class AsyncCallback extends CallbackBase
    {
        public AsyncCallback()
        {
        }    

        public void
        isA(Ice.AsyncResult result)
        {
            test(result.getProxy().end_ice_isA(result));
            called();
        }

        public void 
        ping(Ice.AsyncResult result)
        {
            result.getProxy().end_ice_ping(result);
            called();
        }

        public void 
        id(Ice.AsyncResult result)
        {
            test(result.getProxy().end_ice_id(result).equals("::Test::TestIntf"));
            called();
        }

        public void
        ids(Ice.AsyncResult result)
        {
            test(result.getProxy().end_ice_ids(result).length == 2);
            called();
        }
        
        public void
        op(Ice.AsyncResult result)
        {
            TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_op(result);
            called();
        }

        public void
        opWithResult(Ice.AsyncResult result)
        {
            test(TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_opWithResult(result) == 15);
            called();
        }
        
        public void
        opWithUE(Ice.AsyncResult result)
        {
            try
            {
                TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_opWithUE(result);
                test(false);
            }
            catch(TestIntfException ex)
            {
                called();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }

        public void
        isAEx(Ice.AsyncResult result)
        {
            try
            {
                result.getProxy().end_ice_isA(result);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
                called();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }
        
        public void
        pingEx(Ice.AsyncResult result)
        {
            try
            {
                result.getProxy().end_ice_ping(result);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
                called();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }

        public void
        idEx(Ice.AsyncResult result)
        {
            try
            {
                result.getProxy().end_ice_id(result);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
                called();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }

        public void
        idsEx(Ice.AsyncResult result)
        {
            try
            {
                result.getProxy().end_ice_ids(result);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
                called();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }

        public void
        opEx(Ice.AsyncResult result)
        {
            try
            {
                TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_op(result);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
                called();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }

    };

    static class ResponseCallback extends CallbackBase
    {
        ResponseCallback()
        {
        }
    
        public void
        isA(boolean r)
        {
            test(r);
            called();
        }
        
        public void
        ping()
        {
            called();
        }

        public void
        id(String id)
        {
            test(id.equals("::Test::TestIntf"));
            called();
        }

        public void
        ids(String[] ids)
        {
            test(ids.length == 2);
            called();
        }

        public void
        op()
        {
            called();
        }
        
        public void
        opWithResult(int r)
        {
            test(r == 15);
            called();
        }
        
        public void
        opWithUE(Ice.UserException e)
        {
            try
            {
                throw e;
            }
            catch(TestIntfException ex)
            {
                called();
            }
            catch(Ice.UserException ex)
            {
                test(false);
            }
        }
            
    };

    static class ExceptionCallback extends CallbackBase
    {
        public ExceptionCallback()
        {
        }
    
        public void
        isA(boolean r)
        {
            test(false);
        }
        
        public void
        ping()
        {
            test(false);
        }
        
        public void
        id(String id)
        {
            test(false);
        }
        
        public void
        ids(String[] ids)
        {
            test(false);
        }
        
        public void
        op()
        {
            test(false);
        }
        
        public void
        ex(Ice.LocalException ex)
        {
            test(ex instanceof Ice.NoEndpointException);
            called();
        }

        public void
        noEx(Ice.LocalException ex)
        {
            test(false);
        }
    };

    static class SentCallback extends CallbackBase
    {
        SentCallback()
        {
            _thread = Thread.currentThread().getId();
        }

        public void
        isA(boolean r)
        {
        }

        public void
        ping()
        {
        }

        public void
        id(String s) 
        {
        }

        public void
        ids(String[] s)
        {
        }

        public void
        opAsync(Ice.AsyncResult r)
        {
        }

        public void
        op()
        {
        }

        public void
        ex(Ice.LocalException ex)
        {
        }

        public void
        sent(Ice.AsyncResult r)
        {
            test(r.sentSynchronously() && _thread == Thread.currentThread().getId() ||
                 !r.sentSynchronously() && _thread != Thread.currentThread().getId());
            called();
        }

        public void
        sent(boolean ss)
        {
            test(ss && _thread == Thread.currentThread().getId() ||
                 !ss && _thread != Thread.currentThread().getId());
            called();
        }
        
        long _thread;
    };

    enum ThrowType { LocalException, OtherException };

    static class Thrower extends CallbackBase
    {
        public Thrower(ThrowType t)
        {
            _t = t;
        }

        public void
        opAsync(Ice.AsyncResult r)
        {
            called();
            throwEx();
        }

        public void
        op()
        {
            called();
            throwEx();
        }

        public void
        noOp()
        {
        }

        public void
        ex(Ice.LocalException ex)
        {
            called();
            throwEx();
        }

        public void
        sent(boolean ss)
        {
            called();
            throwEx();
        }

        private void
        throwEx()
        {
            switch(_t)
            {
            case LocalException:
            {
                throw new Ice.ObjectNotExistException();
            }
            case OtherException:
            {
                throw new RuntimeException();
            }
            default:
            {
                assert(false);
                break;
            }
            }
        }
        
        ThrowType _t;
    };

    public static void
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        String sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:tcp -p 12011";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrxHelper.uncheckedCast(obj);

        out.print("testing begin/end invocation... ");
        out.flush();
        {
            Ice.AsyncResult result;
            java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();

            result = p.begin_ice_isA("::Test::TestIntf");
            test(p.end_ice_isA(result));
            result = p.begin_ice_isA("::Test::TestIntf", ctx);
            test(p.end_ice_isA(result));

            result = p.begin_ice_ping();
            p.end_ice_ping(result);
            result = p.begin_ice_ping(ctx);
            p.end_ice_ping(result);
        
            result = p.begin_ice_id();
            test(p.end_ice_id(result).equals("::Test::TestIntf"));
            result = p.begin_ice_id(ctx);
            test(p.end_ice_id(result).equals("::Test::TestIntf"));

            result = p.begin_ice_ids();
            test(p.end_ice_ids(result).length == 2);
            result = p.begin_ice_ids(ctx);
            test(p.end_ice_ids(result).length == 2);

            result = p.begin_op();
            p.end_op(result);
            result = p.begin_op(ctx);
            p.end_op(result);

            result = p.begin_opWithResult();
            test(p.end_opWithResult(result) == 15);
            result = p.begin_opWithResult(ctx);
            test(p.end_opWithResult(result) == 15);

            result = p.begin_opWithUE();
            try
            {
                p.end_opWithUE(result);
                test(false);
            }
            catch(TestIntfException ex)
            {
            }
            result = p.begin_opWithUE(ctx);
            try
            {
                p.end_opWithUE(result);
                test(false);
            }
            catch(TestIntfException ex)
            {
            }
        }
        out.println("ok");

        out.print("testing async callback... ");
        out.flush();
        {
            final AsyncCallback cb = new AsyncCallback();
            java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
        
            p.begin_ice_isA("::Test::TestIntf", new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.isA(r);
                    }
                });
            cb.check();

            p.begin_ice_isA("::Test::TestIntf", ctx, new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.isA(r);
                    }
                });
            cb.check();

            p.begin_ice_ping(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ping(r);
                    }
                });
            cb.check();        
            p.begin_ice_ping(ctx, new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ping(r);
                    }
                });
            cb.check();        
        
            p.begin_ice_id(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.id(r);
                    }
                });
            cb.check();        
            p.begin_ice_id(ctx, new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.id(r);
                    }
                });
            cb.check();        

            p.begin_ice_ids(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ids(r);
                    }
                });
            cb.check();        
            p.begin_ice_ids(ctx, new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ids(r);
                    }
                });
            cb.check();        

            p.begin_op(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.op(r);
                    }
                });
            cb.check();        
            p.begin_op(ctx, new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.op(r);
                    }
                });
            cb.check();        

            p.begin_opWithResult(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opWithResult(r);
                    }
                });
            cb.check();
            p.begin_opWithResult(ctx, new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opWithResult(r);
                    }
                });
            cb.check();

            p.begin_opWithUE(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opWithUE(r);
                    }
                });
            cb.check();
            p.begin_opWithUE(ctx, new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opWithUE(r);
                    }
                });
            cb.check();
        }
        out.println("ok");

        out.print("testing response callback... ");
        out.flush();
        {
            final ResponseCallback cb = new ResponseCallback();
            java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
        
            p.begin_ice_isA("::Test::TestIntf", new Ice.Callback_Object_ice_isA()
                {
                    public void
                    response(boolean r)
                    {
                        cb.isA(r);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        
            p.begin_ice_isA("::Test::TestIntf", ctx, new Ice.Callback_Object_ice_isA()
                {
                    public void
                    response(boolean r)
                    {
                        cb.isA(r);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        

            p.begin_ice_ping(new Ice.Callback_Object_ice_ping()
                {
                    public void
                    response()
                    {
                        cb.ping();
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        
            p.begin_ice_ping(ctx, new Ice.Callback_Object_ice_ping()
                {
                    public void
                    response()
                    {
                        cb.ping();
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        
        
            p.begin_ice_id(new Ice.Callback_Object_ice_id()
                {
                    public void
                    response(String id)
                    {
                        cb.id(id);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        
            p.begin_ice_id(ctx, new Ice.Callback_Object_ice_id()
                {
                    public void
                    response(String id)
                    {
                        cb.id(id);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        

            p.begin_ice_ids(new Ice.Callback_Object_ice_ids()
                {
                    public void
                    response(String[] ids)
                    {
                        cb.ids(ids);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        
            p.begin_ice_ids(ctx, new Ice.Callback_Object_ice_ids()
                {
                    public void
                    response(String[] ids)
                    {
                        cb.ids(ids);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();        

            p.begin_op(new Callback_TestIntf_op()
                {
                    public void
                    response()
                    {
                        cb.op();
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();
            p.begin_op(ctx, new Callback_TestIntf_op()
                {
                    public void
                    response()
                    {
                        cb.op();
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();

            p.begin_opWithResult(new Callback_TestIntf_opWithResult()
                {
                    public void
                    response(int r)
                    {
                        cb.opWithResult(r);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();
            p.begin_opWithResult(ctx, new Callback_TestIntf_opWithResult()
                {
                    public void
                    response(int r)
                    {
                        cb.opWithResult(r);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();

            p.begin_opWithUE(new Callback_TestIntf_opWithUE()
                {
                    public void
                    response()
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.UserException ex)
                    {
                        cb.opWithUE(ex);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();
            p.begin_opWithUE(ctx, new Callback_TestIntf_opWithUE()
                {
                    public void
                    response()
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.UserException ex)
                    {
                        cb.opWithUE(ex);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }
                });
            cb.check();
        }
        out.println("ok");

        out.print("testing local exceptions... ");
        out.flush();
        {
            TestIntfPrx indirect = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            Ice.AsyncResult r;

            r = indirect.begin_op();
            try
            {
                indirect.end_op(r);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
            }


            r = ((TestIntfPrx)p.ice_oneway()).begin_opWithResult();
            try
            {
                TestIntfPrxHelper.uncheckedCast(r.getProxy()).end_opWithResult(r);
                test(false);
            }
            catch(Ice.TwowayOnlyException ex)
            {
            }

            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            Ice.Communicator ic = Ice.Util.initialize(initData);
            Ice.ObjectPrx o = ic.stringToProxy(p.toString());
            TestIntfPrx p2 = TestIntfPrxHelper.checkedCast(o);
            ic.destroy();
    
            try
            {
                p2.begin_op();
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
                // Expected.
            }
        }
        out.println("ok");

        out.print("testing local exceptions with async callback... ");
        out.flush();
        {
            TestIntfPrx i = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            final AsyncCallback cb = new AsyncCallback();
        
            i.begin_ice_isA("::Test::TestIntf", new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.isAEx(r);
                    }
                });
            cb.check();

            i.begin_ice_ping(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.pingEx(r);
                    }
                });
            cb.check();        
        
            i.begin_ice_id(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.idEx(r);
                    }
                });
            cb.check();        

            i.begin_ice_ids(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.idsEx(r);
                    }
                });
            cb.check();        

            i.begin_op(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opEx(r);
                    }
                });
            cb.check();        
        }
        out.println("ok");

        out.print("testing local exceptions with response callback... ");
        out.flush();
        {
            TestIntfPrx i = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            final ExceptionCallback cb = new ExceptionCallback();
        
            i.begin_ice_isA("::Test::TestIntf", new Ice.Callback_Object_ice_isA()
                {
                    public void
                    response(boolean r)
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();        

            i.begin_ice_ping(new Ice.Callback_Object_ice_ping()
                {
                    public void
                    response()
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();        
        
            i.begin_ice_id(new Ice.Callback_Object_ice_id()
                {
                    public void
                    response(String id)
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();        

            i.begin_ice_ids(new Ice.Callback_Object_ice_ids()
                {
                    public void
                    response(String[] ids)
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();        

            i.begin_op(new Callback_TestIntf_op()
                {
                    public void
                    response()
                    {
                        test(false);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();
        }
        out.println("ok");
    
        out.print("testing exception callback... ");
        out.flush();
        {
            TestIntfPrx i = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            final ExceptionCallback cb = new ExceptionCallback();
        
            i.begin_ice_isA("::Test::TestIntf", new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();        

            i.begin_op(new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();

            i.begin_opWithResult(new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();

            i.begin_opWithUE(new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                });
            cb.check();

            // Ensures no exception is called when response is received
            p.begin_ice_isA("::Test::TestIntf", new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.noEx(ex);
                    }
                });
            p.begin_op(new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.noEx(ex);
                    }
                });
            p.begin_opWithResult(new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.noEx(ex);
                    }
                });
            p.begin_opWithUE(new Ice.ExceptionCallback()
                {
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        cb.noEx(ex);
                    }
                });
        }
        out.println("ok");

        out.print("testing sent callback... ");
        out.flush();
        {
            final SentCallback cb = new SentCallback();

            p.begin_ice_isA("", new Ice.Callback_Object_ice_isA()
                {
                    public void
                    response(boolean r)
                    {
                        cb.isA(r);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }

                    public void
                    sent(boolean ss)
                    {
                        cb.sent(ss);
                    }
                });
            cb.check();
        
            p.begin_ice_ping(new Ice.Callback_Object_ice_ping()
                {
                    public void
                    response()
                    {
                        cb.ping();
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }

                    public void
                    sent(boolean ss)
                    {
                        cb.sent(ss);
                    }
                });
            cb.check();

            p.begin_ice_id(new Ice.Callback_Object_ice_id()
                {
                    public void
                    response(String id)
                    {
                        cb.id(id);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }

                    public void
                    sent(boolean ss)
                    {
                        cb.sent(ss);
                    }
                });
            cb.check();
        
            p.begin_ice_ids(new Ice.Callback_Object_ice_ids()
                {
                    public void
                    response(String[] ids)
                    {
                        cb.ids(ids);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }

                    public void
                    sent(boolean ss)
                    {
                        cb.sent(ss);
                    }
                });
            cb.check();

            p.begin_op(new Callback_TestIntf_op()
                {
                    public void
                    response()
                    {
                        cb.op();
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }

                    public void
                    sent(boolean ss)
                    {
                        cb.sent(ss);
                    }
                });
            cb.check();
        
            p.begin_op(new Ice.AsyncCallback()
                {
                    public void
                    completed(Ice.AsyncResult result)
                    {
                        cb.opAsync(result);
                    }
                    
                    public void
                    sent(Ice.AsyncResult result)
                    {
                        cb.sent(result);
                    }
                });
            cb.check();

            p.begin_op(new Ice.ExceptionCallback()
                {
                    public void
                    exception(Ice.LocalException ex)
                    {
                        cb.ex(ex);
                    }
                    
                    public void
                    sent(boolean ss)
                    {
                        cb.sent(ss);
                    }
                });
            cb.check();

            java.util.List<SentCallback> cbs = new java.util.ArrayList<SentCallback>();
            byte[] seq = new byte[10024];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
            Ice.AsyncResult r;
            testController.holdAdapter();
            do
            {
                final SentCallback cb2 = new SentCallback();
                r = p.begin_opWithPayload(seq, new Ice.ExceptionCallback()
                    {
                        public void
                        exception(Ice.LocalException ex)
                        {
                            cb2.ex(ex);
                        }
                    
                        public void
                        sent(boolean ss)
                        {
                            cb2.sent(ss);
                        }
                    });
                cbs.add(cb2);
            }
            while(r.sentSynchronously());
            testController.resumeAdapter();
            for(SentCallback cb3 : cbs)
            {
                cb3.check();
            }
        }
        out.println("ok");

        out.print("testing illegal arguments... ");
        out.flush();
        {
            Ice.AsyncResult result;
        
            result = p.begin_op();
            p.end_op(result);
            try
            {
                p.end_op(result);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }

            result = p.begin_op();
            try
            {
                p.end_opWithResult(result);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }

            try
            {
                p.end_op(null);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }
        
            // try
//             {
//                 p.begin_op((Ice.Callback)null);
//                 test(false);
//             }
//             catch(IllegalArgumentException ex)
//             {
//             }

//             try
//             {
//                 p.begin_op((Callback_TestIntf_op)null);
//                 test(false);
//             }
//             catch(IllegalArgumentException ex)
//             {
//             }
        }
        out.println("ok");

        out.print("testing unexpected exceptions from callback... ");
        out.flush();
        {
            TestIntfPrx q = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ThrowType throwEx[] = { ThrowType.LocalException, ThrowType.OtherException };
        
            for(int i = 0; i < 2; ++i)
            {
                final Thrower cb = new Thrower(throwEx[i]);

                p.begin_op(new Ice.AsyncCallback()
                    {
                        public void
                        completed(Ice.AsyncResult result)
                        {
                            cb.opAsync(result);
                        }
                    });
                cb.check();

                p.begin_op(new Callback_TestIntf_op()
                    {
                        public void 
                        response()
                        {
                            cb.op();
                        }
                        
                        public void
                        exception(Ice.LocalException ex)
                        {
                        }
                    });
                cb.check();

                q.begin_op(new Callback_TestIntf_op()
                    {
                        public void 
                        response()
                        {
                            cb.op();
                        }
                        
                        public void
                        exception(Ice.LocalException ex)
                        {
                            cb.ex(ex);
                        }
                    });
                cb.check();

                p.begin_op(new Callback_TestIntf_op()
                    {
                        public void 
                        response()
                        {
                        }
                        
                        public void
                        exception(Ice.LocalException ex)
                        {
                        }
                        
                        public void
                        sent(boolean ss)
                        {
                            cb.sent(ss);
                        }
                    });
                cb.check();

                q.begin_op(new Ice.ExceptionCallback()
                    {
                        public void
                        exception(Ice.LocalException ex)
                        {
                            cb.ex(ex);
                        }
                    });
                cb.check();
            }
        }
        out.println("ok");

        out.print("testing AsyncResult operations... ");
        out.flush();
        {
            testController.holdAdapter();

            Ice.AsyncResult r1 = p.begin_op();
            byte[] seq = new byte[10024];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
            Ice.AsyncResult r2;
            while((r2 = p.begin_opWithPayload(seq)).sentSynchronously());
        
            test(r1.sentSynchronously() && r1.isSent() && !r1.isCompleted() ||
                 !r1.sentSynchronously() && !r1.isCompleted());

            test(!r2.sentSynchronously() && !r2.isCompleted());

            testController.resumeAdapter();

            r1.waitForSent();
            test(r1.isSent());
        
            r2.waitForSent();
            test(r2.isSent());
        
            r1.waitForCompleted();
            test(r1.isCompleted());
        
            r2.waitForCompleted();
            test(r2.isCompleted());

            test(r1.getOperation().equals("op"));
            test(r2.getOperation().equals("opWithPayload"));
        }
        out.println("ok");

        p.shutdown();        
    }
}
