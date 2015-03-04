// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
import test.Ice.ami.Test.Callback_TestIntf_opWithPayload;
import test.Util.Application;

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
    }

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
    }

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
    }

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
    }

    static class FlushCallback extends CallbackBase
    {
        FlushCallback()
        {
            _thread = Thread.currentThread().getId();
        }

        public void
        completedAsync(Ice.AsyncResult r)
        {
            test(false);
        }

        public void
        exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        sentAsync(Ice.AsyncResult r)
        {
            test((r.sentSynchronously() && _thread == Thread.currentThread().getId()) ||
                 (!r.sentSynchronously() && _thread != Thread.currentThread().getId()));
            called();
        }

        public void
        sent(boolean sentSynchronously)
        {
            test((sentSynchronously && _thread == Thread.currentThread().getId()) ||
                 (!sentSynchronously && _thread != Thread.currentThread().getId()));
            called();
        }

        long _thread;
    }

    static class FlushExCallback extends CallbackBase
    {
        FlushExCallback()
        {
        }

        public void
        completedAsync(Ice.AsyncResult r)
        {
            try
            {
                if(r.getConnection() != null)
                {
                    r.getConnection().end_flushBatchRequests(r);
                }
                else
                {
                    r.getProxy().end_ice_flushBatchRequests(r);
                }
                test(false);
            }
            catch(Ice.LocalException ex)
            {
                called();
            }
        }

        public void
        exception(Ice.LocalException ex)
        {
            called();
        }

        public void
        sentAsync(Ice.AsyncResult r)
        {
            test(false);
        }

        public void
        sent(boolean sentSynchronously)
        {
            test(false);
        }
    }

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
    }

    public static void
    allTests(Application app)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

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

            p.begin_ice_isA("::Test::TestIntf", new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.isA(r);
                    }
                });
            cb.check();

            p.begin_ice_isA("::Test::TestIntf", ctx, new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.isA(r);
                    }
                });
            cb.check();

            p.begin_ice_ping(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ping(r);
                    }
                });
            cb.check();
            p.begin_ice_ping(ctx, new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ping(r);
                    }
                });
            cb.check();

            p.begin_ice_id(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.id(r);
                    }
                });
            cb.check();
            p.begin_ice_id(ctx, new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.id(r);
                    }
                });
            cb.check();

            p.begin_ice_ids(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ids(r);
                    }
                });
            cb.check();
            p.begin_ice_ids(ctx, new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.ids(r);
                    }
                });
            cb.check();

            p.begin_op(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.op(r);
                    }
                });
            cb.check();
            p.begin_op(ctx, new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.op(r);
                    }
                });
            cb.check();

            p.begin_opWithResult(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opWithResult(r);
                    }
                });
            cb.check();
            p.begin_opWithResult(ctx, new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opWithResult(r);
                    }
                });
            cb.check();

            p.begin_opWithUE(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.opWithUE(r);
                    }
                });
            cb.check();
            p.begin_opWithUE(ctx, new Ice.Callback()
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


            try
            {
                r = ((TestIntfPrx)p.ice_oneway()).begin_opWithResult();
                test(false);
            }
            catch(java.lang.IllegalArgumentException ex)
            {
            }

            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            Ice.Communicator ic = app.initialize(initData);
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

            i.begin_ice_isA("::Test::TestIntf", new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.isAEx(r);
                    }
                });
            cb.check();

            i.begin_ice_ping(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.pingEx(r);
                    }
                });
            cb.check();

            i.begin_ice_id(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.idEx(r);
                    }
                });
            cb.check();

            i.begin_ice_ids(new Ice.Callback()
                {
                    public void
                    completed(Ice.AsyncResult r)
                    {
                        cb.idsEx(r);
                    }
                });
            cb.check();

            i.begin_op(new Ice.Callback()
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

            p.begin_op(new Ice.Callback()
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

            java.util.List<SentCallback> cbs = new java.util.ArrayList<SentCallback>();
            byte[] seq = new byte[10024];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
            Ice.AsyncResult r;
            testController.holdAdapter();
            try
            {
                do
                {
                    final SentCallback cb2 = new SentCallback();
                    r = p.begin_opWithPayload(seq, new Callback_TestIntf_opWithPayload()
                        {
                            public void
                            response()
                            {
                            }

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
            }
            finally
            {
                testController.resumeAdapter();
            }
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

                p.begin_op(new Ice.Callback()
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
            }
        }
        out.println("ok");

        out.print("testing batch requests with proxy... ");
        out.flush();
        {
            {
                //
                // AsyncResult.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(2));
            }

            {
                //
                // AsyncResult exception.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                final FlushExCallback cb = new FlushExCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(!r.isSent());
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }

            {
                //
                // Type-safe.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(
                    new Ice.Callback_Object_ice_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(2));
            }

            {
                //
                // Type-safe exception.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                final FlushExCallback cb = new FlushExCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(
                    new Ice.Callback_Object_ice_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(!r.isSent());
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }
        }
        out.println("ok");

        out.print("testing batch requests with connection... ");
        out.flush();
        {
            {
                //
                // AsyncResult.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(2));
            }

            {
                //
                // AsyncResult exception.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                final FlushExCallback cb = new FlushExCallback();
                Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(!r.isSent());
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }

            {
                //
                // Type-safe.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                    new Ice.Callback_Connection_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(2));
            }

            {
                //
                // Type-safe exception.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                final FlushExCallback cb = new FlushExCallback();
                Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                    new Ice.Callback_Connection_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(!r.isSent());
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }
        }
        out.println("ok");

        out.print("testing batch requests with communicator... ");
        out.flush();
        {
            {
                //
                // AsyncResult - 1 connection.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(2));
            }

            {
                //
                // AsyncResult exception - 1 connection.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(r.isSent()); // Exceptions are ignored!
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }

            {
                //
                // AsyncResult - 2 connections.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                TestIntfPrx b2 = (TestIntfPrx)p.ice_connectionId("2").ice_batchOneway();
                b2.ice_getConnection(); // Ensure connection is established.
                b1.opBatch();
                b1.opBatch();
                b2.opBatch();
                b2.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(4));
            }

            {
                //
                // AsyncResult exception - 2 connections - 1 failure.
                //
                // All connections should be flushed even if there are failures on some connections.
                // Exceptions should not be reported.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                TestIntfPrx b2 = (TestIntfPrx)p.ice_connectionId("2").ice_batchOneway();
                b2.ice_getConnection(); // Ensure connection is established.
                b1.opBatch();
                b2.opBatch();
                b1.ice_getConnection().close(false);
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(r.isSent()); // Exceptions are ignored!
                test(r.isCompleted());
                test(p.waitForBatch(1));
            }

            {
                //
                // AsyncResult exception - 2 connections - 2 failures.
                //
                // The sent callback should be invoked even if all connections fail.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                TestIntfPrx b2 = (TestIntfPrx)p.ice_connectionId("2").ice_batchOneway();
                b2.ice_getConnection(); // Ensure connection is established.
                b1.opBatch();
                b2.opBatch();
                b1.ice_getConnection().close(false);
                b2.ice_getConnection().close(false);
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback()
                    {
                        @Override
                        public void completed(Ice.AsyncResult result)
                        {
                            cb.completedAsync(result);
                        }

                        @Override
                        public void sent(Ice.AsyncResult result)
                        {
                            cb.sentAsync(result);
                        }
                    });
                cb.check();
                test(r.isSent()); // Exceptions are ignored!
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }

            {
                //
                // Type-safe - 1 connection.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback_Communicator_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(2));
            }

            {
                //
                // Type-safe exception - 1 connection.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback_Communicator_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(r.isSent()); // Exceptions are ignored!
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }

            {
                //
                // 2 connections.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                TestIntfPrx b2 = (TestIntfPrx)p.ice_connectionId("2").ice_batchOneway();
                b2.ice_getConnection(); // Ensure connection is established.
                b1.opBatch();
                b1.opBatch();
                b2.opBatch();
                b2.opBatch();
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback_Communicator_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(4));
            }

            {
                //
                // Exception - 2 connections - 1 failure.
                //
                // All connections should be flushed even if there are failures on some connections.
                // Exceptions should not be reported.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                TestIntfPrx b2 = (TestIntfPrx)p.ice_connectionId("2").ice_batchOneway();
                b2.ice_getConnection(); // Ensure connection is established.
                b1.opBatch();
                b2.opBatch();
                b1.ice_getConnection().close(false);
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback_Communicator_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(r.isSent()); // Exceptions are ignored!
                test(r.isCompleted());
                test(p.waitForBatch(1));
            }

            {
                //
                // Exception - 2 connections - 2 failures.
                //
                // The sent callback should be invoked even if all connections fail.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                TestIntfPrx b2 = (TestIntfPrx)p.ice_connectionId("2").ice_batchOneway();
                b2.ice_getConnection(); // Ensure connection is established.
                b1.opBatch();
                b2.opBatch();
                b1.ice_getConnection().close(false);
                b2.ice_getConnection().close(false);
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                    new Ice.Callback_Communicator_flushBatchRequests()
                    {
                        @Override
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }

                        @Override
                        public void sent(boolean sentSynchronously)
                        {
                            cb.sent(sentSynchronously);
                        }
                    });
                cb.check();
                test(r.isSent()); // Exceptions are ignored!
                test(r.isCompleted());
                test(p.opBatchCount() == 0);
            }
        }
        out.println("ok");

        out.print("testing AsyncResult operations... ");
        out.flush();
        {
            {
                TestIntfPrx indirect = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
                Ice.AsyncResult r = indirect.begin_op();
                try
                {
                    r.waitForCompleted();
                    r.throwLocalException();
                    test(false);
                }
                catch(Ice.NoEndpointException ex)
                {
                }

                testController.holdAdapter();
                Ice.AsyncResult r1;
                Ice.AsyncResult r2;
                try
                {
                    r1 = p.begin_op();
                    byte[] seq = new byte[10024];
                    new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
                    while((r2 = p.begin_opWithPayload(seq)).sentSynchronously());

                    test(r1.sentSynchronously() && r1.isSent() && !r1.isCompleted() ||
                         !r1.sentSynchronously() && !r1.isCompleted());

                    test(!r2.sentSynchronously() && !r2.isCompleted());
                }
                finally
                {
                    testController.resumeAdapter();
                }

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

            {
                Ice.AsyncResult r;

                //
                // Twoway
                //
                r = p.begin_ice_ping();
                test(r.getOperation().equals("ice_ping"));
                test(r.getConnection() == null); // Expected
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == p);
                p.end_ice_ping(r);

                TestIntfPrx p2;

                //
                // Oneway
                //
                p2 = (TestIntfPrx)p.ice_oneway();
                r = p2.begin_ice_ping();
                test(r.getOperation().equals("ice_ping"));
                test(r.getConnection() == null); // Expected
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == p2);

                //
                // Batch request via proxy
                //
                p2 = (TestIntfPrx)p.ice_batchOneway();
                p2.ice_ping();
                r = p2.begin_ice_flushBatchRequests();
                test(r.getConnection() == null); // Expected
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == p2);
                p2.end_ice_flushBatchRequests(r);

                //
                // Batch request via connection
                //
                Ice.Connection con = p.ice_getConnection();
                p2 = (TestIntfPrx)p.ice_batchOneway();
                p2.ice_ping();
                r = con.begin_flushBatchRequests();
                test(r.getConnection() == con);
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == null); // Expected
                con.end_flushBatchRequests(r);

                //
                // Batch request via communicator
                //
                p2 = (TestIntfPrx)p.ice_batchOneway();
                p2.ice_ping();
                r = communicator.begin_flushBatchRequests();
                test(r.getConnection() == null); // Expected
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == null); // Expected
                communicator.end_flushBatchRequests(r);
            }
        }
        out.println("ok");
        
        out.print("testing close connection with sending queue... ");
        out.flush();
        {
            byte[] seq = new byte[1024 * 10];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.

            //
            // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
            // The goal is to make sure that none of the opWithPayload fail even if the server closes 
            // the connection gracefully in between.
            // 
            int maxQueue = 2;
            boolean done = false;
            while(!done && maxQueue < 50)
            {
                done = true;
                p.ice_ping();
                java.util.List<Ice.AsyncResult> results = new java.util.ArrayList<Ice.AsyncResult>();
                for(int i = 0; i < maxQueue; ++i)
                {
                    results.add(p.begin_opWithPayload(seq));
                }
                if(!p.begin_close(false).isSent())
                {
                    for(int i = 0; i < maxQueue; i++)
                    {
                        Ice.AsyncResult r = p.begin_opWithPayload(seq);
                        results.add(r);
                        if(r.isSent())
                        {
                            done = false;
                            maxQueue *= 2;
                            break;
                        }
                    }
                }
                else
                {
                    maxQueue *= 2;
                    done = false;
                }
                for(Ice.AsyncResult q : results)
                {
                    q.waitForCompleted();
                    try
                    {
                        q.throwLocalException();
                    }
                    catch(Ice.LocalException ex)
                    {
                        test(false);
                    }
                }
            }
        }
        out.println("ok");

        p.shutdown();
    }
}
