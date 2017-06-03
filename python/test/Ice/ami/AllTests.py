# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading, random, logging

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

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
        test(f.result() != None)
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
        except:
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
        self._thread = threading.currentThread()

    def sent(self, f, sentSynchronously):
        test((sentSynchronously and self._thread == threading.currentThread()) or \
             (not sentSynchronously and self._thread != threading.currentThread()))
        self.called()

class FutureFlushCallback(CallbackBase):
    def __init__(self, cookie=None):
        CallbackBase.__init__(self)
        self._thread = threading.currentThread()
        self._cookie = cookie

    def sent(self, f, sentSynchronously):
        test((sentSynchronously and self._thread == threading.currentThread()) or \
             (not sentSynchronously and self._thread != threading.currentThread()))
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

def allTests(communicator, collocated):
    # Ice.Future uses the Python logging facility, this tests throws exceptions from Ice.Future callbacks
    # so we disable errors to prevent them to show up on the console.
    logging.basicConfig()
    logging.disable(logging.ERROR)

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
    result = p.begin_ice_isA("::Test::TestIntf", context=ctx)
    test(p.end_ice_isA(result))

    result = p.begin_ice_ping()
    p.end_ice_ping(result)
    result = p.begin_ice_ping(context=ctx)
    p.end_ice_ping(result)

    result = p.begin_ice_id()
    test(p.end_ice_id(result) == "::Test::TestIntf")
    result = p.begin_ice_id(context=ctx)
    test(p.end_ice_id(result) == "::Test::TestIntf")

    result = p.begin_ice_ids()
    test(len(p.end_ice_ids(result)) == 2)
    result = p.begin_ice_ids(context=ctx)
    test(len(p.end_ice_ids(result)) == 2)

    if not collocated:
        result = p.begin_ice_getConnection()
        test(p.end_ice_getConnection(result) != None)

    result = p.begin_op()
    p.end_op(result)
    result = p.begin_op(context=ctx)
    p.end_op(result)

    result = p.begin_opWithResult()
    test(p.end_opWithResult(result) == 15)
    result = p.begin_opWithResult(context=ctx)
    test(p.end_opWithResult(result) == 15)

    result = p.begin_opWithUE()
    try:
        p.end_opWithUE(result)
        test(False)
    except Test.TestIntfException:
        pass
    result = p.begin_opWithUE(context=ctx)
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
    p.begin_ice_isA(Test.TestIntf.ice_staticId(), cb.isA, cb.ex, context=ctx)
    cb.check()
    p.begin_ice_isA(Test.TestIntf.ice_staticId(), lambda r: cbWC.isA(r, cookie), lambda ex: cbWC.ex(ex, cookie),
                    context=ctx)
    cbWC.check()

    p.begin_ice_ping(cb.ping, cb.ex)
    cb.check()
    p.begin_ice_ping(lambda: cbWC.ping(cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_ice_ping(cb.ping, cb.ex, context=ctx)
    cb.check()
    p.begin_ice_ping(lambda: cbWC.ping(cookie), lambda: cbWC.ex(ex, cookie), context=ctx)
    cbWC.check()

    p.begin_ice_id(cb.id, cb.ex)
    cb.check()
    p.begin_ice_id(lambda id: cbWC.id(id, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_ice_id(cb.id, cb.ex, context=ctx)
    cb.check()
    p.begin_ice_id(lambda id: cbWC.id(id, cookie), lambda ex: cbWC.ex(ex, cookie), context=ctx)
    cbWC.check()

    p.begin_ice_ids(cb.ids, cb.ex)
    cb.check()
    p.begin_ice_ids(lambda ids: cbWC.ids(ids, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_ice_ids(cb.ids, cb.ex, context=ctx)
    cb.check()
    p.begin_ice_ids(lambda ids: cbWC.ids(ids, cookie), lambda ex: cbWC.ex(ex, cookie), context=ctx)
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
    p.begin_op(cb.op, cb.ex, context=ctx)
    cb.check()
    p.begin_op(lambda: cbWC.op(cookie), lambda ex: cbWC.ex(ex, cookie), context=ctx)
    cbWC.check()

    p.begin_opWithResult(cb.opWithResult, cb.ex)
    cb.check()
    p.begin_opWithResult(lambda r: cbWC.opWithResult(r, cookie), lambda ex: cbWC.ex(ex, cookie))
    cbWC.check()
    p.begin_opWithResult(cb.opWithResult, cb.ex, context=ctx)
    cb.check()
    p.begin_opWithResult(lambda r: cbWC.opWithResult(r, cookie), lambda ex: cbWC.ex(ex, cookie), context=ctx)
    cbWC.check()

    p.begin_opWithUE(cb.op, cb.opWithUE)
    cb.check()
    p.begin_opWithUE(lambda: cbWC.op(cookie), lambda ex: cbWC.opWithUE(ex, cookie))
    cbWC.check()
    p.begin_opWithUE(cb.op, cb.opWithUE, context=ctx)
    cb.check()
    p.begin_opWithUE(lambda: cbWC.op(cookie), lambda ex: cbWC.opWithUE(ex, cookie), context=ctx)
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

        def thrower(future):
            try:
                future.result()
            except:
                test(false)
            throwEx(t)
        f = p.opAsync()
        try:
            f.add_done_callback(thrower)
        except Exception as ex:
            try:
                throwEx(t)
            except Exception as ex2:
                test(type(ex) == type(ex2))
        f.add_done_callback_async(thrower)
        f.result()

        p.begin_op(lambda: cb.opWC(cookie), lambda ex: cb.noExWC(ex, cookie))
        cb.check()

        q.begin_op(cb.op, cb.ex)
        cb.check()

        f = q.opAsync()
        def throwerEx(future):
            try:
                future.result()
                test(false)
            except:
                throwEx(t)
        try:
            f.add_done_callback(throwerEx)
        except Exception as ex:
            try:
                throwEx(t)
            except Exception as ex2:
                test(type(ex) == type(ex2))
        f.add_done_callback_async(throwerEx)
        try:
            f.result()
        except:
            pass

        q.begin_op(lambda: cb.opWC(cookie), lambda ex: cb.exWC(ex, cookie))
        cb.check()

        p.begin_op(cb.noOp, cb.ex, cb.sent)
        cb.check()

        f = p.opAsync()
        try:
            f.add_sent_callback(lambda f, s: throwEx(t))
        except Exception as ex:
            try:
                throwEx(t)
            except Exception as ex2:
                test(type(ex) == type(ex2))
        f.add_sent_callback_async(throwerEx)
        f.result()

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
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
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
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
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
        r = b1.ice_getConnection().begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy, cb.exception, cb.sent)
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
        r = b1.ice_getConnection().begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy,
                                                            lambda ex: cb.exceptionWC(ex, cookie),
                                                            lambda ss: cb.sentWC(ss, cookie))
        cb.check()
        test(p.waitForBatch(2))

        #
        # Exception without cookie.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FlushExCallback()
        r = b1.ice_getConnection().begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy, cb.exception, cb.sent)
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
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FlushExCallback(cookie)
        r = b1.ice_getConnection().begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy,
                                                            lambda ex: cb.exceptionWC(ex, cookie),
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
        r = communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy, cb.exception, cb.sent)
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
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy, cb.exception, cb.sent)
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
        r = communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy, cb.exception, cb.sent)
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
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy, cb.exception, cb.sent)
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
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        b2.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FlushCallback()
        r = communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy, cb.exception, cb.sent)
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
        r = con.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy)
        test(r.getConnection() == con)
        test(r.getCommunicator() == communicator)
        test(r.getProxy() == None) # Expected
        con.end_flushBatchRequests(r)

    #
    # Batch request via communicator
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    r = communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy)
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

    if p.ice_getConnection() and p.supportsAMD():

        sys.stdout.write("testing graceful close connection with wait... ")
        sys.stdout.flush()

        #
        # Local case: begin a request, close the connection gracefully, and make sure it waits
        # for the request to complete.
        #
        cb = CallbackBase()
        con = p.ice_getConnection()
        con.setCloseCallback(lambda con: cb.called())
        f = p.sleepAsync(100)
        con.close(Ice.ConnectionClose.GracefullyWithWait) # Blocks until the request completes.
        f.result() # Should complete successfully.
        cb.check()

        #
        # Remote case.
        #
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
            if not p.begin_close(Test.CloseMode.GracefullyWithWait).isSent():
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

        sys.stdout.write("testing graceful close connection without wait... ")
        sys.stdout.flush()

        #
        # Local case: start an operation and then close the connection gracefully on the client side
        # without waiting for the pending invocation to complete. There will be no retry and we expect the
        # invocation to fail with ConnectionManuallyClosedException.
        #
        p = p.ice_connectionId("CloseGracefully") # Start with a new connection.
        con = p.ice_getConnection()
        f = p.startDispatchAsync()
        f.sent() # Ensure the request was sent before we close the connection.
        con.close(Ice.ConnectionClose.Gracefully)
        try:
            f.result()
            test(False)
        except Ice.ConnectionManuallyClosedException as ex:
            test(ex.graceful)
        p.finishDispatch()

        #
        # Remote case: the server closes the connection gracefully, which means the connection
        # will not be closed until all pending dispatched requests have completed.
        #
        con = p.ice_getConnection()
        cb = CallbackBase()
        con.setCloseCallback(lambda c: cb.called())
        f = p.sleepAsync(100)
        p.close(Test.CloseMode.Gracefully) # Close is delayed until sleep completes.
        cb.check() # Ensure connection was closed.
        try:
            f.result()
        except:
            test(false)

        print("ok")

        sys.stdout.write("testing forceful close connection... ")
        sys.stdout.flush()

        #
        # Local case: start an operation and then close the connection forcefully on the client side.
        # There will be no retry and we expect the invocation to fail with ConnectionManuallyClosedException.
        #
        p.ice_ping()
        con = p.ice_getConnection()
        f = p.startDispatchAsync()
        f.sent() # Ensure the request was sent before we close the connection.
        con.close(Ice.ConnectionClose.Forcefully)
        try:
            f.result()
            test(False)
        except Ice.ConnectionManuallyClosedException as ex:
            test(not ex.graceful)
        p.finishDispatch()

        #
        # Remote case: the server closes the connection forcefully. This causes the request to fail
        # with a ConnectionLostException. Since the close() operation is not idempotent, the client
        # will not retry.
        #
        try:
            p.close(Test.CloseMode.Forcefully)
            test(False)
        except Ice.ConnectionLostException:
            # Expected.
            pass

        print("ok")

def allTestsFuture(communicator, collocated):
    sref = "test:default -p 12010"
    obj = communicator.stringToProxy(sref)
    test(obj)

    p = Test.TestIntfPrx.uncheckedCast(obj)

    sref = "testController:default -p 12011"
    obj = communicator.stringToProxy(sref)
    test(obj)

    testController = Test.TestIntfControllerPrx.uncheckedCast(obj)

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
        test(p.ice_getConnectionAsync().result() != None)

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
        ic.destroy();

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

    cbs = []
    if sys.version_info[0] == 2:
        b = [chr(random.randint(0, 255)) for x in range(0, 1024)]
        seq = ''.join(b)
    else:
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
    cb.check()
    test(f.is_sent())
    test(f.done())
    test(p.waitForBatch(2))

    if p.ice_getConnection(): # No collocation optimization
        test(p.opBatchCount() == 0)
        b1 = p.ice_batchOneway()
        b1.opBatch()
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FutureFlushCallback()
        f = b1.ice_flushBatchRequestsAsync()
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result() # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(1))

    print("ok")

    if p.ice_getConnection(): # No collocation optimization
        sys.stdout.write("testing batch requests with connection... ")
        sys.stdout.flush()

        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.opBatch()
        cb = FutureFlushCallback()
        f = b1.ice_getConnection().flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result() # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(2))

        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FutureFlushExCallback()
        f = b1.ice_getConnection().flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
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
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result() # Wait until finished.
        test(f.is_sent())
        test(f.done())
        test(p.waitForBatch(2))

        #
        # 1 connection.
        #
        test(p.opBatchCount() == 0)
        b1 = Test.TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway())
        b1.opBatch()
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result() # Wait until finished.
        test(f.is_sent()) # Exceptions are ignored!
        test(f.done())
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
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result() # Wait until finished.
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
        b2 = Test.TestIntfPrx.uncheckedCast(p.ice_connectionId("2").ice_getConnection().createProxy(
            p.ice_getIdentity()).ice_batchOneway())
        b2.ice_getConnection() # Ensure connection is established.
        b1.opBatch()
        b2.opBatch()
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result() # Wait until finished.
        test(f.is_sent()) # Exceptions are ignored!
        test(f.done())
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
        b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        b2.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        cb = FutureFlushCallback()
        f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        f.add_sent_callback(cb.sent)
        cb.check()
        f.result() # Wait until finished.
        test(f.is_sent()) # Exceptions are ignored!
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
        if sys.version_info[0] == 2:
            b = [chr(random.randint(0, 255)) for x in range(0, 1024)]
            seq = ''.join(b)
        else:
            b = [random.randint(0, 255) for x in range(0, 1024)]
            seq = bytes(b)
        while(True):
            f2 = p.opWithPayloadAsync(seq)
            if not f2.is_sent_synchronously():
                break

        test(f1 == f1)
        test(f1 != f2)

        if p.ice_getConnection():
            test((f1.is_sent_synchronously() and f1.is_sent() and not f1.done()) or
                 (not f1.is_sent_synchronously() and not f1.done()))

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
    test(f.connection() == None) # Expected
    test(f.communicator() == communicator)
    test(f.proxy() == p)
    f.result()

    #
    # Oneway
    #
    p2 = p.ice_oneway()
    f = p2.ice_pingAsync()
    test(f.operation() == "ice_ping")
    test(f.connection() == None) # Expected
    test(f.communicator() == communicator)
    test(f.proxy() == p2)

    #
    # Batch request via proxy
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    f = p2.ice_flushBatchRequestsAsync()
    test(f.connection() == None) # Expected
    test(f.communicator() == communicator)
    test(f.proxy() == p2)
    f.result()

    if p.ice_getConnection():
        #
        # Batch request via connection
        #
        con = p.ice_getConnection()
        p2 = p.ice_batchOneway()
        p2.ice_ping()
        f = con.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
        test(f.connection() == con)
        test(f.communicator() == communicator)
        test(f.proxy() == None) # Expected
        f.result()

    #
    # Batch request via communicator
    #
    p2 = p.ice_batchOneway()
    p2.ice_ping()
    f = communicator.flushBatchRequestsAsync(Ice.CompressBatch.BasedOnProxy)
    test(f.connection() == None) # Expected
    test(f.communicator() == communicator)
    test(f.proxy() == None) # Expected
    f.result()

    if(p.ice_getConnection()):
        f1 = None
        f2 = None

        if sys.version_info[0] == 2:
            b = [chr(random.randint(0, 255)) for x in range(0, 10024)]
            seq = ''.join(b)
        else:
            b = [random.randint(0, 255) for x in range(0, 10024)]
            seq = bytes(b)

        testController.holdAdapter()

        for x in range(0, 200): # 2MB
            f = p.opWithPayloadAsync(seq)

        test(not f.is_sent())

        f1 = p.ice_pingAsync()
        f2 = p.ice_idAsync()
        f1.cancel()
        f2.cancel()
        try:
            f1.result()
            test(false)
        except(Ice.InvocationCanceledException):
            pass

        try:
            f2.result()
            test(false)
        except(Ice.InvocationCanceledException):
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
            test(false)
        except:
            pass
        try:
            f2.result()
            test(false)
        except:
            pass
        testController.resumeAdapter()

    print("ok")

    p.shutdown()
