# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading, random

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def check(self):
        self._cond.acquire()
        try:
            while not self._called:
                self._cond.wait()
            self._called = False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

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
        test(conn != None)
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
        except:
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
        test(conn != None)
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
        except:
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
        self._thread = threading.currentThread()

    def response(self, *args):
        pass

    def ex(self, ex):
        pass

    def sent(self, sentSynchronously):
        test((sentSynchronously and self._thread == threading.currentThread()) or \
             (not sentSynchronously and self._thread != threading.currentThread()))
        self.called()

class SentCallbackWC(CallbackBase):
    def __init__(self, cookie):
        CallbackBase.__init__(self)
        self._thread = threading.currentThread()
        self._cookie = cookie

    def response(self, *args):
        pass

    def ex(self, ex, cookie):
        pass

    def sent(self, sentSynchronously, cookie):
        test(cookie == self._cookie)
        test((sentSynchronously and self._thread == threading.currentThread()) or \
             (not sentSynchronously and self._thread != threading.currentThread()))
        self.called()

class FlushCallback(CallbackBase):
    def __init__(self, cookie=None):
        CallbackBase.__init__(self)
        self._thread = threading.currentThread()
        self._cookie = cookie

    def exception(self, ex):
        test(False)

    def exceptionWC(self, ex, cookie):
        test(False)

    def sent(self, sentSynchronously):
        test((sentSynchronously and self._thread == threading.currentThread()) or \
             (not sentSynchronously and self._thread != threading.currentThread()))
        self.called()

    def sentWC(self, sentSynchronously, cookie):
        test((sentSynchronously and self._thread == threading.currentThread()) or \
             (not sentSynchronously and self._thread != threading.currentThread()))
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

def allTests(communicator, collocated):
    sref = "test:default -p 12010"
    obj = communicator.stringToProxy(sref)
    test(obj)

    p = Test.TestIntfPrx.uncheckedCast(obj)

    sref = "testController:default -p 12011"
    obj = communicator.stringToProxy(sref)
    test(obj)

    testController = Test.TestIntfControllerPrx.uncheckedCast(obj)

    sys.stdout.write("testing begin/end invocation... ")
    sys.stdout.flush()
    ctx = {}

    result = p.begin_ice_isA("::Test::TestIntf")
    test(p.end_ice_isA(result))
    result = p.begin_ice_isA("::Test::TestIntf", _ctx=ctx)
    test(p.end_ice_isA(result))

    result = p.begin_ice_ping()
    p.end_ice_ping(result)
    result = p.begin_ice_ping(_ctx=ctx)
    p.end_ice_ping(result)

    result = p.begin_ice_id()
    test(p.end_ice_id(result) == "::Test::TestIntf")
    result = p.begin_ice_id(_ctx=ctx)
    test(p.end_ice_id(result) == "::Test::TestIntf")

    result = p.begin_ice_ids()
    test(len(p.end_ice_ids(result)) == 2)
    result = p.begin_ice_ids(_ctx=ctx)
    test(len(p.end_ice_ids(result)) == 2)

    if not collocated:
        result = p.begin_ice_getConnection()
        test(p.end_ice_getConnection(result) != None)

    result = p.begin_op()
    p.end_op(result)
    result = p.begin_op(_ctx=ctx)
    p.end_op(result)

    result = p.begin_opWithResult()
    test(p.end_opWithResult(result) == 15)
    result = p.begin_opWithResult(_ctx=ctx)
    test(p.end_opWithResult(result) == 15)

    result = p.begin_opWithUE()
    try:
        p.end_opWithUE(result)
        test(False)
    except Test.TestIntfException:
        pass
    result = p.begin_opWithUE(_ctx=ctx)
    try:
        p.end_opWithUE(result)
        test(False)
    except Test.TestIntfException:
        pass

    print("ok")

    sys.stdout.write("testing response callback... ")
    sys.stdout.flush()

    ctx = {}
    cb = ResponseCallback()
    cookie = 5
    cbWC = ResponseCallbackWC(cookie)

    p.begin_ice_isA(Test.TestIntf.ice_staticId(), cb.isA, cb.ex)
    cb.check()
    p.begin_ice_isA(Test.TestIntf.ice_staticId(), lambda r: cbWC.isA(r, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_ice_isA(Test.TestIntf.ice_staticId(), cb.isA, cb.ex, _ctx=ctx)
    cb.check()
    p.begin_ice_isA(Test.TestIntf.ice_staticId(), lambda r: cbWC.isA(r, cookie), lambda ex: cbWC.ex(ex, cookie),
                    _ctx=ctx)
    cbWC.check()

    p.begin_ice_ping(cb.ping, cb.ex)
    cb.check()
    p.begin_ice_ping(lambda: cbWC.ping(cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_ice_ping(cb.ping, cb.ex, _ctx=ctx)
    cb.check()
    p.begin_ice_ping(lambda: cbWC.ping(cookie), lambda: cbWC.ex(ex, cookie), _ctx=ctx)
    cbWC.check()

    p.begin_ice_id(cb.id, cb.ex)
    cb.check()
    p.begin_ice_id(lambda id: cbWC.id(id, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_ice_id(cb.id, cb.ex, _ctx=ctx)
    cb.check()
    p.begin_ice_id(lambda id: cbWC.id(id, cookie), lambda ex: cbWC.ex(ex, cookie), _ctx=ctx)
    cbWC.check()

    p.begin_ice_ids(cb.ids, cb.ex)
    cb.check()
    p.begin_ice_ids(lambda ids: cbWC.ids(ids, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_ice_ids(cb.ids, cb.ex, _ctx=ctx)
    cb.check()
    p.begin_ice_ids(lambda ids: cbWC.ids(ids, cookie), lambda ex: cbWC.ex(ex, cookie), _ctx=ctx)
    cbWC.check()

    if not collocated:
        p.begin_ice_getConnection(cb.connection, cb.ex)
        cb.check()
        p.begin_ice_getConnection(lambda conn: cbWC.connection(conn, cookie), lambda ex: cbWC.ex(ex, cookie))
        cbWC.check()

    p.begin_op(cb.op, cb.ex)
    cb.check()
    p.begin_op(lambda: cbWC.op(cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_op(cb.op, cb.ex, _ctx=ctx)
    cb.check()
    p.begin_op(lambda: cbWC.op(cookie), lambda ex: cbWC.ex(ex, cookie), _ctx=ctx)
    cbWC.check()

    p.begin_opWithResult(cb.opWithResult, cb.ex)
    cb.check()
    p.begin_opWithResult(lambda r: cbWC.opWithResult(r, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_opWithResult(cb.opWithResult, cb.ex, _ctx=ctx)
    cb.check()
    p.begin_opWithResult(lambda r: cbWC.opWithResult(r, cookie), lambda ex: cbWC.ex(ex, cookie), _ctx=ctx)
    cbWC.check()

    p.begin_opWithUE(cb.op, cb.opWithUE)
    cb.check()
    p.begin_opWithUE(lambda: cbWC.op(cookie), lambda ex: cbWC.opWithUE(ex, cookie))
    cbWC.check()
    p.begin_opWithUE(cb.op, cb.opWithUE, _ctx=ctx)
    cb.check()
    p.begin_opWithUE(lambda: cbWC.op(cookie), lambda ex: cbWC.opWithUE(ex, cookie), _ctx=ctx)
    cbWC.check()

    print("ok")

    sys.stdout.write("testing local exceptions... ")
    sys.stdout.flush()

    indirect = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"))

    r = indirect.begin_op()
    try:
        indirect.end_op(r)
        test(False)
    except Ice.NoEndpointException:
        pass

    try:
        p.ice_oneway().begin_opWithResult()
        test(False)
    except RuntimeError:
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
        ic.destroy();

        try:
            p2.begin_op()
            test(False)
        except Ice.CommunicatorDestroyedException:
            pass

    print("ok")

    sys.stdout.write("testing local exceptions with response callback... ")
    sys.stdout.flush()

    i = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"))
    cb = ExceptionCallback()
    cookie = 5
    cbWC = ExceptionCallbackWC(cookie)

    i.begin_ice_isA(Test.TestIntf.ice_staticId(), cb.response, cb.ex)
    cb.check()
    i.begin_ice_isA(Test.TestIntf.ice_staticId(), lambda b: cbWC.response(b, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()

    i.begin_ice_ping(cb.response, cb.ex)
    cb.check()
    i.begin_ice_ping(lambda: cbWC.response(cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()

    i.begin_ice_id(cb.response, cb.ex)
    cb.check()
    i.begin_ice_id(lambda id: cbWC.response(id, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()

    i.begin_ice_ids(cb.response, cb.ex)
    cb.check()
    i.begin_ice_ids(lambda ids: cbWC.response(ids, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()

    if not collocated:
        i.begin_ice_getConnection(cb.response, cb.ex)
        cb.check()
        i.begin_ice_getConnection(lambda conn: cbWC.response(conn, cookie), lambda ex: cbWC.ex(ex, cookie))
        cbWC.check()

    i.begin_op(cb.response, cb.ex)
    cb.check()
    i.begin_op(lambda: cbWC.response(cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()

    print("ok")

    sys.stdout.write("testing exception callback... ")
    sys.stdout.flush()

    cb = ExceptionCallback()
    cookie = 5
    cbWC = ExceptionCallbackWC(cookie)

    # Ensures no exception is called when response is received.
    p.begin_ice_isA(Test.TestIntf.ice_staticId(), cb.nullResponse, cb.noEx)
    p.begin_ice_isA(Test.TestIntf.ice_staticId(), lambda b: cbWC.nullResponse(b, cookie),
                    lambda ex: cbWC.noEx(ex, cookie))
    p.begin_op(cb.nullResponse, cb.noEx)
    p.begin_op(lambda: cbWC.nullResponse(cookie), lambda ex: cbWC.noEx(ex, cookie))

    # If response is a user exception, it should be received.
    p.begin_opWithUE(cb.nullResponse, cb.opWithUE)
    cb.check()
    p.begin_opWithUE(lambda: cbWC.nullResponse(cookie), lambda ex: cbWC.opWithUE(ex, cookie))
    cbWC.check()

    print("ok")

    sys.stdout.write("testing sent callback... ")
    sys.stdout.flush()

    cb = SentCallback()
    cookie = 5
    cbWC = SentCallbackWC(cookie)

    p.begin_ice_isA("", cb.response, cb.ex, cb.sent)
    cb.check()
    p.begin_ice_isA("", lambda b: cbWC.response(b, cookie), lambda ex: cbWC.ex(ex, cookie),
                    lambda ss: cbWC.sent(ss, cookie))
    cbWC.check()

    p.begin_ice_ping(cb.response, cb.ex, cb.sent)
    cb.check()
    p.begin_ice_ping(lambda: cbWC.response(cookie), lambda ex: cbWC.ex(ex, cookie), lambda ss: cbWC.sent(ss, cookie))
    cbWC.check()

    p.begin_ice_id(cb.response, cb.ex, cb.sent)
    cb.check()
    p.begin_ice_id(lambda id: cbWC.response(id, cookie), lambda ex: cbWC.ex(ex, cookie),
                   lambda ss: cbWC.sent(ss, cookie))
    cbWC.check()

    p.begin_ice_ids(cb.response, cb.ex, cb.sent)
    cb.check()
    p.begin_ice_ids(lambda ids: cbWC.response(ids, cookie), lambda ex: cbWC.ex(ex, cookie),
                    lambda ss: cbWC.sent(ss, cookie))
    cbWC.check()

    p.begin_op(cb.response, cb.ex, cb.sent)
    cb.check()
    p.begin_op(lambda: cbWC.response(cookie), lambda ex: cbWC.ex(ex, cookie), lambda ss: cbWC.sent(ss, cookie))
    cbWC.check()

    cbs = []
    if sys.version_info[0] == 2:
        b = [chr(random.randint(0, 255)) for x in range(0, 1024)]
        seq = ''.join(b)
    else:
        b = [random.randint(0, 255) for x in range(0, 1024)]
        seq = bytes(b)
    testController.holdAdapter()
    try:
        cb = SentCallback()
        while(p.begin_opWithPayload(seq, None, cb.ex, cb.sent).sentSynchronously()):
            cbs.append(cb)
            cb = SentCallback()
    except Exception as ex:
        testController.resumeAdapter()
        raise ex
    testController.resumeAdapter()
    for r in cbs:
        r.check()

    print("ok")

    sys.stdout.write("testing illegal arguments... ")
    sys.stdout.flush()

    result = p.begin_op()
    p.end_op(result)
    try:
        p.end_op(result)
        test(False)
    except RuntimeError:
        pass

    result = p.begin_op()
    try:
        p.end_opWithResult(result)
        test(False)
    except RuntimeError:
        pass

    print("ok")

    sys.stdout.write("testing unexpected exceptions from callback... ")
    sys.stdout.flush()

    q = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"))
    throwTypes = [ LocalException, UserException, OtherException ]

    for t in throwTypes:
        cb = Thrower(t)
        cookie = 5

        p.begin_op(cb.op, cb.noEx)
        cb.check()

        p.begin_op(lambda: cb.opWC(cookie), lambda ex: cb.noExWC(ex, cookie))
        cb.check()

        q.begin_op(cb.op, cb.ex)
        cb.check()

        q.begin_op(lambda: cb.opWC(cookie), lambda ex: cb.exWC(ex, cookie))
        cb.check()

        p.begin_op(cb.noOp, cb.ex, cb.sent)
        cb.check()

        p.begin_op(lambda: cb.noOpWC(cookie), lambda ex: cb.exWC(ex, cookie), lambda ss: cb.sentWC(ss, cookie))
        cb.check()

        q.begin_op(None, cb.ex)
        cb.check()

        q.begin_op(None, lambda ex: cb.exWC(ex, cookie))
        cb.check()

    print("ok")

    sys.stdout.write("testing batch requests with proxy... ")
    sys.stdout.flush()

    cookie = 5

    #
    # Without cookie.
    #
    test(p.opBatchCount() == 0)
    b1 = p.ice_batchOneway()
    b1.opBatch()
    b1.opBatch()
    cb = FlushCallback()
    r = b1.begin_ice_flushBatchRequests(cb.exception, cb.sent)
    cb.check()
    test(r.isSent())
    test(r.isCompleted())
    test(p.waitForBatch(2))

    #
    # With cookie.
    #
    test(p.opBatchCount() == 0)
    b1 = p.ice_batchOneway()
    b1.opBatch()
    b1.opBatch()
    cb = FlushCallback(cookie)
    r = b1.begin_ice_flushBatchRequests(lambda ex: cb.exceptionWC(ex, cookie), lambda ss: cb.sentWC(ss, cookie))
    cb.check()
    test(p.waitForBatch(2))

    if p.ice_getConnection(): # No collocation optimization
        #
        # Exception without cookie.
        #
        test(p.opBatchCount() == 0)
        b1 = p.ice_batchOneway()
        b1.opBatch()
        b1.ice_getConnection().close(False)
        cb = FlushCallback()
        r = b1.begin_ice_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(r.isSent())
        test(r.isCompleted())
        test(p.waitForBatch(1))

        #
        # Exception with cookie.
        #
        test(p.opBatchCount() == 0)
        b1 = p.ice_batchOneway()
        b1.opBatch()
        b1.ice_getConnection().close(False)
        cb = FlushCallback(cookie)
        r = b1.begin_ice_flushBatchRequests(lambda ex: cb.exceptionWC(ex, cookie), lambda ss: cb.sentWC(ss, cookie))
        cb.check()
        test(p.waitForBatch(1))

    print("ok")

    if p.ice_getConnection(): # No collocation optimization
        sys.stdout.write("testing batch requests with connection... ")
        sys.stdout.flush()

        cookie = 5

        #
        # Without cookie.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.opBatch()
        cb = FlushCallback()
        r = b1.ice_getConnection().begin_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(r.isSent())
        test(r.isCompleted())
        test(p.waitForBatch(2))

        #
        # With cookie.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.opBatch()
        cb = FlushCallback(cookie)
        r = b1.ice_getConnection().begin_flushBatchRequests(lambda ex: cb.exceptionWC(ex, cookie),
                                                            lambda ss: cb.sentWC(ss, cookie))
        cb.check()
        test(p.waitForBatch(2))

        #
        # Exception without cookie.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.ice_getConnection().close(False)
        cb = FlushExCallback()
        r = b1.ice_getConnection().begin_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(not r.isSent())
        test(r.isCompleted())
        test(p.opBatchCount() == 0)

        #
        # Exception with cookie.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.ice_getConnection().close(False)
        cb = FlushExCallback(cookie)
        r = b1.ice_getConnection().begin_flushBatchRequests(lambda ex: cb.exceptionWC(ex, cookie),
                                                            lambda ss: cb.sentWC(ss, cookie))
        cb.check()
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
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(r.isSent())
        test(r.isCompleted())
        test(p.waitForBatch(2))

        #
        # 1 connection.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.ice_getConnection().close(False)
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(r.isSent()) # Exceptions are ignored!
        test(r.isCompleted())
        test(p.opBatchCount() == 0)

        #
        # 2 connections.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b2 = Test.TestIntfPrx.uncheckedCast(p.ice_connectionId("2").ice_getConnection().createProxy(
            p.ice_getIdentity()).ice_batchOneway())
        b2.ice_getConnection() # Ensure connection is established.
        b1.opBatch()
        b1.opBatch()
        b2.opBatch()
        b2.opBatch()
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(r.isSent())
        test(r.isCompleted())
        test(p.waitForBatch(4))

        #
        # 2 connections - 1 failure.
        #
        # All connections should be flushed even if there are failures on some connections.
        # Exceptions should not be reported.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b2 = Test.TestIntfPrx.uncheckedCast(p.ice_connectionId("2").ice_getConnection().createProxy(
            p.ice_getIdentity()).ice_batchOneway())
        b2.ice_getConnection() # Ensure connection is established.
        b1.opBatch()
        b2.opBatch()
        b1.ice_getConnection().close(False)
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(r.isSent()) # Exceptions are ignored!
        test(r.isCompleted())
        test(p.waitForBatch(1))

        #
        # 2 connections - 2 failures.
        #
        # The sent callback should be invoked even if all connections fail.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b2 = Test.TestIntfPrx.uncheckedCast(p.ice_connectionId("2").ice_getConnection().createProxy(
            p.ice_getIdentity()).ice_batchOneway())
        b2.ice_getConnection() # Ensure connection is established.
        b1.opBatch()
        b2.opBatch()
        b1.ice_getConnection().close(False)
        b2.ice_getConnection().close(False)
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(cb.exception, cb.sent)
        cb.check()
        test(r.isSent()) # Exceptions are ignored!
        test(r.isCompleted())
        test(p.opBatchCount() == 0)

        print("ok")

    sys.stdout.write("testing AsyncResult operations... ")
    sys.stdout.flush()

    indirect = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"))
    r = indirect.begin_op()
    try:
        r.waitForCompleted()
        r.throwLocalException()
        test(False)
    except Ice.NoEndpointException:
        pass

    testController.holdAdapter()
    r1 = None
    r2 = None
    try:
        r1 = p.begin_op()
        if sys.version_info[0] == 2:
            b = [chr(random.randint(0, 255)) for x in range(0, 1024)]
            seq = ''.join(b)
        else:
            b = [random.randint(0, 255) for x in range(0, 1024)]
            seq = bytes(b)
        while(True):
            r2 = p.begin_opWithPayload(seq)
            if not r2.sentSynchronously():
                break

        test(r1 == r1)
        test(r1 != r2)

        if p.ice_getConnection():
            test((r1.sentSynchronously() and r1.isSent() and not r1.isCompleted()) or
                 (not r1.sentSynchronously() and not r1.isCompleted()));

            test(not r2.sentSynchronously() and not r2.isCompleted());
    except Exception as ex:
        testController.resumeAdapter()
        raise ex
    testController.resumeAdapter()

    r1.waitForSent()
    test(r1.isSent())

    r2.waitForSent()
    test(r2.isSent())

    r1.waitForCompleted()
    test(r1.isCompleted())

    r2.waitForCompleted()
    test(r2.isCompleted())

    test(r1.getOperation() == "op")
    test(r2.getOperation() == "opWithPayload")

    #
    # Twoway
    #
    r = p.begin_ice_ping()
    test(r.getOperation() == "ice_ping")
    test(r.getConnection() == None) # Expected
    test(r.getCommunicator() == communicator)
    test(r.getProxy() == p)
    p.end_ice_ping(r)

    #
    # Oneway
    #
    p2 = p.ice_oneway()
    r = p2.begin_ice_ping()
    test(r.getOperation() == "ice_ping")
    test(r.getConnection() == None) # Expected
    test(r.getCommunicator() == communicator)
    test(r.getProxy() == p2)

    #
    # Batch request via proxy
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    r = p2.begin_ice_flushBatchRequests()
    test(r.getConnection() == None) # Expected
    test(r.getCommunicator() == communicator)
    test(r.getProxy() == p2)
    p2.end_ice_flushBatchRequests(r)

    if p.ice_getConnection():
        #
        # Batch request via connection
        #
        con = p.ice_getConnection()
        p2 = p.ice_batchOneway()
        p2.ice_ping()
        r = con.begin_flushBatchRequests()
        test(r.getConnection() == con)
        test(r.getCommunicator() == communicator)
        test(r.getProxy() == None) # Expected
        con.end_flushBatchRequests(r)

    #
    # Batch request via communicator
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    r = communicator.begin_flushBatchRequests()
    test(r.getConnection() == None) # Expected
    test(r.getCommunicator() == communicator)
    test(r.getProxy() == None) # Expected
    communicator.end_flushBatchRequests(r)

    if(p.ice_getConnection()):
        r1 = None;
        r2 = None;

        if sys.version_info[0] == 2:
            b = [chr(random.randint(0, 255)) for x in range(0, 10024)]
            seq = ''.join(b)
        else:
            b = [random.randint(0, 255) for x in range(0, 10024)]
            seq = bytes(b)

        testController.holdAdapter()

        for x in range(0, 200): # 2MB
            r = p.begin_opWithPayload(seq)

        test(not r.isSent())

        r1 = p.begin_ice_ping()
        r2 = p.begin_ice_id()
        r1.cancel()
        r2.cancel()
        try:
            p.end_ice_ping(r1)
            test(false)
        except(Ice.InvocationCanceledException):
            pass

        try:
            p.end_ice_id(r2)
            test(false)
        except(Ice.InvocationCanceledException):
            pass

        testController.resumeAdapter()
        p.ice_ping()
        test(not r1.isSent() and r1.isCompleted())
        test(not r2.isSent() and r2.isCompleted())

        testController.holdAdapter()

        r1 = p.begin_op()
        r2 = p.begin_ice_id()
        r1.waitForSent()
        r2.waitForSent()
        r1.cancel()
        r2.cancel()
        try:
            p.end_op(r1)
            test(false)
        except:
            pass
        try:
            p.end_ice_id(r2)
            test(false)
        except:
            pass
        testController.resumeAdapter()

    print("ok")

    if p.ice_getConnection():
        sys.stdout.write("testing close connection with sending queue... ")
        sys.stdout.flush()

        if sys.version_info[0] == 2:
            b = [chr(random.randint(0, 255)) for x in range(0, 10*1024)]
            seq = ''.join(b)
        else:
            b = [random.randint(0, 255) for x in range(0, 10*1024)]
            seq = bytes(b)

        #
        # Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
        # The goal is to make sure that none of the opWithPayload fail even if the server closes
        # the connection gracefully in between.
        #
        maxQueue = 2
        done = False
        while not done and maxQueue < 50:
            done = True
            p.ice_ping()
            results = []
            for i in range(0, maxQueue):
                results.append(p.begin_opWithPayload(seq))
            if not p.begin_close(False).isSent():
                for i in range(0, maxQueue):
                    r = p.begin_opWithPayload(seq)
                    results.append(r)
                    if r.isSent():
                        done = False
                        maxQueue = maxQueue * 2
                        break
            else:
                maxQueue = maxQueue * 2
                done = False
            for r in results:
                r.waitForCompleted()
                try:
                    r.throwLocalException()
                except Ice.LocalException:
                    test(False)

        print("ok")

    p.shutdown()
