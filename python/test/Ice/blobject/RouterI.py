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
        with self._condVar:
            self._queue.append(call)
            self._condVar.notify()

    def destroy(self):
        with self._condVar:
            self._destroy = True
            self._condVar.notify()

    def run(self):
        while True:
            with self._condVar:
                while len(self._queue) == 0 and not self._destroy:
                    self._condVar.wait()
                if self._destroy:
                    break
                call = self._queue.pop()
            call.execute()

class BlobjectCall(object):
    def __init__(self, proxy, future, inParams, curr):
        self._proxy = proxy
        self._future = future
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
                self._future.set_result((ok, out))
            except Ice.Exception as e:
                self._future.set_exception(e)
        else:
            f = proxy.ice_invokeAsync(self._curr.operation, self._curr.mode, self._inParams, self._curr.ctx)
            f.add_done_callback(self.done)

    def done(self, future):
        try:
            (ok, bytes) = future.result()
            self._future.set_result((ok, bytes))
        except Exception as ex:
            self._future.set_exception(ex)

class BlobjectAsyncI(Ice.BlobjectAsync):
    def __init__(self):
        self._queue = CallQueue()
        self._queue.start()
        self._objects = {}
        self._lock = threading.Lock()

    def ice_invoke(self, inParams, curr):
        f = Ice.Future()
        with self._lock:
            proxy = self._objects[curr.id]
            assert proxy
            self._queue.add(BlobjectCall(proxy, f, inParams, curr))
        return f

    def add(self, proxy):
        with self._lock:
            self._objects[proxy.ice_getIdentity()] = proxy.ice_facet("").ice_twoway().ice_router(None)

    def destroy(self):
        with self._lock:
            self._queue.destroy()
            self._queue.join()

class BlobjectI(Ice.Blobject):
    def __init__(self):
        self._objects = {}
        self._lock = threading.Lock()

    def ice_invoke(self, inParams, curr):
        with self._lock:
            proxy = self._objects[curr.id]

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
        with self._lock:
            self._objects[proxy.ice_getIdentity()] = proxy.ice_facet("").ice_twoway().ice_router(None)

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
        return (self._blobjectProxy, True)

    def getServerProxy(self, current):
        assert false

    def addProxies(self, proxies, current):
        for p in proxies:
            self._blobject.add(p)

    def destroy(self):
        self._blobject.destroy()
