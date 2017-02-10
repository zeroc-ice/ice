// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami.lambda;

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

public class AMI
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
        connection(Ice.AsyncResult result)
        {
            test(result.getProxy().end_ice_getConnection(result) != null);
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
            catch(Ice.Exception ex)
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
            catch(Ice.Exception ex)
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
            catch(Ice.Exception ex)
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
            catch(Ice.Exception ex)
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
            catch(Ice.Exception ex)
            {
                test(false);
            }
        }

        public void
        connectionEx(Ice.AsyncResult result)
        {
            try
            {
                result.getProxy().end_ice_getConnection(result);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
                called();
            }
            catch(Ice.Exception ex)
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
            catch(Ice.Exception ex)
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
        connection(Ice.Connection conn)
        {
            test(conn != null);
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
        connection(Ice.Connection conn)
        {
            test(false);
        }

        public void
        op()
        {
            test(false);
        }

        public void
        ex(Ice.Exception ex)
        {
            test(ex instanceof Ice.NoEndpointException);
            called();
        }

        public void
        noEx(Ice.Exception ex)
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
        ex(Ice.Exception ex)
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
        exception(Ice.Exception ex)
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
            catch(Ice.Exception ex)
            {
                called();
            }
        }

        public void
        exception(Ice.Exception ex)
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
        ex(Ice.Exception ex)
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
    run(Application app, Ice.Communicator communicator, boolean collocated, TestIntfPrx p,
        TestIntfControllerPrx testController)
    {

        PrintWriter out = app.getWriter();

        out.print("testing response callback... ");
        out.flush();
        {
            final ResponseCallback cb = new ResponseCallback();
            java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();

            p.begin_ice_isA("::Test::TestIntf",
                (boolean r) -> cb.isA(r),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_ice_isA("::Test::TestIntf",
                (boolean r) -> cb.isA(r),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_ice_ping(
                () -> cb.ping(),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_ice_ping(ctx,
                () -> cb.ping(),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_ice_id(
                (String id) -> cb.id(id),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_ice_id(ctx,
                (String id) -> cb.id(id),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_ice_ids(
                (String[] ids) -> cb.ids(ids),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_ice_ids(ctx,
                (String[] ids) -> cb.ids(ids),
                (Ice.Exception ex) -> test(false));
            cb.check();

            if(!collocated)
            {
                p.begin_ice_getConnection(
                    (Ice.Connection conn) -> cb.connection(conn),
                    (Ice.Exception ex) -> test(false));
                cb.check();
            }

            p.begin_op(
                () -> cb.op(),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_op(ctx,
                () -> cb.op(),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_opWithResult(
                (int r) -> cb.opWithResult(r),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_opWithResult(ctx,
                (int r) -> cb.opWithResult(r),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_opWithUE(
                () -> test(false),
                (Ice.UserException ex) -> cb.opWithUE(ex),
                (Ice.Exception ex) -> test(false));
            cb.check();

            p.begin_opWithUE(ctx,
                () -> test(false),
                (Ice.UserException ex) -> cb.opWithUE(ex),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }
        out.println("ok");

        out.print("testing local exceptions with response callback... ");
        out.flush();
        {
            TestIntfPrx i = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            final ExceptionCallback cb = new ExceptionCallback();

            i.begin_ice_isA("::Test::TestIntf",
                (boolean r) -> test(false),
                (Ice.Exception ex) -> cb.ex(ex));
            cb.check();

            i.begin_ice_ping(
                () -> test(false),
                (Ice.Exception ex) -> cb.ex(ex));
            cb.check();

            i.begin_ice_id(
                (String id) -> test(false),
                (Ice.Exception ex) -> cb.ex(ex));
            cb.check();

            i.begin_ice_ids(
                (String[] ids) -> test(false),
                (Ice.Exception ex) -> cb.ex(ex));
            cb.check();

            if(!collocated)
            {
                i.begin_ice_getConnection(
                    (Ice.Connection conn) -> test(false),
                    (Ice.Exception ex) -> cb.ex(ex));
                cb.check();
            }

            i.begin_op(
                () -> test(false),
                (Ice.Exception ex) -> cb.ex(ex));
            cb.check();
        }
        out.println("ok");

        out.print("testing sent callback... ");
        out.flush();
        {
            final SentCallback cb = new SentCallback();

            p.begin_ice_isA("",
                (boolean r) -> cb.isA(r),
                (Ice.Exception ex) -> cb.ex(ex),
                (boolean ss) -> cb.sent(ss));
            cb.check();

            p.begin_ice_ping(
                () -> cb.ping(),
                (Ice.Exception ex) -> cb.ex(ex),
                (boolean ss) -> cb.sent(ss));
            cb.check();

            p.begin_ice_id(
                (String id) -> cb.id(id),
                (Ice.Exception ex) -> cb.ex(ex),
                (boolean ss) -> cb.sent(ss));
            cb.check();

            p.begin_ice_ids(
                (String[] ids) -> cb.ids(ids),
                (Ice.Exception ex) -> cb.ex(ex),
                (boolean ss) -> cb.sent(ss));
            cb.check();

            p.begin_op(
                () -> cb.op(),
                (Ice.Exception ex) -> cb.ex(ex),
                (boolean ss) -> cb.sent(ss));
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
                    r = p.begin_opWithPayload(seq,
                        () -> {},
                        (Ice.Exception ex) -> cb2.ex(ex),
                        (boolean ss) -> cb2.sent(ss));
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

        out.print("testing unexpected exceptions from callback... ");
        out.flush();
        {
            TestIntfPrx q = TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ThrowType throwEx[] = { ThrowType.LocalException, ThrowType.OtherException };

            for(int i = 0; i < 2; ++i)
            {
                final Thrower cb = new Thrower(throwEx[i]);

                p.begin_op(
                    () -> cb.op(),
                    null);
                cb.check();

                q.begin_op(
                    () -> cb.op(),
                    (Ice.Exception ex) -> cb.ex(ex));
                cb.check();

                p.begin_op(
                    () -> {},
                    (Ice.Exception ex) -> {},
                    (boolean ss) -> cb.sent(ss));
                cb.check();
            }
        }
        out.println("ok");

        out.print("testing batch requests with proxy... ");
        out.flush();
        {
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
                    null,
                    (Ice.Exception ex) -> cb.exception(ex),
                    (boolean sentSynchronously) -> cb.sent(sentSynchronously));
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(2));
            }

            if(p.ice_getConnection() != null)
            {
                //
                // Type-safe exception.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                final FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(
                    null,
                    (Ice.Exception ex) -> cb.exception(ex),
                    (boolean sentSynchronously) -> cb.sent(sentSynchronously));
                cb.check();
                test(r.isSent());
                test(r.isCompleted());
                test(p.waitForBatch(1));
            }
        }
        out.println("ok");

        if(p.ice_getConnection() != null)
        {
            out.print("testing batch requests with connection... ");
            out.flush();
            {
                {
                    //
                    // Type-safe.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    final FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb.exception(ex),
                        (boolean sentSynchronously) -> cb.sent(sentSynchronously));
                    cb.check();
                    test(r.isSent());
                    test(r.isCompleted());
                    test(p.waitForBatch(2));

                    final FlushCallback cb2 = new FlushCallback();
                    Ice.AsyncResult r2 = b1.ice_getConnection().begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb2.exception(ex),
                        (boolean sentSynchronously) -> cb2.sent(sentSynchronously));
                    cb2.check();
                    test(r2.isSent());
                    test(r2.isCompleted());
                }

                {
                    //
                    // Type-safe exception.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    final FlushExCallback cb = new FlushExCallback();
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb.exception(ex),
                        (boolean sentSynchronously) -> cb.sent(sentSynchronously));
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
                    // Type-safe - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    final FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb.exception(ex),
                        (boolean sentSynchronously) -> cb.sent(sentSynchronously));
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    final FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb.exception(ex),
                        (boolean sentSynchronously) -> cb.sent(sentSynchronously));
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b1.opBatch();
                    b2.opBatch();
                    b2.opBatch();
                    final FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb.exception(ex),
                        (boolean sentSynchronously) -> cb.sent(sentSynchronously));
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    final FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb.exception(ex),
                        (boolean sentSynchronously) -> cb.sent(sentSynchronously));
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    b2.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    final FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        Ice.CompressBatch.BasedOnProxy,
                        null,
                        (Ice.Exception ex) -> cb.exception(ex),
                        (boolean sentSynchronously) -> cb.sent(sentSynchronously));
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.isCompleted());
                    test(p.opBatchCount() == 0);
                }
            }
            out.println("ok");
        }

        out.print("testing null callbacks...");
        try
        {
            IceInternal.Functional_VoidCallback response = null;
            IceInternal.Functional_GenericCallback1<Ice.Exception> exception = null;
            p.begin_ice_ping(response, exception);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Excepted when response and exception callback are both null.
        }

        try
        {
            p.begin_ice_ping(() -> {}, null);

        }
        catch(IllegalArgumentException ex)
        {
            test(false);
        }

        try
        {
            p.begin_ice_ping(null, (Ice.Exception ex) -> {});

        }
        catch(IllegalArgumentException ex)
        {
            test(false);
        }

        try
        {
            IceInternal.Functional_BoolCallback response = null;
            IceInternal.Functional_GenericCallback1<Ice.Exception> exception = null;
            p.begin_ice_isA("::Test::TestIntf", response, exception);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Excepted when response and exception callback are both null.
        }

        try
        {
            p.begin_ice_isA("::Test::TestIntf", (boolean v) -> {}, null);

        }
        catch(IllegalArgumentException ex)
        {
            test(false);
        }

        try
        {
            p.begin_ice_isA("::Test::TestIntf", null, (Ice.Exception ex) -> {});

        }
        catch(IllegalArgumentException ex)
        {
            test(false);
        }

        try
        {
            IceInternal.Functional_VoidCallback response = null;
            p.begin_opWithUE(response, null, (Ice.Exception ex) -> {});
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Excepted when response and exception callback are both null, for
            // an operation that throws user exceptions both user exception callback
            // an local exception callback must be present.
        }

        try
        {
            IceInternal.Functional_VoidCallback response = null;
            p.begin_opWithUE(response, (Ice.UserException ex) -> {}, null);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Excepted when response and exception callback are both null, for
            // an operation that throws user exceptions both user exception callback
            // an local exception callback must be present.
        }

        try
        {
            IceInternal.Functional_VoidCallback response = null;
            IceInternal.Functional_GenericCallback1<Ice.UserException> userException = null;
            IceInternal.Functional_GenericCallback1<Ice.Exception> exception = null;
            p.begin_opWithUE(response, userException, exception);
            test(false);
        }
        catch(IllegalArgumentException ex)
        {
            // Excepted when response and exception callback are both null.
        }
        out.println("ok");
    }
}
