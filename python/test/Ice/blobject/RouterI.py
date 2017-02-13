# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import threading, Ice

class CallQueue(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self._condVar = threading.Condition()
        self._queue = []
        self._destroy = False

    def add(self, call):
        self._condVar.acquire()
        self._queue.append(call)
        self._condVar.notify()
        self._condVar.release()

    def destroy(self):
        self._condVar.acquire()
        self._destroy = True
        self._condVar.notify()
        self._condVar.release()

    def run(self):
        while True:
            self._condVar.acquire()
            while len(self._queue) == 0 and not self._destroy:
                self._condVar.wait()
            if self._destroy:
                self._condVar.release()
                break
            call = self._queue.pop()
            self._condVar.release()
            call.execute()

class AsyncCallback(object):
    def __init__(self, cb):
        self._cb = cb

    def response(self, ok, results):
        self._cb.ice_response(ok, results)

    def exception(self, ex):
        self._cb.ice_exception(ex)

class BlobjectCall(object):
    def __init__(self, proxy, amdCallback, inParams, curr):
        self._proxy = proxy
        self._amdCallback = amdCallback
        self._inParams = inParams
        self._curr = curr

    def execute(self):
        proxy = self._proxy
        if len(self._curr.facet) > 0:
            proxy = self._proxy.ice_facet(self._curr.facet)

        if "_fwd" in self._curr.ctx and self._curr.ctx["_fwd"] == "o":
            proxy = proxy.ice_oneway()
            try:
                ok, out = proxy.ice_invoke(self._curr.operation, self._curr.mode, self._inParams, self._curr.ctx)
                self._amdCallback.ice_response(ok, out)
            except Ice.Exception as e:
                self._amdCallback.ice_exception(e)
        else:
            cb = AsyncCallback(self._amdCallback)
            proxy.begin_ice_invoke(self._curr.operation, self._curr.mode, self._inParams, cb.response, cb.exception,
                                   None, self._curr.ctx)

class BlobjectAsyncI(Ice.BlobjectAsync):
    def __init__(self):
        self._queue = CallQueue()
        self._queue.start()
        self._objects = {}
        self._lock = threading.Lock()

    def ice_invoke_async(self, amdCallback, inParams, curr):
        self._lock.acquire()
        proxy = self._objects[curr.id]
        assert proxy
        self._lock.release()
        self._queue.add(BlobjectCall(proxy, amdCallback, inParams, curr))

    def add(self, proxy):
        self._lock.acquire()
        self._objects[proxy.ice_getIdentity()] = proxy.ice_facet("").ice_twoway().ice_router(None)
        self._lock.release()

    def destroy(self):
        self._lock.acquire()
        self._queue.destroy()
        self._queue.join()
        self._lock.release()

class BlobjectI(Ice.Blobject):
    def __init__(self):
        self._objects = {}
        self._lock = threading.Lock()

    def ice_invoke(self, inParams, curr):
        self._lock.acquire()
        proxy = self._objects[curr.id]
        self._lock.release()

        if len(curr.facet) > 0:
            proxy = proxy.ice_facet(curr.facet)

        try:
            if "_fwd" in curr.ctx and curr.ctx["_fwd"] == "o":
                proxy = proxy.ice_oneway()
                return proxy.ice_invoke(curr.operation, curr.mode, inParams, curr.ctx)
            else:
                return proxy.ice_invoke(curr.operation, curr.mode, inParams, curr.ctx)
        except Ice.Exception as e:
            raise

    def add(self, proxy):
        self._lock.acquire()
        self._objects[proxy.ice_getIdentity()] = proxy.ice_facet("").ice_twoway().ice_router(None)
        self._lock.release()

    def destroy(self):
        pass

class ServantLocatorI(Ice.ServantLocator):
    def __init__(self, blobject):
        self._blobject = blobject

    def locate(self, current):
        return self._blobject # and the cookie

    def finished(self, current, object, cookie):
        pass

    def deactivate(self, s):
        pass

class RouterI(Ice.Router):
    def __init__(self, communicator, sync):
        self._adapter = communicator.createObjectAdapterWithEndpoints("forward", "default -h 127.0.0.1")
        if sync:
            self._blobject = BlobjectI()
        else:
            self._blobject = BlobjectAsyncI()
        self._adapter.addServantLocator(ServantLocatorI(self._blobject), "")
        self._blobjectProxy = self._adapter.addWithUUID(self._blobject)
        proxy = Ice.RouterPrx.uncheckedCast(self._adapter.addWithUUID(self))
        communicator.setDefaultRouter(proxy)
        self._adapter.activate()

    def useSync(self, sync):
        self._locator.useSync(sync)

    def getClientProxy(self, current):
        return self._blobjectProxy

    def getServerProxy(self, current):
        assert false

    def addProxies(self, proxies, current):
        for p in proxies:
            self._blobject.add(p)

    def destroy(self):
        self._blobject.destroy()
