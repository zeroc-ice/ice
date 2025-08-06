# Copyright (c) ZeroC, Inc.

import random
import sys
import threading
import time
from typing import Any, Type, cast, override

from generated.test.Ice.ami import Test
from TestHelper import TestHelper, test

import Ice
import IcePy


class PingReplyI(Test.PingReply):
    def __init__(self):
        self._received = False

    def reply(self, current: Ice.Current):
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

    def exception(self, ex: BaseException):
        test(False)


class FutureDoneCallback(CallbackBase):
    def isA(self, f: Ice.Future[bool]):
        test(f.result())
        self.called()

    def ping(self, f: Ice.Future):
        self.called()

    def id(self, f: Ice.Future[str]):
        test(f.result() == "::Test::TestIntf")
        self.called()

    def ids(self, f: Ice.Future[list[str]]):
        test(len(f.result()) == 2)
        self.called()

    def connection(self, f: Ice.Future[Ice.Connection]):
        assert isinstance(f.result(), Ice.Connection)
        self.called()

    def op(self, f: Ice.Future[None]):
        self.called()

    def opWithResult(self, f: Ice.Future[int]):
        test(f.result() == 15)
        self.called()

    def opWithUE(self, f: Ice.Future[None]):
        try:
            f.result()
            test(False)
        except Test.TestIntfException:
            self.called()
        except Exception:
            test(False)


class FutureExceptionCallback(CallbackBase):
    def opWithUE(self, f: Ice.Future):
        test(isinstance(f.exception(), Test.TestIntfException))
        self.called()

    def ex(self, f: Ice.Future):
        test(isinstance(f.exception(), Ice.NoEndpointException))
        self.called()

    def noEx(self, f: Ice.Future):
        test(f.exception() is None)


class FutureSentCallback(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)
        self._thread = threading.current_thread()

    def sent(self, f: Ice.InvocationFuture, sentSynchronously: bool):
        self.called()

    def sentAsync(self, f: Ice.InvocationFuture, sentSynchronously: bool):
        test(self._thread != threading.current_thread())
        self.called()


class FutureFlushCallback(CallbackBase):
    def __init__(self, cookie: Any = None):
        CallbackBase.__init__(self)
        self._thread = threading.current_thread()
        self._cookie = cookie

    def sent(self, f: Ice.InvocationFuture, sentSynchronously: bool):
        self.called()

    def sentAsync(self, f: Ice.InvocationFuture, sentSynchronously: bool):
        test(self._thread != threading.current_thread())
        self.called()


class FutureFlushExCallback(CallbackBase):
    def __init__(self, cookie: Any = None):
        CallbackBase.__init__(self)
        self._cookie = cookie

    @override
    def exception(self, ex: BaseException):
        self.called()

    def sent(self, f: Ice.InvocationFuture, sentSynchronously: bool):
        test(False)


LocalException = 0
UserException = 1
OtherException = 2


def allTests(helper: TestHelper, communicator: Ice.Communicator, collocated: bool):
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
        assert isinstance(f, Ice.InvocationFuture)
        f.sent()  # Ensure the request was sent before we close the connection.
        closeConnectionFuture = con.close()

        # give time for startDispatch to start in the server before we call finishDispatch
        time.sleep(0.1)  # 100ms
        p.finishDispatch()
        f.result()
        cb.check()

        #
        # Remote case: the server closes the connection gracefully, which means the connection
        # will not be closed until all pending dispatched requests have completed.
        #
        f = p.sleepAsync(100)
        assert isinstance(f, Ice.InvocationFuture)
        p.closeConnection()  # Close is delayed until sleep completes.
        try:
            f.result()
        except Exception:
            test(False)

        # Wait until the connection is gracefully closed.
        con = p.ice_getCachedConnection()
        assert con is not None
        con.close().result()
        test(closeConnectionFuture.done())

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
        assert isinstance(f, Ice.InvocationFuture)
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


def allTestsFuture(helper: TestHelper, communicator: Ice.Communicator, collocated: bool):
    p = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint(num=0)}")
    testController = Test.TestIntfControllerPrx(communicator, f"testController:{helper.getTestEndpoint(num=1)}")

    sys.stdout.write("testing future invocations... ")
    sys.stdout.flush()
    ctx = {}

    test(cast(Ice.Future, p.ice_isAAsync("::Test::TestIntf")).result())
    test(cast(Ice.Future, p.ice_isAAsync("::Test::TestIntf", ctx)).result())

    test(cast(Ice.Future, p.ice_pingAsync()).result() == None)
    test(cast(Ice.Future, p.ice_pingAsync(ctx)).result() == None)

    test(cast(Ice.Future, p.ice_idAsync()).result() == "::Test::TestIntf")
    test(cast(Ice.Future, p.ice_idAsync(ctx)).result() == "::Test::TestIntf")

    test(len(cast(Ice.Future, p.ice_idsAsync()).result()) == 2)
    test(len(cast(Ice.Future, p.ice_idsAsync(ctx)).result()) == 2)

    if not collocated:
        con = cast(Ice.Future, p.ice_getConnectionAsync()).result()
        assert type(con) is IcePy.Connection

    cast(Ice.Future, p.opAsync()).result()
    cast(Ice.Future, p.opAsync(ctx)).result()

    test(cast(Ice.Future, p.opWithResultAsync()).result() == 15)
    test(cast(Ice.Future, p.opWithResultAsync(ctx)).result() == 15)

    try:
        cast(Ice.Future, p.opWithUEAsync()).result()
        test(False)
    except Test.TestIntfException:
        pass
    try:
        cast(Ice.Future, p.opWithUEAsync(ctx)).result()
        test(False)
    except Test.TestIntfException:
        pass

    print("ok")

    sys.stdout.write("testing done callback... ")
    sys.stdout.flush()

    ctx = {}
    cb = FutureDoneCallback()

    cast(Ice.InvocationFuture, p.ice_isAAsync(Test.TestIntf.ice_staticId())).add_done_callback(cb.isA)
    cb.check()
    cast(Ice.InvocationFuture, p.ice_isAAsync(Test.TestIntf.ice_staticId(), ctx)).add_done_callback(cb.isA)
    cb.check()

    cast(Ice.InvocationFuture, p.ice_pingAsync()).add_done_callback(cb.ping)
    cb.check()
    cast(Ice.InvocationFuture, p.ice_pingAsync(ctx)).add_done_callback(cb.ping)
    cb.check()

    cast(Ice.InvocationFuture, p.ice_idAsync()).add_done_callback(cb.id)
    cb.check()
    cast(Ice.InvocationFuture, p.ice_idAsync(ctx)).add_done_callback(cb.id)
    cb.check()

    cast(Ice.InvocationFuture, p.ice_idsAsync()).add_done_callback(cb.ids)
    cb.check()
    cast(Ice.InvocationFuture, p.ice_idsAsync(ctx)).add_done_callback(cb.ids)
    cb.check()

    if not collocated:
        cast(Ice.InvocationFuture, p.ice_getConnectionAsync()).add_done_callback(cb.connection)
        cb.check()

    cast(Ice.InvocationFuture, p.opAsync()).add_done_callback(cb.op)
    cb.check()
    cast(Ice.InvocationFuture, p.opAsync(ctx)).add_done_callback(cb.op)
    cb.check()

    cast(Ice.InvocationFuture, p.opWithResultAsync()).add_done_callback(cb.opWithResult)
    cb.check()
    cast(Ice.InvocationFuture, p.opWithResultAsync(ctx)).add_done_callback(cb.opWithResult)
    cb.check()

    cast(Ice.InvocationFuture, p.opWithUEAsync()).add_done_callback(cb.opWithUE)
    cb.check()
    cast(Ice.InvocationFuture, p.opWithUEAsync(ctx)).add_done_callback(cb.opWithUE)
    cb.check()

    print("ok")

    if not collocated:
        sys.stdout.write("testing bi-dir... ")
        sys.stdout.flush()
        adapter = communicator.createObjectAdapter("")
        replyI = PingReplyI()
        reply = Test.PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI))

        context = {"ONE": ""}
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
        cast(Ice.InvocationFuture, indirect.opAsync()).result()
        test(False)
    except Ice.NoEndpointException:
        pass

    try:
        cast(Ice.InvocationFuture, p.ice_oneway().opWithResultAsync()).result()
        test(False)
    except Ice.TwowayOnlyException:
        pass

    #
    # Check that CommunicatorDestroyedException is raised directly.
    #
    if p.ice_getConnection():
        initData = Ice.InitializationData()
        initData.properties = communicator.getProperties().clone()
        ic = Ice.initialize([], initData=initData)
        obj = ic.stringToProxy(p.ice_toString())
        p2 = Test.TestIntfPrx.checkedCast(obj)
        assert p2 is not None
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

    cast(Ice.InvocationFuture, i.ice_isAAsync(Test.TestIntf.ice_staticId())).add_done_callback(cb.ex)
    cb.check()

    cast(Ice.InvocationFuture, i.ice_pingAsync()).add_done_callback(cb.ex)
    cb.check()

    cast(Ice.InvocationFuture, i.ice_idAsync()).add_done_callback(cb.ex)
    cb.check()

    cast(Ice.InvocationFuture, i.ice_idsAsync()).add_done_callback(cb.ex)
    cb.check()

    if not collocated:
        cast(Ice.InvocationFuture, i.ice_getConnectionAsync()).add_done_callback(cb.ex)
        cb.check()

    cast(Ice.InvocationFuture, i.opAsync()).add_done_callback(cb.ex)
    cb.check()

    print("ok")

    sys.stdout.write("testing exceptions with done callback... ")
    sys.stdout.flush()

    cb = FutureExceptionCallback()

    # Ensures no exception is set when response is received.
    cast(Ice.InvocationFuture, p.ice_isAAsync(Test.TestIntf.ice_staticId())).add_done_callback(cb.noEx)
    cast(Ice.InvocationFuture, p.opAsync()).add_done_callback(cb.noEx)

    # If response is a user exception, it should be received.
    cast(Ice.InvocationFuture, p.opWithUEAsync()).add_done_callback(cb.opWithUE)
    cb.check()

    print("ok")

    sys.stdout.write("testing sent callback... ")
    sys.stdout.flush()

    cb = FutureSentCallback()

    cast(Ice.InvocationFuture, p.ice_isAAsync("")).add_sent_callback(cb.sent)
    cb.check()

    cast(Ice.InvocationFuture, p.ice_pingAsync()).add_sent_callback(cb.sent)
    cb.check()

    cast(Ice.InvocationFuture, p.ice_idAsync()).add_sent_callback(cb.sent)
    cb.check()

    cast(Ice.InvocationFuture, p.ice_idsAsync()).add_sent_callback(cb.sent)
    cb.check()

    cast(Ice.InvocationFuture, p.opAsync()).add_sent_callback(cb.sent)
    cb.check()

    cbs = []
    b = [random.randint(0, 255) for _ in range(0, 1024)]
    seq = bytes(b)
    testController.holdAdapter()
    try:
        cb = FutureSentCallback()
        while True:
            f = p.opWithPayloadAsync(seq)
            assert isinstance(f, Ice.InvocationFuture)
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
    assert isinstance(f, Ice.InvocationFuture)
    f.add_sent_callback(cb.sent)
    cb.check()
    test(f.is_sent())
    test(f.done())
    test(p.waitForBatch(2))

    if p.ice_getConnection():  # No collocation optimization
        test(p.opBatchCount() == 0)
        b1 = p.ice_batchOneway()
        b1.opBatch()
        # Wait until the connection is gracefully closed.
        b1.ice_getConnection().close().result()
        cb = FutureFlushCallback()
        f = b1.ice_flushBatchRequestsAsync()
        assert isinstance(f, Ice.InvocationFuture)
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
            cast(Ice.Connection, p.ice_getConnection()).createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b1.opBatch()
        b1.opBatch()
        cb = FutureFlushCallback()
        f = cast(Ice.Connection, b1.ice_getConnection()).flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result()  # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(2))

        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        # Wait until the connection is gracefully closed.
        b1.ice_getConnection().close().result()
        cb = FutureFlushExCallback()
        f = b1.ice_getConnection().flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
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
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.opBatch()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
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
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        # Wait until the connection is gracefully closed.
        b1.ice_getConnection().close().result()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
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
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b2 = Test.TestIntfPrx.uncheckedCast(
            p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b2.ice_getConnection()  # Ensure connection is established.
        b1.opBatch()
        b1.opBatch()
        b2.opBatch()
        b2.opBatch()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
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
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b2 = Test.TestIntfPrx.uncheckedCast(
            p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b2.ice_getConnection()  # Ensure connection is established.
        b1.opBatch()
        b2.opBatch()
        # Wait until the connection is gracefully closed.
        b1.ice_getConnection().close().result()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
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
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b2 = Test.TestIntfPrx.uncheckedCast(
            p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway()
        )
        b2.ice_getConnection()  # Ensure connection is established.
        b1.opBatch()
        b2.opBatch()
        # Wait until the connections are gracefully closed.
        b1.ice_getConnection().close().result()
        b2.ice_getConnection().close().result()
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
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
    assert isinstance(f, Ice.InvocationFuture)
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
        assert isinstance(f1, Ice.InvocationFuture)
        b = [random.randint(0, 255) for _ in range(0, 1024)]
        seq = bytes(b)
        while True:
            f2 = p.opWithPayloadAsync(seq)
            assert isinstance(f2, Ice.InvocationFuture)
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

    #
    # Twoway
    #
    f = p.ice_pingAsync()
    assert isinstance(f, Ice.InvocationFuture)
    f.result()

    #
    # Oneway
    #
    p2 = p.ice_oneway()
    f = p2.ice_pingAsync()

    #
    # Batch request via proxy
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    f = p2.ice_flushBatchRequestsAsync()
    assert isinstance(f, Ice.InvocationFuture)
    f.result()

    if p.ice_getConnection():
        #
        # Batch request via connection
        #
        con = p.ice_getConnection()
        p2 = p.ice_batchOneway()
        p2.ice_ping()
        f = con.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        assert isinstance(f, Ice.InvocationFuture)
        f.result()

    #
    # Batch request via communicator
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
    assert isinstance(f, Ice.InvocationFuture)
    f.result()

    if p.ice_getConnection():
        f1 = None
        f2 = None

        b = [random.randint(0, 255) for _ in range(0, 10024)]
        seq = bytes(b)

        testController.holdAdapter()

        for _ in range(0, 200):  # 2MB
            f = p.opWithPayloadAsync(seq)

        assert isinstance(f, Ice.InvocationFuture)
        test(not f.is_sent())

        f1 = p.ice_pingAsync()
        f2 = p.ice_idAsync()

        assert isinstance(f1, Ice.InvocationFuture)
        assert isinstance(f2, Ice.InvocationFuture)

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

        assert isinstance(f1, Ice.InvocationFuture)
        assert isinstance(f2, Ice.InvocationFuture)

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
