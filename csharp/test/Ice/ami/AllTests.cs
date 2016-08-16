// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Test;

public class AllTests : TestCommon.TestApp
{
    private class Cookie
    {
        public Cookie(int i)
        {
            val = i;
        }

        public int val;
    }

    public class Progress : IProgress<bool>
    {
        public Progress(Action<bool> report)
        {
            _report = report;
        }

        public void Report(bool sentSynchronously)
        {
            _report(sentSynchronously);
        }

        Action<bool> _report;
    }

    public class ProgresCallback : IProgress<bool>
    {
        public bool Sent
        {
            get
            {
                lock(this)
                {
                    return _sent;
                }
            }
            set
            {
                lock(this)
                {
                    _sent = value;
                }
            }
        }

        public bool SentSynchronously
        {
            get
            {
                lock(this)
                {
                    return _sentSynchronously;
                }
            }
            set
            {
                lock(this)
                {
                    _sentSynchronously = value;
                }
            }
        }

        public void Report(bool sentSynchronously)
        {
            SentSynchronously = sentSynchronously;
            Sent = true;
        }

        private bool _sent = false;
        private bool _sentSynchronously = false;
    }

    private class CallbackBase
    {
        internal CallbackBase()
        {
            _called = false;
        }

        public virtual void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }
                _called = false;
            }
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    private class AsyncCallback : CallbackBase
    {
        public AsyncCallback()
        {
        }

        public AsyncCallback(Cookie cookie)
        {
            _cookie = cookie;
        }

        public void
        isA(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            test(result.getProxy().end_ice_isA(result));
            called();
        }

        public void
        ping(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            result.getProxy().end_ice_ping(result);
            called();
        }

        public void
        id(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            test(result.getProxy().end_ice_id(result).Equals("::Test::TestIntf"));
            called();
        }

        public void
        ids(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            test(result.getProxy().end_ice_ids(result).Length == 2);
            called();
        }

        public void
        connection(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            test(result.getProxy().end_ice_getConnection(result) != null);
            called();
        }

        public void
        op(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_op(result);
            called();
        }

        public void
        opWithResult(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            test(TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_opWithResult(result) == 15);
            called();
        }

        public void
        opWithUE(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            try
            {
                TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_opWithUE(result);
                test(false);
            }
            catch(TestIntfException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        public void
        isAEx(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            try
            {
                result.getProxy().end_ice_isA(result);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        public void
        pingEx(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            try
            {
                result.getProxy().end_ice_ping(result);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        public void
        idEx(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            try
            {
                result.getProxy().end_ice_id(result);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        public void
        idsEx(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            try
            {
                result.getProxy().end_ice_ids(result);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        public void
        connectionEx(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            try
            {
                result.getProxy().end_ice_getConnection(result);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        public void
        opEx(Ice.AsyncResult result)
        {
            test(result.AsyncState == _cookie);
            try
            {
                TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_op(result);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        private Cookie _cookie;
    }

    private class ResponseCallback : CallbackBase
    {
        public ResponseCallback()
        {
        }

        public void
        isA(bool r)
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
        id(string id)
        {
            test(id.Equals("::Test::TestIntf"));
            called();
        }

        public void
        ids(string[] ids)
        {
            test(ids.Length == 2);
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
        opWithUE(Ice.Exception e)
        {
            try
            {
                throw e;
            }
            catch(TestIntfException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

    }

    private class ExceptionCallback : CallbackBase
    {
        public ExceptionCallback()
        {
        }

        public void
        isA(bool r)
        {
            test(false);
        }

        public void
        ping()
        {
            test(false);
        }

        public void
        id(string id)
        {
            test(false);
        }

        public void
        ids(string[] ids)
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
        opWithUE(Ice.Exception e)
        {
            try
            {
                throw e;
            }
            catch(TestIntfException)
            {
                called();
            }
            catch(Ice.Exception)
            {
                test(false);
            }
        }

        public void
        ex(Ice.Exception ex)
        {
            test(ex is Ice.NoEndpointException);
            called();
        }

        public void
        noEx(Ice.Exception ex)
        {
            test(false);
        }
    }

    private class SentCallback : CallbackBase
    {
        public SentCallback()
        {
            _thread = Thread.CurrentThread;
        }

        public void
        isA(bool r)
        {
        }

        public void
        ping()
        {
        }

        public void
        id(string s)
        {
        }

        public void
        ids(string[] s)
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
        sentAsync(Ice.AsyncResult r)
        {
            test(r.sentSynchronously() && _thread == Thread.CurrentThread ||
                 !r.sentSynchronously() && _thread != Thread.CurrentThread);
            called();
        }

        public void
        sent(bool ss)
        {
            test(ss && _thread == Thread.CurrentThread || !ss && _thread != Thread.CurrentThread);

            called();
        }

        Thread _thread;
    }

    private class FlushCallback : CallbackBase
    {
        public FlushCallback()
        {
            _thread = Thread.CurrentThread;
        }

        public FlushCallback(Cookie cookie)
        {
            _thread = Thread.CurrentThread;
            _cookie = cookie;
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
            test(r.AsyncState == _cookie);
            test(r.sentSynchronously() && _thread == Thread.CurrentThread ||
                 !r.sentSynchronously() && _thread != Thread.CurrentThread);
            called();
        }

        public void
        sent(bool sentSynchronously)
        {
            test(sentSynchronously && _thread == Thread.CurrentThread ||
                 !sentSynchronously && _thread != Thread.CurrentThread);
            called();
        }

        Cookie _cookie;
        Thread _thread;
    }

    private class FlushExCallback : CallbackBase
    {
        public FlushExCallback()
        {
        }

        public FlushExCallback(Cookie cookie)
        {
            _cookie = cookie;
        }

        public void
        completedAsync(Ice.AsyncResult r)
        {
            test(r.AsyncState == _cookie);
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
            catch(Ice.LocalException)
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
        sent(bool sentSynchronously)
        {
            test(false);
        }

        Cookie _cookie;
    }

    enum ThrowType { LocalException, UserException, OtherException };

    private class Thrower : CallbackBase
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
        sent(bool ss)
        {
            called();
            throwEx();
        }

        private void
        throwEx()
        {
            switch(_t)
            {
            case ThrowType.LocalException:
            {
                throw new Ice.ObjectNotExistException();
            }
            case ThrowType.UserException:
            {
                throw new TestIntfException();
            }
            case ThrowType.OtherException:
            {
                throw new Exception();
            }
            default:
            {
                Debug.Assert(false);
                break;
            }
            }
        }

        ThrowType _t;
    }

    public static void allTests(Ice.Communicator communicator, bool collocated)
    {
        string sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfPrx p = Test.TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:default -p 12011";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfControllerPrx testController = Test.TestIntfControllerPrxHelper.uncheckedCast(obj);

        Write("testing async invocation...");
        Flush();
        {
            Dictionary<string, string> ctx = new Dictionary<string, string>();

            test(p.ice_isAAsync("::Test::TestIntf").Result);
            test(p.ice_isAAsync("::Test::TestIntf", ctx).Result);

            p.ice_pingAsync().Wait();
            p.ice_pingAsync(ctx).Wait();

            test(p.ice_idAsync().Result.Equals("::Test::TestIntf"));
            test(p.ice_idAsync(ctx).Result.Equals("::Test::TestIntf"));

            test(p.ice_idsAsync().Result.Length == 2);
            test(p.ice_idsAsync(ctx).Result.Length == 2);

            if(!collocated)
            {
                test(p.ice_getConnectionAsync().Result != null);
            }

            p.opAsync().Wait();
            p.opAsync(ctx).Wait();

            test(p.opWithResultAsync().Result == 15);
            test(p.opWithResultAsync(ctx).Result == 15);

            try
            {
                p.opWithUEAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                    ae.Handle((ex) =>
                {
                    return ex is Test.TestIntfException;
                });
            }

            try
            {
                p.opWithUEAsync(ctx).Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                ae.Handle((ex) =>
                {
                    return ex is Test.TestIntfException;
                });
            }
        }
        WriteLine("ok");

        Write("testing begin/end invocation... ");
        Flush();
        {
            Ice.AsyncResult result;
            Dictionary<string, string> ctx = new Dictionary<string, string>();

            result = p.begin_ice_isA("::Test::TestIntf");
            test(p.end_ice_isA(result));
            result = p.begin_ice_isA("::Test::TestIntf", ctx);
            test(p.end_ice_isA(result));

            result = p.begin_ice_ping();
            p.end_ice_ping(result);
            result = p.begin_ice_ping(ctx);
            p.end_ice_ping(result);

            result = p.begin_ice_id();
            test(p.end_ice_id(result).Equals("::Test::TestIntf"));
            result = p.begin_ice_id(ctx);
            test(p.end_ice_id(result).Equals("::Test::TestIntf"));

            result = p.begin_ice_ids();
            test(p.end_ice_ids(result).Length == 2);
            result = p.begin_ice_ids(ctx);
            test(p.end_ice_ids(result).Length == 2);

            if(!collocated)
            {
                result = p.begin_ice_getConnection();
                test(p.end_ice_getConnection(result) != null);
            }

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
            catch(Test.TestIntfException)
            {
            }
            result = p.begin_opWithUE(ctx);
            try
            {
                p.end_opWithUE(result);
                test(false);
            }
            catch(Test.TestIntfException)
            {
            }
        }
        WriteLine("ok");

        Write("testing async callback... ");
        Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            Dictionary<string, string> ctx = new Dictionary<string, string>();
            Cookie cookie = new Cookie(5);
            AsyncCallback cbWC = new AsyncCallback(cookie);

            p.begin_ice_isA("::Test::TestIntf", cb.isA, null);
            cb.check();
            p.begin_ice_isA("::Test::TestIntf", cbWC.isA, cookie);
            cbWC.check();
            p.begin_ice_isA("::Test::TestIntf", ctx, cb.isA, null);
            cb.check();
            p.begin_ice_isA("::Test::TestIntf", ctx, cbWC.isA, cookie);
            cbWC.check();

            p.begin_ice_ping(cb.ping, null);
            cb.check();
            p.begin_ice_ping(cbWC.ping, cookie);
            cbWC.check();
            p.begin_ice_ping(ctx, cb.ping, null);
            cb.check();
            p.begin_ice_ping(ctx, cbWC.ping, cookie);
            cbWC.check();

            p.begin_ice_id(cb.id, null);
            cb.check();
            p.begin_ice_id(cbWC.id, cookie);
            cbWC.check();
            p.begin_ice_id(ctx, cb.id, null);
            cb.check();
            p.begin_ice_id(ctx, cbWC.id, cookie);
            cbWC.check();

            p.begin_ice_ids(cb.ids, null);
            cb.check();
            p.begin_ice_ids(cbWC.ids, cookie);
            cbWC.check();
            p.begin_ice_ids(ctx, cb.ids, null);
            cb.check();
            p.begin_ice_ids(ctx, cbWC.ids, cookie);
            cbWC.check();

            if(!collocated)
            {
                p.begin_ice_getConnection(cb.connection, null);
                cb.check();
                p.begin_ice_getConnection(cbWC.connection, cookie);
                cbWC.check();
            }

            p.begin_op(cb.op, null);
            cb.check();
            p.begin_op(cbWC.op, cookie);
            cbWC.check();
            p.begin_op(ctx, cb.op, null);
            cb.check();
            p.begin_op(ctx, cbWC.op, cookie);
            cbWC.check();

            p.begin_opWithResult(cb.opWithResult, null);
            cb.check();
            p.begin_opWithResult(cbWC.opWithResult, cookie);
            cbWC.check();
            p.begin_opWithResult(ctx, cb.opWithResult, null);
            cb.check();
            p.begin_opWithResult(ctx, cbWC.opWithResult, cookie);
            cbWC.check();

            p.begin_opWithUE(cb.opWithUE, null);
            cb.check();
            p.begin_opWithUE(cbWC.opWithUE, cookie);
            cbWC.check();
            p.begin_opWithUE(ctx, cb.opWithUE, null);
            cb.check();
            p.begin_opWithUE(ctx, cbWC.opWithUE, cookie);
            cbWC.check();
        }
        WriteLine("ok");

        Write("testing response callback... ");
        Flush();
        {
            ResponseCallback cb = new ResponseCallback();
            Dictionary<string, string> ctx = new Dictionary<string, string>();

            p.begin_ice_isA("::Test::TestIntf").whenCompleted(cb.isA, null);
            cb.check();
            p.begin_ice_isA("::Test::TestIntf", ctx).whenCompleted(cb.isA, null);
            cb.check();

            p.begin_ice_ping().whenCompleted(cb.ping, null);
            cb.check();
            p.begin_ice_ping(ctx).whenCompleted(cb.ping, null);
            cb.check();

            p.begin_ice_id().whenCompleted(cb.id, null);
            cb.check();
            p.begin_ice_id(ctx).whenCompleted(cb.id, null);
            cb.check();

            p.begin_ice_ids().whenCompleted(cb.ids, null);
            cb.check();
            p.begin_ice_ids(ctx).whenCompleted(cb.ids, null);
            cb.check();

            if(!collocated)
            {
                p.begin_ice_getConnection().whenCompleted(cb.connection, null);
                cb.check();
            }

            p.begin_op().whenCompleted(cb.op, null);
            cb.check();
            p.begin_op(ctx).whenCompleted(cb.op, null);
            cb.check();

            p.begin_opWithResult().whenCompleted(cb.opWithResult, null);
            cb.check();
            p.begin_opWithResult(ctx).whenCompleted(cb.opWithResult, null);
            cb.check();

            p.begin_opWithUE().whenCompleted(cb.op, cb.opWithUE);
            cb.check();
            p.begin_opWithUE(ctx).whenCompleted(cb.op, cb.opWithUE);
            cb.check();
        }
        WriteLine("ok");

        Write("testing lambda callback... ");
        Flush();
        {
            ResponseCallback cb = new ResponseCallback();
            Dictionary<string, string> ctx = new Dictionary<string, string>();

            p.begin_ice_isA("::Test::TestIntf").whenCompleted(
                (bool r) =>
                {
                    cb.isA(r);
                }, null);
            cb.check();
            p.begin_ice_isA("::Test::TestIntf", ctx).whenCompleted(
                (bool r) =>
                {
                    cb.isA(r);
                }, null);
            cb.check();

            p.begin_ice_ping().whenCompleted(
                () =>
                {
                    cb.ping();
                }, null);
            cb.check();
            p.begin_ice_ping(ctx).whenCompleted(
                () =>
                {
                    cb.ping();
                }, null);
            cb.check();

            p.begin_ice_id().whenCompleted(
                (string id) =>
                {
                    cb.id(id);
                }, null);
            cb.check();
            p.begin_ice_id(ctx).whenCompleted(
                (string id) =>
                {
                    cb.id(id);
                }, null);
            cb.check();

            p.begin_ice_ids().whenCompleted(
                (string[] ids) =>
                {
                    cb.ids(ids);
                }, null);
            cb.check();
            p.begin_ice_ids(ctx).whenCompleted(
                (string[] ids) =>
                {
                    cb.ids(ids);
                }, null);
            cb.check();

            if(!collocated)
            {
                p.begin_ice_getConnection().whenCompleted(
                    (Ice.Connection conn) =>
                    {
                        cb.connection(conn);
                    }, null);
                cb.check();
            }

            p.begin_op().whenCompleted(
                () =>
                {
                    cb.op();
                }, null);
            cb.check();
            p.begin_op(ctx).whenCompleted(
                () =>
                {
                    cb.op();
                }, null);
            cb.check();

            p.begin_opWithResult().whenCompleted(
                (int r) =>
                {
                    cb.opWithResult(r);
                }, null);
            cb.check();
            p.begin_opWithResult(ctx).whenCompleted(
                (int r) =>
                {
                    cb.opWithResult(r);
                }, null);
            cb.check();

            p.begin_opWithUE().whenCompleted(
                () =>
                {
                    cb.op();
                },
                (Ice.Exception ex) =>
                {
                    cb.opWithUE(ex);
                });
            cb.check();
            p.begin_opWithUE(ctx).whenCompleted(
                () =>
                {
                    cb.op();
                },
                (Ice.Exception ex) =>
                {
                    cb.opWithUE(ex);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("testing local exceptions... ");
        Flush();
        {
            Test.TestIntfPrx indirect = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            Ice.AsyncResult r;

            r = indirect.begin_op();
            try
            {
                indirect.end_op(r);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
            }


            try
            {
                r = ((Test.TestIntfPrx)p.ice_oneway()).begin_opWithResult();
                test(false);
            }
            catch(System.ArgumentException)
            {
            }

            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            if(p.ice_getConnection() != null)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().ice_clone_();
                Ice.Communicator ic = Ice.Util.initialize(initData);
                Ice.ObjectPrx o = ic.stringToProxy(p.ToString());
                Test.TestIntfPrx p2 = Test.TestIntfPrxHelper.checkedCast(o);
                ic.destroy();

                try
                {
                    p2.begin_op();
                    test(false);
                }
                catch(Ice.CommunicatorDestroyedException)
                {
                    // Expected.
                }
            }
        }
        WriteLine("ok");

        Write("testing local exceptions with async tasks... ");
        Flush();
        {
            Test.TestIntfPrx indirect = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));

            try
            {
                indirect.opAsync().Wait();
                test(false);
            }
            catch(System.AggregateException ae)
            {
                ae.Handle((ex) =>
                {
                    return ex is Ice.NoEndpointException;
                });
            }

            try
            {
                ((Test.TestIntfPrx)p.ice_oneway()).opWithResultAsync();
                test(false);
            }
            catch(System.ArgumentException)
            {
            }

            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            if(p.ice_getConnection() != null)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().ice_clone_();
                Ice.Communicator ic = Ice.Util.initialize(initData);
                Ice.ObjectPrx o = ic.stringToProxy(p.ToString());
                Test.TestIntfPrx p2 = Test.TestIntfPrxHelper.checkedCast(o);
                ic.destroy();

                try
                {
                    p2.opAsync();
                    test(false);
                }
                catch(Ice.CommunicatorDestroyedException)
                {
                    // Expected.
                }
            }
        }
        WriteLine("ok");

        Write("testing local exceptions with async callback... ");
        Flush();
        {
            Test.TestIntfPrx i = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AsyncCallback cb = new AsyncCallback();
            Cookie cookie = new Cookie(5);
            AsyncCallback cbWC = new AsyncCallback(cookie);

            i.begin_ice_isA("::Test::TestIntf", cb.isAEx, null);
            cb.check();
            i.begin_ice_isA("::Test::TestIntf", cbWC.isAEx, cookie);
            cbWC.check();

            i.begin_ice_ping(cb.pingEx, null);
            cb.check();
            i.begin_ice_ping(cbWC.pingEx, cookie);
            cbWC.check();

            i.begin_ice_id(cb.idEx, null);
            cb.check();
            i.begin_ice_id(cbWC.idEx, cookie);
            cbWC.check();

            i.begin_ice_ids(cb.idsEx, null);
            cb.check();
            i.begin_ice_ids(cbWC.idsEx, cookie);
            cbWC.check();

            if(!collocated)
            {
                i.begin_ice_getConnection(cb.connectionEx, null);
                cb.check();
                i.begin_ice_getConnection(cbWC.connectionEx, cookie);
                cbWC.check();
            }

            i.begin_op(cb.opEx, null);
            cb.check();
            i.begin_op(cbWC.opEx, cookie);
            cbWC.check();
        }
        WriteLine("ok");

        Write("testing local exceptions with response callback... ");
        Flush();
        {
            Test.TestIntfPrx i = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ExceptionCallback cb = new ExceptionCallback();

            i.begin_ice_isA("::Test::TestIntf").whenCompleted(cb.isA, cb.ex);
            cb.check();

            i.begin_ice_ping().whenCompleted(cb.ping, cb.ex);
            cb.check();

            i.begin_ice_id().whenCompleted(cb.id, cb.ex);
            cb.check();

            i.begin_ice_ids().whenCompleted(cb.ids, cb.ex);
            cb.check();

            if(!collocated)
            {
                i.begin_ice_getConnection().whenCompleted(cb.connection, cb.ex);
                cb.check();
            }

            i.begin_op().whenCompleted(cb.op, cb.ex);
            cb.check();
        }
        WriteLine("ok");

        Write("testing local exceptions with lambda callback... ");
        Flush();
        {
            Test.TestIntfPrx i = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ExceptionCallback cb = new ExceptionCallback();

            i.begin_ice_isA("::Test::TestIntf").whenCompleted(
                (bool r) =>
                {
                    cb.isA(r);
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();

            i.begin_ice_ping().whenCompleted(
                () =>
                {
                    cb.ping();
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();

            i.begin_ice_id().whenCompleted(
                (string id) =>
                {
                    cb.id(id);
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();

            i.begin_ice_ids().whenCompleted(
                (string[] ids) =>
                {
                    cb.ids(ids);
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();

            if(!collocated)
            {
                i.begin_ice_getConnection().whenCompleted(
                    (Ice.Connection conn) =>
                    {
                        cb.connection(conn);
                    },
                    (Ice.Exception ex) =>
                    {
                        cb.ex(ex);
                    });
                cb.check();
            }

            i.begin_op().whenCompleted(
                () =>
                {
                    cb.op();
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("testing exception callback... ");
        Flush();
        {
            Test.TestIntfPrx i = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ExceptionCallback cb = new ExceptionCallback();

            i.begin_ice_isA("::Test::TestIntf").whenCompleted(cb.ex);
            cb.check();

            i.begin_op().whenCompleted(cb.ex);
            cb.check();

            i.begin_opWithResult().whenCompleted(cb.ex);
            cb.check();

            i.begin_opWithUE().whenCompleted(cb.ex);
            cb.check();

            // Ensures no exception is called when response is received
            p.begin_ice_isA("::Test::TestIntf").whenCompleted(cb.noEx);
            p.begin_op().whenCompleted(cb.noEx);
            p.begin_opWithResult().whenCompleted(cb.noEx);

            // If response is a user exception, it should be received.
            p.begin_opWithUE().whenCompleted(cb.opWithUE);
            cb.check();
        }
        WriteLine("ok");

        Write("testing exception with async task... ");
        Flush();
        {
            Test.TestIntfPrx i = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ExceptionCallback cb = new ExceptionCallback();

            try
            {
                i.ice_isAAsync("::Test::TestIntf").Wait();
                test(false);
            }
            catch(AggregateException)
            {
            }

            try
            {
                i.opAsync().Wait();
                test(false);
            }
            catch(AggregateException)
            {
            }

            try
            {
                i.opWithResultAsync().Wait();
                test(false);
            }
            catch(AggregateException)
            {
            }

            try
            {
                i.opWithUEAsync().Wait();
                test(false);
            }
            catch(AggregateException)
            {
            }

            // Ensures no exception is called when response is received
            test(p.ice_isAAsync("::Test::TestIntf").Result);
            p.opAsync().Wait();
            p.opWithResultAsync().Wait();

            // If response is a user exception, it should be received.
            try
            {
                p.opWithUEAsync().Wait();
                test(false);
            }
            catch(AggregateException ae)
            {
                ae.Handle((ex) =>
                {
                    return ex is Test.TestIntfException;
                });
            }
        }
        WriteLine("ok");

        Write("testing lambda exception callback... ");
        Flush();
        {
            Test.TestIntfPrx i = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ExceptionCallback cb = new ExceptionCallback();

            i.begin_ice_isA("::Test::TestIntf").whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();

            i.begin_op().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });

            cb.check();

            i.begin_opWithResult().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();

            i.begin_opWithUE().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                });
            cb.check();

            // Ensures no exception is called when response is received
            p.begin_ice_isA("::Test::TestIntf").whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.noEx(ex);
                });
            p.begin_op().whenCompleted(
                 (Ice.Exception ex) =>
                {
                    cb.noEx(ex);
                });
            p.begin_opWithResult().whenCompleted(
                 (Ice.Exception ex) =>
                {
                    cb.noEx(ex);
                });

            // If response is a user exception, it should be received.
            p.begin_opWithUE().whenCompleted(
                 (Ice.Exception ex) =>
                {
                    cb.opWithUE(ex);
                });
            cb.check();
        }
        WriteLine("ok");

        Write("testing sent callback... ");
        Flush();
        {
            SentCallback cb = new SentCallback();

            p.begin_ice_isA("").whenCompleted(cb.isA, cb.ex).whenSent(cb.sent);
            cb.check();

            p.begin_ice_ping().whenCompleted(cb.ping, cb.ex).whenSent(cb.sent);
            cb.check();

            p.begin_ice_id().whenCompleted(cb.id, cb.ex).whenSent(cb.sent);
            cb.check();

            p.begin_ice_ids().whenCompleted(cb.ids, cb.ex).whenSent(cb.sent);
            cb.check();

            p.begin_op().whenCompleted(cb.op, cb.ex).whenSent(cb.sent);
            cb.check();

            p.begin_op(cb.opAsync, null).whenSent(cb.sentAsync);
            cb.check();

            p.begin_op().whenCompleted(cb.ex).whenSent(cb.sent);
            cb.check();

            List<SentCallback> cbs = new List<SentCallback>();
            byte[] seq = new byte[10024];
            (new System.Random()).NextBytes(seq);
            testController.holdAdapter();
            try
            {
                Ice.AsyncResult r;
                do
                {
                    SentCallback cb2 = new SentCallback();
                    r = p.begin_opWithPayload(seq).whenCompleted(cb2.ex).whenSent(cb2.sent);
                    cbs.Add(cb2);
                }
                while(r.sentSynchronously());
            }
            finally
            {
                testController.resumeAdapter();
            }
            foreach(SentCallback cb3 in cbs)
            {
                cb3.check();
            }
        }
        WriteLine("ok");

        Write("testing progress callback... ");
        Flush();
        {
            {
                SentCallback cb = new SentCallback();

                System.Threading.Tasks.Task t = p.ice_isAAsync("",
                    progress: new Progress(sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
                t.Wait();

                t = p.ice_pingAsync(
                    progress: new Progress(sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
                t.Wait();

                t = p.ice_idAsync(
                    progress: new Progress(sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
                t.Wait();

                t = p.ice_idsAsync(
                    progress: new Progress(sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
                t.Wait();

                t = p.opAsync(
                    progress: new Progress(sentSynchronously =>
                    {
                        cb.sent(sentSynchronously);
                    }));
                cb.check();
                t.Wait();
            }

            List<Task> tasks = new List<Task>();
            byte[] seq = new byte[10024];
            (new System.Random()).NextBytes(seq);
            testController.holdAdapter();
            try
            {
                Task t = null;
                ProgresCallback cb;
                do
                {
                    cb = new ProgresCallback();
                    t = p.opWithPayloadAsync(seq, progress: cb);
                    tasks.Add(t);
                }
                while(cb.SentSynchronously);
            }
            finally
            {
                testController.resumeAdapter();
            }
            foreach(Task t in tasks)
            {
                t.Wait();
            }
        }
        WriteLine("ok");

        Write("testing lambda sent callback... ");
        Flush();
        {
            SentCallback cb = new SentCallback();

            p.begin_ice_isA("").whenCompleted(
                (bool r) =>
                {
                    cb.isA(r);
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();

            p.begin_ice_ping().whenCompleted(
                () =>
                {
                    cb.ping();
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();

            p.begin_ice_id().whenCompleted(
                (string id) =>
                {
                    cb.id(id);
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();

            p.begin_ice_ids().whenCompleted(
                (string[] ids) =>
                {
                    cb.ids(ids);
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();

            p.begin_op().whenCompleted(
                () =>
                {
                    cb.op();
                },
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();

            p.begin_op(cb.opAsync, null).whenSent(
                (Ice.AsyncResult r) =>
                {
                    cb.sentAsync(r);
                });
            cb.check();

            p.begin_op().whenCompleted(
                (Ice.Exception ex) =>
                {
                    cb.ex(ex);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    cb.sent(sentSynchronously);
                });
            cb.check();

            List<SentCallback> cbs = new List<SentCallback>();
            byte[] seq = new byte[10024];
            (new System.Random()).NextBytes(seq);
            testController.holdAdapter();
            try
            {
                Ice.AsyncResult r;
                do
                {
                    SentCallback cb2 = new SentCallback();
                    r = p.begin_opWithPayload(seq).whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb2.ex(ex);
                        }
                    ).whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb2.sent(sentSynchronously);
                        });
                    cbs.Add(cb2);
                }
                while(r.sentSynchronously());
            }
            finally
            {
                testController.resumeAdapter();
            }
            foreach(SentCallback cb3 in cbs)
            {
                cb3.check();
            }
        }
        WriteLine("ok");

        Write("testing illegal arguments... ");
        Flush();
        {
            Ice.AsyncResult result;

            result = p.begin_op();
            p.end_op(result);
            try
            {
                p.end_op(result);
                test(false);
            }
            catch(System.ArgumentException)
            {
            }

            result = p.begin_op();
            try
            {
                p.end_opWithResult(result);
                test(false);
            }
            catch(System.ArgumentException)
            {
            }

            try
            {
                p.end_op(null);
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }
        WriteLine("ok");

        Write("testing unexpected exceptions from callback... ");
        Flush();
        {
            Test.TestIntfPrx q = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ThrowType[] throwEx = new ThrowType[]{ ThrowType.LocalException, ThrowType.UserException,
                                                   ThrowType.OtherException };

            for(int i = 0; i < 3; ++i)
            {
                Thrower cb = new Thrower(throwEx[i]);

                p.begin_op(cb.opAsync, null);
                cb.check();

                p.begin_op().whenCompleted(cb.op, null);
                cb.check();

                q.begin_op().whenCompleted(cb.op, cb.ex);
                cb.check();

                p.begin_op().whenCompleted(cb.noOp, cb.ex).whenSent(cb.sent);
                cb.check();

                q.begin_op().whenCompleted(cb.ex);
                cb.check();
            }
        }
        WriteLine("ok");

        Write("testing unexpected exceptions from callback with lambda... ");
        Flush();
        {
            Test.TestIntfPrx q = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            ThrowType[] throwEx = new ThrowType[]{ ThrowType.LocalException, ThrowType.UserException,
                                                   ThrowType.OtherException };

            for(int i = 0; i < 3; ++i)
            {
                Thrower cb = new Thrower(throwEx[i]);

                p.begin_op().whenCompleted(
                    () =>
                    {
                        cb.op();
                    }, null);
                cb.check();

                q.begin_op().whenCompleted(
                    () =>
                    {
                        cb.op();
                    },
                    (Ice.Exception ex) =>
                    {
                        cb.ex(ex);
                    });
                cb.check();

                p.begin_op().whenCompleted(
                    () =>
                    {
                        cb.noOp();
                    },
                    (Ice.Exception ex) =>
                    {
                        cb.ex(ex);
                    }
                ).whenSent(
                    (bool sentSynchronously) =>
                    {
                        cb.sent(sentSynchronously);
                    });
                cb.check();

                q.begin_op().whenCompleted(
                    (Ice.Exception ex) =>
                    {
                        cb.ex(ex);
                    });
                cb.check();
            }
        }
        WriteLine("ok");

        Write("testing batch requests with proxy... ");
        Flush();
        {
            test(p.ice_batchOneway().begin_ice_flushBatchRequests().sentSynchronously());

            Cookie cookie = new Cookie(5);

            {
                //
                // AsyncResult.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                FlushCallback cb = new FlushCallback(cookie);
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(cb.completedAsync, cookie);
                r.whenSent(cb.sentAsync);
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
                test(p.waitForBatch(2));
            }

            if(p.ice_getConnection() != null)
            {
                //
                // AsyncResult exception.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                FlushCallback cb = new FlushCallback(cookie);
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(cb.completedAsync, cookie);
                r.whenSent(cb.sentAsync);
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
                test(p.waitForBatch(1));
            }

            {
                //
                // Type-safe.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests();
                r.whenCompleted(cb.exception);
                r.whenSent(cb.sent);
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
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
                b1.ice_getConnection().close(false);
                FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests();
                r.whenCompleted(cb.exception);
                r.whenSent(cb.sent);
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
                test(p.waitForBatch(1));
            }
        }
        WriteLine("ok");

        Write("testing batch requests with proxy and async tasks... ");
        Flush();
        {
            {
                CallbackBase cb = new CallbackBase();
                System.Threading.Tasks.Task t = p.ice_batchOneway().ice_flushBatchRequestsAsync(
                    progress: new Progress<bool>(sentSynchronously =>
                    {
                        test(sentSynchronously);
                        cb.called();
                    }));
                cb.check();
                t.Wait();
            }

            {
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                CallbackBase cb = new CallbackBase();
                System.Threading.Tasks.Task t = b1.ice_flushBatchRequestsAsync(
                    progress: new Progress<bool>(sentSynchronoully =>
                    {
                        cb.called();
                    }));

                cb.check();
                t.Wait();
                test(p.waitForBatch(2));
            }

            if(p.ice_getConnection() != null)
            {
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                CallbackBase cb = new CallbackBase();
                System.Threading.Tasks.Task t = b1.ice_flushBatchRequestsAsync(
                    progress:new Progress<bool>(sentSynchronoully =>
                    {
                        test(!sentSynchronoully);
                        cb.called();
                    }));
                cb.check();
                t.Wait();
                test(p.waitForBatch(1));
            }
        }
        WriteLine("ok");

        Write("testing batch requests with proxy and lambda... ");
        Flush();
        {
            test(p.ice_batchOneway().begin_ice_flushBatchRequests().sentSynchronously());

            Cookie cookie = new Cookie(5);

            {
                //
                // AsyncResult.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                FlushCallback cb = new FlushCallback(cookie);
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(
                    (Ice.AsyncResult result) =>
                    {
                        cb.completedAsync(result);
                    }, cookie);
                r.whenSent(
                    (Ice.AsyncResult result) =>
                    {
                        cb.sentAsync(result);
                    });
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
                test(p.waitForBatch(2));

                FlushCallback cb2 = new FlushCallback(cookie);
                Ice.AsyncResult r2 = b1.begin_ice_flushBatchRequests(
                    (Ice.AsyncResult result) =>
                    {
                        cb2.completedAsync(result);
                    }, cookie);
                r2.whenSent(
                    (Ice.AsyncResult result) =>
                    {
                        cb2.sentAsync(result);
                    });
                cb2.check();
                test(r2.isSent());
                test(r2.IsCompleted);
            }

            if(p.ice_getConnection() != null)
            {
                //
                // AsyncResult exception.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(false);
                FlushCallback cb = new FlushCallback(cookie);
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests(
                    (Ice.AsyncResult result) =>
                    {
                        cb.completedAsync(result);
                    }, cookie);
                r.whenSent(
                    (Ice.AsyncResult result) =>
                    {
                        cb.sentAsync(result);
                    });
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
                test(p.waitForBatch(1));
            }

            {
                //
                // Type-safe.
                //
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = (TestIntfPrx)p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests();
                r.whenCompleted(
                    (Ice.Exception ex) =>
                    {
                        cb.exception(ex);
                    });
                r.whenSent(
                    (bool sentSynchronously) =>
                    {
                        cb.sent(sentSynchronously);
                    });
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
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
                b1.ice_getConnection().close(false);
                FlushCallback cb = new FlushCallback();
                Ice.AsyncResult r = b1.begin_ice_flushBatchRequests();
                r.whenCompleted(
                    (Ice.Exception ex) =>
                    {
                        cb.exception(ex);
                    });
                r.whenSent(
                    (bool sentSynchronously) =>
                    {
                        cb.sent(sentSynchronously);
                    });
                cb.check();
                test(r.isSent());
                test(r.IsCompleted);
                test(p.waitForBatch(1));
            }
        }
        WriteLine("ok");

        if(p.ice_getConnection() != null)
        {
            Write("testing batch requests with connection... ");
            Flush();
            {
                {
                    //
                    // Async task.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                     p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    SentCallback cb = new SentCallback();
                    Task t = b1.ice_getConnection().flushBatchRequestsAsync(
                        progress:new Progress(
                            sentSyncrhonously =>
                            {
                                cb.sent(sentSyncrhonously);
                            }));

                    cb.check();
                    test(t.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Async task exception.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    Task t = b1.ice_getConnection().flushBatchRequestsAsync(
                        progress: new Progress(
                            sentSynchronously =>
                            {
                                test(false);
                            }));
                    test(t.IsFaulted);
                    test(p.opBatchCount() == 0);
                }

                Cookie cookie = new Cookie(5);

                {
                    //
                    // AsyncResult.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                     p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(cb.completedAsync, cookie);
                    r.whenSent(cb.sentAsync);
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // AsyncResult exception.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushExCallback cb = new FlushExCallback(cookie);
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(cb.completedAsync, cookie);
                    r.whenSent(cb.sentAsync);
                    cb.check();
                    test(!r.isSent());
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // Type-safe.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests();
                    r.whenCompleted(cb.exception);
                    r.whenSent(cb.sent);
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Type-safe exception.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushExCallback cb = new FlushExCallback();
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests();
                    r.whenCompleted(cb.exception);
                    r.whenSent(cb.sent);
                    cb.check();
                    test(!r.isSent());
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }
            }
            WriteLine("ok");
        }

        if(p.ice_getConnection() != null)
        {
            Write("testing batch requests with connection and lambda... ");
            Flush();
            {
                Cookie cookie = new Cookie(5);

                {
                    //
                    // AsyncResult.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                        (Ice.AsyncResult result) =>
                        {
                            cb.completedAsync(result);
                        }, cookie);
                    r.whenSent(
                        (Ice.AsyncResult result) =>
                        {
                            cb.sentAsync(result);
                        });
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // AsyncResult exception.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushExCallback cb = new FlushExCallback(cookie);
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests(
                        (Ice.AsyncResult result) =>
                        {
                            cb.completedAsync(result);
                        }, cookie);
                    r.whenSent(
                        (Ice.AsyncResult result) =>
                        {
                            cb.sentAsync(result);
                        });
                    cb.check();
                    test(!r.isSent());
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // Type-safe.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests();
                    r.whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    r.whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Type-safe exception.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushExCallback cb = new FlushExCallback();
                    Ice.AsyncResult r = b1.ice_getConnection().begin_flushBatchRequests();
                    r.whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    r.whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(!r.isSent());
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }
            }
            WriteLine("ok");

            Write("testing batch requests with communicator... ");
            Flush();
            {
                {
                    //
                    // Async task - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();

                    SentCallback cb = new SentCallback();
                    Task t = communicator.flushBatchRequestsAsync(
                        progress: new Progress(
                            sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                    cb.check();
                    test(t.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Async task exception - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    SentCallback cb = new SentCallback();
                    Task t = communicator.flushBatchRequestsAsync(
                        progress:new Progress(
                            sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                    cb.check(); // Exceptions are ignored!
                    test(t.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // Async task - 2 connections.
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

                    SentCallback cb = new SentCallback();
                    Task t = communicator.flushBatchRequestsAsync(
                        new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                    cb.check();
                    test(t.IsCompleted);
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(false);
                    SentCallback cb = new SentCallback();
                    Task t = communicator.flushBatchRequestsAsync(
                        new Progress(
                            sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                    cb.check(); // Exceptions are ignored!
                    test(t.IsCompleted);
                    test(p.waitForBatch(1));
                }

                {
                    //
                    // Async task exception - 2 connections - 2 failures.
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
                    b1.ice_getConnection().close(false);
                    b2.ice_getConnection().close(false);
                    SentCallback cb = new SentCallback();
                    Task t = communicator.flushBatchRequestsAsync(
                        new Progress(
                            sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                    cb.check(); // Exceptions are ignored!
                    test(t.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                Cookie cookie = new Cookie(5);

                {
                    //
                    // AsyncResult - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(cb.completedAsync, cookie);
                    r.whenSent(cb.sentAsync);
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // AsyncResult exception - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(cb.completedAsync, cookie);
                    r.whenSent(cb.sentAsync);
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // AsyncResult - 2 connections.
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
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(cb.completedAsync, cookie);
                    r.whenSent(cb.sentAsync);
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(cb.completedAsync, cookie);
                    r.whenSent(cb.sentAsync);
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.waitForBatch(1));
                }

                {
                    //
                    // AsyncResult exception - 2 connections - 2 failures.
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
                    b1.ice_getConnection().close(false);
                    b2.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(cb.completedAsync, cookie);
                    r.whenSent(cb.sentAsync);
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // Type-safe - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(cb.exception);
                    r.whenSent(cb.sent);
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
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
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(cb.exception);
                    r.whenSent(cb.sent);
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
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
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(cb.exception);
                    r.whenSent(cb.sent);
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
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
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(cb.exception);
                    r.whenSent(cb.sent);
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
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
                    b1.ice_getConnection().close(false);
                    b2.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(cb.exception);
                    r.whenSent(cb.sent);
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }
            }
            WriteLine("ok");

            Write("testing batch requests with communicator with lambda... ");
            Flush();
            {
                Cookie cookie = new Cookie(5);

                {
                    //
                    // AsyncResult - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        (Ice.AsyncResult result) =>
                        {
                            cb.completedAsync(result);
                        }, cookie);
                    r.whenSent(
                        (Ice.AsyncResult result) =>
                        {
                            cb.sentAsync(result);
                        });
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // AsyncResult exception - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        (Ice.AsyncResult result) =>
                        {
                            cb.completedAsync(result);
                        }, cookie);
                    r.whenSent(
                        (Ice.AsyncResult result) =>
                        {
                            cb.sentAsync(result);
                        });
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // AsyncResult - 2 connections.
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
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        (Ice.AsyncResult result) =>
                        {
                            cb.completedAsync(result);
                        }, cookie);
                    r.whenSent(
                        (Ice.AsyncResult result) =>
                        {
                            cb.sentAsync(result);
                        });
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        (Ice.AsyncResult result) =>
                        {
                            cb.completedAsync(result);
                        }, cookie);
                    r.whenSent(
                        (Ice.AsyncResult result) =>
                        {
                            cb.sentAsync(result);
                        });
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.waitForBatch(1));
                }

                {
                    //
                    // AsyncResult exception - 2 connections - 2 failures.
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
                    b1.ice_getConnection().close(false);
                    b2.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback(cookie);
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests(
                        (Ice.AsyncResult result) =>
                        {
                            cb.completedAsync(result);
                        }, cookie);
                    r.whenSent(
                        (Ice.AsyncResult result) =>
                        {
                            cb.sentAsync(result);
                        });
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // Type-safe - 1 connection.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                         p.ice_getIdentity()).ice_batchOneway());
                    b1.opBatch();
                    b1.opBatch();
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    r.whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
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
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    r.whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // 2 connections.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b1.opBatch();
                    b2.opBatch();
                    b2.opBatch();
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    r.whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(r.isSent());
                    test(r.IsCompleted);
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
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    r.whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.waitForBatch(1));
                }

                {
                    //
                    // Exception - 2 connections - 2 failures.
                    //
                    // The sent callback should be invoked even if all connections fail.
                    //
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    TestIntfPrx b2 = TestIntfPrxHelper.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(false);
                    b2.ice_getConnection().close(false);
                    FlushCallback cb = new FlushCallback();
                    Ice.AsyncResult r = communicator.begin_flushBatchRequests();
                    r.whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.exception(ex);
                        });
                    r.whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(r.isSent()); // Exceptions are ignored!
                    test(r.IsCompleted);
                    test(p.opBatchCount() == 0);
                }
            }
            WriteLine("ok");
        }

        Write("testing async/await... ");
        Flush();
        Func<Task> task = async () =>
        {
            try
            {
                await p.opAsync();

                var r = await p.opWithResultAsync();
                test(r == 15);

                try
                {
                    await p.opWithUEAsync();
                }
                catch(TestIntfException)
                {
                }

                // Operations implemented with amd and async.
                await p.opAsyncDispatchAsync();

                r = await p.opWithResultAsyncDispatchAsync();
                test(r == 15);

                try
                {
                    await p.opWithUEAsyncDispatchAsync();
                    test(false);
                }
                catch(TestIntfException)
                {
                }
            }
            catch(Ice.OperationNotExistException)
            {
                // Expected with cross testing, this opXxxAsyncDispatch methods are C# only.
            }
        };
        task().Wait();
        WriteLine("ok");

        if(p.ice_getConnection() != null)
        {
            Write("testing async Task cancellation... ");
            Flush();
            {
                var cs1 = new CancellationTokenSource();
                var cs2 = new CancellationTokenSource();
                var cs3 = new CancellationTokenSource();
                Task t1;
                Task t2;
                Task t3;
                try
                {
                    testController.holdAdapter();
                    ProgresCallback cb = null;
                    byte[] seq = new byte[10024];
                    for(int i = 0; i < 200; ++i) // 2MB
                    {
                        cb = new ProgresCallback();
                        p.opWithPayloadAsync(seq, progress: cb);
                    }

                    test(!cb.Sent);

                    t1 = p.ice_pingAsync(cancel: cs1.Token);
                    t2 = p.ice_pingAsync(cancel: cs2.Token);
                    cs3.Cancel();
                    t3 = p.ice_pingAsync(cancel: cs3.Token);
                    cs1.Cancel();
                    cs2.Cancel();
                    try
                    {
                        t1.Wait();
                        test(false);
                    }
                    catch(AggregateException ae)
                    {
                        ae.Handle(ex =>
                        {
                            return ex is Ice.InvocationCanceledException;
                        });
                    }
                    try
                    {
                        t2.Wait();
                        test(false);
                    }
                    catch(AggregateException ae)
                    {
                        ae.Handle(ex =>
                        {
                            return ex is Ice.InvocationCanceledException;
                        });
                    }

                    try
                    {
                        t3.Wait();
                        test(false);
                    }
                    catch(AggregateException ae)
                    {
                        ae.Handle(ex =>
                        {
                            return ex is Ice.InvocationCanceledException;
                        });
                    }

                }
                finally
                {
                    testController.resumeAdapter();
                    p.ice_ping();
                }
            }
            WriteLine("ok");
        }

        Write("testing AsyncResult operations... ");
        Flush();
        {
            {
                testController.holdAdapter();
                Ice.AsyncResult r1;
                Ice.AsyncResult r2;
                try
                {
                    r1 = p.begin_op();
                    byte[] seq = new byte[10024];
                    (new System.Random()).NextBytes(seq);
                    while((r2 = p.begin_opWithPayload(seq)).sentSynchronously());

                    if(p.ice_getConnection() != null)
                    {
                        test(r1.sentSynchronously() && r1.isSent() && !r1.isCompleted_() ||
                             !r1.sentSynchronously() && !r1.isCompleted_());

                        test(!r2.sentSynchronously() && !r2.isCompleted_());

                        test(!r1.IsCompleted && !r1.CompletedSynchronously);
                        test(!r2.IsCompleted && !r2.CompletedSynchronously);
                    }
                }
                finally
                {
                    testController.resumeAdapter();
                }

                WaitHandle w1 = r1.AsyncWaitHandle;
                WaitHandle w2 = r2.AsyncWaitHandle;

                r1.waitForSent();
                test(r1.isSent());

                r2.waitForSent();
                test(r2.isSent());

                r1.waitForCompleted();
                test(r1.isCompleted_());
                w1.WaitOne();

                r2.waitForCompleted();
                test(r2.isCompleted_());
                w2.WaitOne();

                test(r1.getOperation().Equals("op"));
                test(r2.getOperation().Equals("opWithPayload"));
            }

            {
                Ice.AsyncResult r;

                //
                // Twoway
                //
                r = p.begin_ice_ping();
                test(r.getOperation().Equals("ice_ping"));
                test(r.getConnection() == null); // Expected
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == p);
                p.end_ice_ping(r);

                Test.TestIntfPrx p2;

                //
                // Oneway
                //
                p2 = p.ice_oneway() as Test.TestIntfPrx;
                r = p2.begin_ice_ping();
                test(r.getOperation().Equals("ice_ping"));
                test(r.getConnection() == null); // Expected
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == p2);

                //
                // Batch request via proxy
                //
                p2 = p.ice_batchOneway() as Test.TestIntfPrx;
                p2.ice_ping();
                r = p2.begin_ice_flushBatchRequests();
                test(r.getConnection() == null); // Expected
                test(r.getCommunicator() == communicator);
                test(r.getProxy() == p2);
                p2.end_ice_flushBatchRequests(r);

                if(p.ice_getConnection() != null)
                {
                    //
                    // Batch request via connection
                    //
                    Ice.Connection con = p.ice_getConnection();
                    p2 = p.ice_batchOneway() as Test.TestIntfPrx;
                    p2.ice_ping();
                    r = con.begin_flushBatchRequests();
                    test(r.getConnection() == con);
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == null); // Expected
                    con.end_flushBatchRequests(r);

                    //
                    // Batch request via communicator
                    //
                    p2 = p.ice_batchOneway() as Test.TestIntfPrx;
                    p2.ice_ping();
                    r = communicator.begin_flushBatchRequests();
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == null); // Expected
                    communicator.end_flushBatchRequests(r);
                }
            }

            if(p.ice_getConnection() != null)
            {
                Ice.AsyncResult r1 = null;
                Ice.AsyncResult r2 = null;
                testController.holdAdapter();
                try
                {
                    Ice.AsyncResult r = null;
                    byte[] seq = new byte[10024];
                    for(int i = 0; i < 200; ++i) // 2MB
                    {
                        r = p.begin_opWithPayload(seq);
                    }

                    test(!r.isSent());

                    r1 = p.begin_ice_ping();
                    r2 = p.begin_ice_id();
                    r1.cancel();
                    r2.cancel();
                    try
                    {
                        p.end_ice_ping(r1);
                        test(false);
                    }
                    catch(Ice.InvocationCanceledException)
                    {
                    }
                    try
                    {
                        p.end_ice_id(r2);
                        test(false);
                    }
                    catch(Ice.InvocationCanceledException)
                    {
                    }

                }
                finally
                {
                    testController.resumeAdapter();
                    p.ice_ping();
                    test(!r1.isSent() && r1.isCompleted_());
                    test(!r2.isSent() && r2.isCompleted_());
                }


                testController.holdAdapter();
                try
                {
                    r1 = p.begin_op();
                    r2 = p.begin_ice_id();
                    r1.waitForSent();
                    r2.waitForSent();
                    r1.cancel();
                    r2.cancel();
                    try
                    {
                        p.end_op(r1);
                        test(false);
                    }
                    catch(Ice.InvocationCanceledException)
                    {
                    }
                    try
                    {
                        p.end_ice_id(r2);
                        test(false);
                    }
                    catch(Ice.InvocationCanceledException)
                    {
                    }
                }
                finally
                {
                    testController.resumeAdapter();
                }
            }
        }
        WriteLine("ok");

        if(p.ice_getConnection() != null)
        {
            Write("testing close connection with sending queue... ");
            Flush();
            {
                byte[] seq = new byte[1024 * 10];

                //
                // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
                // The goal is to make sure that none of the opWithPayload fail even if the server closes
                // the connection gracefully in between.
                //
                int maxQueue = 2;
                bool done = false;
                while(!done && maxQueue < 50)
                {
                    done = true;
                    p.ice_ping();
                    List<Task> results = new List<Task>();
                    for(int i = 0; i < maxQueue; ++i)
                    {
                        results.Add(p.opWithPayloadAsync(seq));
                    }

                    ProgresCallback cb = new ProgresCallback();
                    p.closeAsync(false, progress:cb);

                    if(!cb.SentSynchronously)
                    {
                        for(int i = 0; i < maxQueue; i++)
                        {
                            cb = new ProgresCallback();
                            Task t = p.opWithPayloadAsync(seq, progress:cb);
                            results.Add(t);
                            if(cb.SentSynchronously)
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
                    foreach(Task q in results)
                    {
                        q.Wait();
                    }
                }
            }
            WriteLine("ok");
        }

        p.shutdown();
    }
}
