# Copyright (c) ZeroC, Inc.

import Ice
import IcePy
import Test
import sys
import time
import threading
import random


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class PingReplyI(Test.PingReply):
    def __init__(self):
        self._received = False

    def reply(self, current):
        self._received = True

    def checkReceived(self):
        return self._received


class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def check(self):
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

    def exception(self, ex):
        test(False)


class ResponseCallback(CallbackBase):
    def isA(self, r):
        test(r)
        self.called()

    def ping(self):
        self.called()

    def id(self, id):
        test(id == "::Test::TestIntf")
        self.called()

    def ids(self, ids):
        test(len(ids) == 2)
        self.called()

    def connection(self, conn):
        test(conn is not None)
        test(type(conn) is IcePy.Connection)
        self.called()

    def op(self):
        self.called()

    def opWithResult(self, r):
        test(r == 15)
        self.called()

    def opWithUE(self, ex):
        try:
            raise ex
        except Test.TestIntfException:
            self.called()
        except Exception:
            test(False)

    def ex(self, ex):
        pass


class ResponseCallbackWC(CallbackBase):
    def __init__(self, cookie):
        CallbackBase.__init__(self)
        self._cookie = cookie

    def isA(self, r, cookie):
        test(cookie == self._cookie)
        test(r)
        self.called()

    def ping(self, cookie):
        test(cookie == self._cookie)
        self.called()

    def id(self, id, cookie):
        test(cookie == self._cookie)
        test(id == "::Test::TestIntf")
        self.called()

    def ids(self, ids, cookie):
        test(cookie == self._cookie)
        test(len(ids) == 2)
        self.called()

    def connection(self, conn, cookie):
        test(cookie == self._cookie)
        test(conn is not None)
        self.called()

    def op(self, cookie):
        test(cookie == self._cookie)
        self.called()

    def opWithResult(self, r, cookie):
        test(cookie == self._cookie)
        test(r == 15)
        self.called()

    def opWithUE(self, ex, cookie):
        test(cookie == self._cookie)
        try:
            raise ex
        except Test.TestIntfException:
            self.called()
        except Exception:
            test(False)

    def ex(self, ex, cookie):
        pass


class ExceptionCallback(CallbackBase):
    def response(self, *args):
        test(False)

    def nullResponse(self, *args):
        pass

    def opWithUE(self, ex):
        test(isinstance(ex, Test.TestIntfException))
        self.called()

    def ex(self, ex):
        test(isinstance(ex, Ice.NoEndpointException))
        self.called()

    def noEx(self, ex):
        test(False)


class ExceptionCallbackWC(CallbackBase):
    def __init__(self, cookie):
        CallbackBase.__init__(self)
        self._cookie = cookie

    def response(self, *args):
        test(False)

    def nullResponse(self, *args):
        pass

    def opWithUE(self, ex, cookie):
        test(cookie == self._cookie)
        test(isinstance(ex, Test.TestIntfException))
        self.called()

    def ex(self, ex, cookie):
        test(cookie == self._cookie)
        test(isinstance(ex, Ice.NoEndpointException))
        self.called()

    def noEx(self, ex, cookie):
        test(False)


class SentCallback(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)
        self._thread = threading.current_thread()

    def response(self, *args):
        pass

    def ex(self, ex):
        pass

    def sent(self, sentSynchronously):
        test(
            (sentSynchronously and self._thread == threading.current_thread())
            or (not sentSynchronously and self._thread != threading.current_thread())
        )
        self.called()


class SentCallbackWC(CallbackBase):
    def __init__(self, cookie):
        CallbackBase.__init__(self)
        self._thread = threading.current_thread()
        self._cookie = cookie

    def response(self, *args):
        pass

    def ex(self, ex, cookie):
        pass

    def sent(self, sentSynchronously, cookie):
        test(cookie == self._cookie)
        test(
            (sentSynchronously and self._thread == threading.current_thread())
            or (not sentSynchronously and self._thread != threading.current_thread())
        )
        self.called()


class FlushCallback(CallbackBase):
    def __init__(self, cookie=None):
        CallbackBase.__init__(self)
        self._thread = threading.current_thread()
        self._cookie = cookie

    def exception(self, ex):
        test(False)

    def exceptionWC(self, ex, cookie):
        test(False)

    def sent(self, sentSynchronously):
        test(
            (sentSynchronously and self._thread == threading.current_thread())
            or (not sentSynchronously and self._thread != threading.current_thread())
        )
        self.called()

    def sentWC(self, sentSynchronously, cookie):
        test(
            (sentSynchronously and self._thread == threading.current_thread())
            or (not sentSynchronously and self._thread != threading.current_thread())
        )
        test(cookie == self._cookie)
        self.called()


class FlushExCallback(CallbackBase):
    def __init__(self, cookie=None):
        CallbackBase.__init__(self)
        self._cookie = cookie

    def exception(self, ex):
        self.called()

    def exceptionWC(self, ex, cookie):
        test(cookie == self._cookie)
        self.called()

    def sent(self, sentSynchronously):
        test(False)

    def sentWC(self, sentSynchronously, cookie):
        test(False)


class FutureDoneCallback(CallbackBase):
    def isA(self, f):
        test(f.result())
        self.called()

    def ping(self, f):
        self.called()

    def id(self, f):
        test(f.result() == "::Test::TestIntf")
        self.called()

    def ids(self, f):
        test(len(f.result()) == 2)
        self.called()

    def connection(self, f):
        test(f.result() is not None)
        self.called()

    def op(self, f):
        self.called()

    def opWithResult(self, f):
        test(f.result() == 15)
        self.called()

    def opWithUE(self, f):
        try:
            f.result()
            test(False)
        except Test.TestIntfException:
            self.called()
        except Exception:
            test(False)


class FutureExceptionCallback(CallbackBase):
    def opWithUE(self, f):
        test(isinstance(f.exception(), Test.TestIntfException))
        self.called()

    def ex(self, f):
        test(isinstance(f.exception(), Ice.NoEndpointException))
        self.called()

    def noEx(self, f):
        test(f.exception() is None)


class FutureSentCallback(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)
        self._thread = threading.current_thread()

    def sent(self, f, sentSynchronously):
        self.called()

    def sentAsync(self, f, sentSynchronously):
        test(self._thread != threading.current_thread())
        self.called()


class FutureFlushCallback(CallbackBase):
    def __init__(self, cookie=None):
        CallbackBase.__init__(self)
        self._thread = threading.current_thread()
        self._cookie = cookie

    def sent(self, f, sentSynchronously):
        self.called()

    def sentAsync(self, f, sentSynchronously):
        test(self._thread != threading.current_thread())
        self.called()


class FutureFlushExCallback(CallbackBase):
    def __init__(self, cookie=None):
        CallbackBase.__init__(self)
        self._cookie = cookie

    def exception(self, f):
        self.called()

    def sent(self, f, sentSynchronously):
        test(False)


LocalException = 0
UserException = 1
OtherException = 2


def throwEx(t):
    if t == LocalException:
        raise Ice.ObjectNotExistException()
    elif t == UserException:
        raise Test.TestIntfException()
    elif t == OtherException:
        raise RuntimeError()
    else:
        test(False)


class Thrower(CallbackBase):
    def __init__(self, t):
        CallbackBase.__init__(self)
        self._t = t

    def op(self):
        self.called()
        throwEx(self._t)

    def opWC(self, cookie):
        self.called()
        throwEx(self._t)

    def noOp(self):
        pass

    def noOpWC(self, cookie):
        pass

    def ex(self, ex):
        self.called()
        throwEx(self._t)

    def exWC(self, ex, cookie):
        self.called()
        throwEx(self._t)

    def noEx(self, ex):
        test(False)

    def noExWC(self, ex, cookie):
        test(False)

    def sent(self, ss):
        self.called()
        throwEx(self._t)

    def sentWC(self, ss, cookie):
        self.called()
        throwEx(self._t)


def allTests(helper, communicator, collocated):
    p = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint(num=0)}")

    if p.ice_getConnection():
        sys.stdout.write("testing close connection... ")
        sys.stdout.flush()

        #
        # Local case: begin a request, close the connection gracefully, and make sure it waits
        # for the request to complete.
        #
        p = p.ice_connectionId("CloseGracefully")  # Start with a new connection.
        con = p.ice_getConnection()
        cb = CallbackBase()
        con.setCloseCallback(lambda c: cb.called())
        f = p.startDispatchAsync()
        f.sent()  # Ensure the request was sent before we close the connection.
        con.close()

        # give time for startDispatch to start in the server before we call finishDispatch
        time.sleep(0.1) # 100ms
        p.finishDispatch()
        f.result()
        cb.check()

        #
        # Remote case: the server closes the connection gracefully, which means the connection
        # will not be closed until all pending dispatched requests have completed.
        #
        f = p.sleepAsync(100)
        p.closeConnection()  # Close is delayed until sleep completes.
        try:
            f.result()
        except Exception:
            test(False)

        p.ice_getCachedConnection().close().result() # Wait until the connection is closed.

        print("ok")

        sys.stdout.write("testing abort connection... ")
        sys.stdout.flush()

        #
        # Local case: start an operation and then close the connection forcefully on the client side.
        # There will be no retry and we expect the invocation to fail with ConnectionAbortedException.
        #
        p.ice_ping()
        con = p.ice_getConnection()
        f = p.startDispatchAsync()
        f.sent()  # Ensure the request was sent before we close the connection.
        con.abort()
        try:
            f.result()
            test(False)
        except Ice.ConnectionAbortedException as ex:
            test(ex.closedByApplication)
        p.finishDispatch()

        #
        # Remote case: the server closes the connection forcefully. This causes the request to fail
        # with a ConnectionLostException. Since the close() operation is not idempotent, the client
        # will not retry.
        #
        try:
            p.abortConnection()
            test(False)
        except Ice.ConnectionLostException:
            # Expected.
            pass

        print("ok")


def allTestsFuture(helper, communicator, collocated):

    p = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint(num=0)}")
    testController = Test.TestIntfControllerPrx(communicator, f"testController:{helper.getTestEndpoint(num=1)}")

    sys.stdout.write("testing future invocations... ")
    sys.stdout.flush()
    ctx = {}

    test(p.ice_isAAsync("::Test::TestIntf").result())
    test(p.ice_isAAsync("::Test::TestIntf", ctx).result())

    p.ice_pingAsync().result()
    p.ice_pingAsync(ctx).result()

    test(p.ice_idAsync().result() == "::Test::TestIntf")
    test(p.ice_idAsync(ctx).result() == "::Test::TestIntf")

    test(len(p.ice_idsAsync().result()) == 2)
    test(len(p.ice_idsAsync(ctx).result()) == 2)

    if not collocated:
        test(type(p.ice_getConnectionAsync().result()) is IcePy.Connection)

    p.opAsync().result()
    p.opAsync(ctx).result()

    test(p.opWithResultAsync().result() == 15)
    test(p.opWithResultAsync(ctx).result() == 15)

    try:
        p.opWithUEAsync().result()
        test(False)
    except Test.TestIntfException:
        pass
    try:
        p.opWithUEAsync(ctx).result()
        test(False)
    except Test.TestIntfException:
        pass

    print("ok")

    sys.stdout.write("testing done callback... ")
    sys.stdout.flush()

    ctx = {}
    cb = FutureDoneCallback()

    p.ice_isAAsync(Test.TestIntf.ice_staticId()).add_done_callback(cb.isA)
    cb.check()
    p.ice_isAAsync(Test.TestIntf.ice_staticId(), ctx).add_done_callback(cb.isA)
    cb.check()

    p.ice_pingAsync().add_done_callback(cb.ping)
    cb.check()
    p.ice_pingAsync(ctx).add_done_callback(cb.ping)
    cb.check()

    p.ice_idAsync().add_done_callback(cb.id)
    cb.check()
    p.ice_idAsync(ctx).add_done_callback(cb.id)
    cb.check()

    p.ice_idsAsync().add_done_callback(cb.ids)
    cb.check()
    p.ice_idsAsync(ctx).add_done_callback(cb.ids)
    cb.check()

    if not collocated:
        p.ice_getConnectionAsync().add_done_callback(cb.connection)
        cb.check()

    p.opAsync().add_done_callback(cb.op)
    cb.check()
    p.opAsync(ctx).add_done_callback(cb.op)
    cb.check()

    p.opWithResultAsync().add_done_callback(cb.opWithResult)
    cb.check()
    p.opWithResultAsync(ctx).add_done_callback(cb.opWithResult)
    cb.check()

    p.opWithUEAsync().add_done_callback(cb.opWithUE)
    cb.check()
    p.opWithUEAsync(ctx).add_done_callback(cb.opWithUE)
    cb.check()

    print("ok")

    sys.stdout.write("testing done callback async... ")
    sys.stdout.flush()

    # Now repeat with add_done_callback_async
    ctx = {}
    cb = FutureDoneCallback()

    p.ice_isAAsync(Test.TestIntf.ice_staticId()).add_done_callback_async(cb.isA)
    cb.check()
    p.ice_isAAsync(Test.TestIntf.ice_staticId(), ctx).add_done_callback_async(cb.isA)
    cb.check()

    p.ice_pingAsync().add_done_callback_async(cb.ping)
    cb.check()
    p.ice_pingAsync(ctx).add_done_callback_async(cb.ping)
    cb.check()

    p.ice_idAsync().add_done_callback_async(cb.id)
    cb.check()
    p.ice_idAsync(ctx).add_done_callback_async(cb.id)
    cb.check()

    p.ice_idsAsync().add_done_callback_async(cb.ids)
    cb.check()
    p.ice_idsAsync(ctx).add_done_callback_async(cb.ids)
    cb.check()

    if not collocated:
        p.ice_getConnectionAsync().add_done_callback_async(cb.connection)
        cb.check()

    p.opAsync().add_done_callback_async(cb.op)
    cb.check()
    p.opAsync(ctx).add_done_callback_async(cb.op)
    cb.check()

    p.opWithResultAsync().add_done_callback_async(cb.opWithResult)
    cb.check()
    p.opWithResultAsync(ctx).add_done_callback_async(cb.opWithResult)
    cb.check()

    p.opWithUEAsync().add_done_callback_async(cb.opWithUE)
    cb.check()
    p.opWithUEAsync(ctx).add_done_callback_async(cb.opWithUE)
    cb.check()

    print("ok")

    if not collocated:
        sys.stdout.write("testing bi-dir... ")
        sys.stdout.flush()
        adapter = communicator.createObjectAdapter("")
        replyI = PingReplyI()
        reply = Test.PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI))

        context = { "ONE": "" }
        p.pingBiDir(reply, context)

        p.ice_getConnection().setAdapter(adapter)
        p.pingBiDir(reply)
        test(replyI.checkReceived())
        adapter.destroy()

        print("ok")

    sys.stdout.write("testing local exceptions... ")
    sys.stdout.flush()

    indirect = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"))

    try:
        indirect.opAsync().result()
        test(False)
    except Ice.NoEndpointException:
        pass

    try:
        p.ice_oneway().opWithResultAsync().result()
        test(False)
    except Ice.TwowayOnlyException:
        pass

    #
    # Check that CommunicatorDestroyedException is raised directly.
    #
    if p.ice_getConnection():
        initData = Ice.InitializationData()
        initData.properties = communicator.getProperties().clone()
        ic = Ice.initialize(initData)
        obj = ic.stringToProxy(p.ice_toString())
        p2 = Test.TestIntfPrx.checkedCast(obj)
        ic.destroy()

        try:
            p2.opAsync()
            test(False)
        except Ice.CommunicatorDestroyedException:
            pass

    print("ok")

    sys.stdout.write("testing local exceptions with done callback... ")
    sys.stdout.flush()

    i = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"))
    cb = FutureExceptionCallback()

    i.ice_isAAsync(Test.TestIntf.ice_staticId()).add_done_callback(cb.ex)
    cb.check()

    i.ice_pingAsync().add_done_callback(cb.ex)
    cb.check()

    i.ice_idAsync().add_done_callback(cb.ex)
    cb.check()

    i.ice_idsAsync().add_done_callback(cb.ex)
    cb.check()

    if not collocated:
        i.ice_getConnectionAsync().add_done_callback(cb.ex)
        cb.check()

    i.opAsync().add_done_callback(cb.ex)
    cb.check()

    print("ok")

    sys.stdout.write("testing exceptions with done callback... ")
    sys.stdout.flush()

    cb = FutureExceptionCallback()

    # Ensures no exception is set when response is received.
    p.ice_isAAsync(Test.TestIntf.ice_staticId()).add_done_callback(cb.noEx)
    p.opAsync().add_done_callback(cb.noEx)

    # If response is a user exception, it should be received.
    p.opWithUEAsync().add_done_callback(cb.opWithUE)
    cb.check()

    print("ok")

    sys.stdout.write("testing sent callback... ")
    sys.stdout.flush()

    cb = FutureSentCallback()

    p.ice_isAAsync("").add_sent_callback(cb.sent)
    cb.check()

    p.ice_pingAsync().add_sent_callback(cb.sent)
    cb.check()

    p.ice_idAsync().add_sent_callback(cb.sent)
    cb.check()

    p.ice_idsAsync().add_sent_callback(cb.sent)
    cb.check()

    p.opAsync().add_sent_callback(cb.sent)
    cb.check()

    p.ice_isAAsync("").add_sent_callback_async(cb.sentAsync)
    cb.check()

    p.ice_pingAsync().add_sent_callback_async(cb.sentAsync)
    cb.check()

    p.ice_idAsync().add_sent_callback_async(cb.sentAsync)
    cb.check()

    p.ice_idsAsync().add_sent_callback_async(cb.sentAsync)
    cb.check()

    p.opAsync().add_sent_callback_async(cb.sentAsync)
    cb.check()

    cbs = []
    b = [random.randint(0, 255) for x in range(0, 1024)]
    seq = bytes(b)
    testController.holdAdapter()
    try:
        cb = FutureSentCallback()
        while True:
            f = p.opWithPayloadAsync(seq)
            f.add_sent_callback(cb.sent)
            cbs.append(cb)
            if not f.is_sent_synchronously():
                break
            cb = FutureSentCallback()
    except Exception as ex:
        testController.resumeAdapter()
        raise ex
    testController.resumeAdapter()
    for r in cbs:
        r.check()

    print("ok")

    sys.stdout.write("testing batch requests with proxy... ")
    sys.stdout.flush()

    test(p.opBatchCount() == 0)
    b1 = p.ice_batchOneway()
    b1.opBatch()
    b1.opBatch()
    cb = FutureFlushCallback()
    f = b1.ice_flushBatchRequestsAsync()
    f.add_sent_callback(cb.sent)
    f.add_sent_callback_async(cb.sentAsync)
    cb.check()
    test(f.is_sent())
    test(f.done())
    test(p.waitForBatch(2))

    if p.ice_getConnection():  # No collocation optimization
        test(p.opBatchCount() == 0)
        b1 = p.ice_batchOneway()
        b1.opBatch()
        b1.ice_getConnection().close()
        cb = FutureFlushCallback()
        f = b1.ice_flushBatchRequestsAsync()
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(1))

    print("ok")

    if p.ice_getConnection():  # No collocation optimization
        sys.stdout.write("testing batch requests with connection... ")
        sys.stdout.flush()

        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(
            p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b1.opBatch()
        b1.opBatch()
        cb = FutureFlushCallback()
        f = b1.ice_getConnection().flushBatchRequestsAsync(
            Ice.CompressBatch.BasedOnProxy
        )
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(2))

        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(
            p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b1.opBatch()
        b1.ice_getConnection().close()
        cb = FutureFlushExCallback()
        f = b1.ice_getConnection().flushBatchRequestsAsync(
            Ice.CompressBatch.BasedOnProxy
        )
        f.add_done_callback(cb.exception)
        f.add_sent_callback(cb.sent)
        cb.check()
        test(not f.is_sent())
        test(f.done())
        test(p.opBatchCount() == 0)

        print("ok")

        sys.stdout.write("testing batch requests with communicator... ")
        sys.stdout.flush()

        #
        # 1 connection.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(
            p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b1.opBatch()
        b1.opBatch()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(2))

        #
        # 1 connection.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(
            p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b1.opBatch()
        b1.ice_getConnection().close()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())  # Exceptions are ignored!
        test(f.done())
        test(p.opBatchCount() == 0)

        #
        # 2 connections.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(
            p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b2 = Test.TestIntfPrx.uncheckedCast(
            p.ice_connectionId("2")
            .ice_getConnection()
            .createProxy(p.ice_getIdentity())
            .ice_batchOneway()
        )
        b2.ice_getConnection()  # Ensure connection is established.
        b1.opBatch()
        b1.opBatch()
        b2.opBatch()
        b2.opBatch()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(4))

        #
        # 2 connections - 1 failure.
        #
        # All connections should be flushed even if there are failures on some connections.
        # Exceptions should not be reported.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(
            p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b2 = Test.TestIntfPrx.uncheckedCast(
            p.ice_connectionId("2")
            .ice_getConnection()
            .createProxy(p.ice_getIdentity())
            .ice_batchOneway()
        )
        b2.ice_getConnection()  # Ensure connection is established.
        b1.opBatch()
        b2.opBatch()
        b1.ice_getConnection().close()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())  # Exceptions are ignored!
        test(f.done())
        test(p.waitForBatch(1))

        #
        # 2 connections - 2 failures.
        #
        # The sent callback should be invoked even if all connections fail.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(
            p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b2 = Test.TestIntfPrx.uncheckedCast(
            p.ice_connectionId("2")
            .ice_getConnection()
            .createProxy(p.ice_getIdentity())
            .ice_batchOneway()
        )
        b2.ice_getConnection()  # Ensure connection is established.
        b1.opBatch()
        b2.opBatch()
        b1.ice_getConnection().close()
        b2.ice_getConnection().close()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())  # Exceptions are ignored!
        test(f.done())
        test(p.opBatchCount() == 0)

        print("ok")

    sys.stdout.write("testing future operations... ")
    sys.stdout.flush()

    indirect = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"))
    f = indirect.opAsync()
    try:
        f.result()
        test(False)
    except Ice.NoEndpointException:
        pass

    testController.holdAdapter()
    f1 = None
    f2 = None
    try:
        f1 = p.opAsync()
        b = [random.randint(0, 255) for x in range(0, 1024)]
        seq = bytes(b)
        while True:
            f2 = p.opWithPayloadAsync(seq)
            if not f2.is_sent_synchronously():
                break

        test(f1 == f1)
        test(f1 != f2)

        if p.ice_getConnection():
            test(
                (f1.is_sent_synchronously() and f1.is_sent() and not f1.done())
                or (not f1.is_sent_synchronously() and not f1.done())
            )

            test(not f2.is_sent_synchronously() and not f2.done())
    except Exception as ex:
        testController.resumeAdapter()
        raise ex
    testController.resumeAdapter()

    f1.sent()
    test(f1.is_sent())

    f2.sent()
    test(f2.is_sent())

    f1.result()
    test(f1.done())

    f2.result()
    test(f2.done())

    test(f1.operation() == "op")
    test(f2.operation() == "opWithPayload")

    #
    # Twoway
    #
    f = p.ice_pingAsync()
    test(f.operation() == "ice_ping")
    f.result()

    #
    # Oneway
    #
    p2 = p.ice_oneway()
    f = p2.ice_pingAsync()
    test(f.operation() == "ice_ping")

    #
    # Batch request via proxy
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    f = p2.ice_flushBatchRequestsAsync()
    f.result()

    if p.ice_getConnection():
        #
        # Batch request via connection
        #
        con = p.ice_getConnection()
        p2 = p.ice_batchOneway()
        p2.ice_ping()
        f = con.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.result()

    #
    # Batch request via communicator
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
    f.result()

    if p.ice_getConnection():
        f1 = None
        f2 = None

        b = [random.randint(0, 255) for x in range(0, 10024)]
        seq = bytes(b)

        testController.holdAdapter()

        for x in range(0, 200):  # 2MB
            f = p.opWithPayloadAsync(seq)

        test(not f.is_sent())

        f1 = p.ice_pingAsync()
        f2 = p.ice_idAsync()
        f1.cancel()
        f2.cancel()
        try:
            f1.result()
            test(False)
        except Ice.InvocationCanceledException:
            pass

        try:
            f2.result()
            test(False)
        except Ice.InvocationCanceledException:
            pass

        testController.resumeAdapter()
        p.ice_ping()
        test(not f1.is_sent() and f1.done())
        test(not f2.is_sent() and f2.done())

        testController.holdAdapter()

        f1 = p.opAsync()
        f2 = p.ice_idAsync()
        f1.sent()
        f2.sent()
        f1.cancel()
        f2.cancel()
        try:
            f1.result()
            test(False)
        except Exception:
            pass
        try:
            f2.result()
            test(False)
        except Exception:
            pass
        testController.resumeAdapter()

    print("ok")

    p.shutdown()
