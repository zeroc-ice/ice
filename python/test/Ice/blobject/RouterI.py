# Copyright (c) ZeroC, Inc.

import threading
from collections.abc import Awaitable, Callable, Sequence
from typing import override

import Ice


class BlobjectCall(object):
    def __init__(self, proxy: Ice.ObjectPrx, future: Ice.Future, inParams: bytes, curr: Ice.Current):
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
                ok, out = proxy.ice_invoke(
                    self._curr.operation,
                    self._curr.mode,
                    self._inParams,
                    self._curr.ctx,
                )
                self._future.set_result((ok, out))
            except Ice.Exception as e:
                self._future.set_exception(e)
        else:
            f = proxy.ice_invokeAsync(self._curr.operation, self._curr.mode, self._inParams, self._curr.ctx)
            assert isinstance(f, Ice.Future)
            f.add_done_callback(self.done)

    def done(self, future: Ice.Future):
        try:
            (ok, bytes) = future.result()
            self._future.set_result((ok, bytes))
        except Exception as ex:
            self._future.set_exception(ex)


class CallQueue(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self._condVar = threading.Condition()
        self._queue = []
        self._destroy = False

    def add(self, call: BlobjectCall):
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


class BlobjectAsyncI(Ice.Blobject):
    def __init__(self):
        self._queue = CallQueue()
        self._queue.start()
        self._objects = {}
        self._lock = threading.Lock()

    @override
    def ice_invoke(self, bytes: bytes, current: Ice.Current) -> Awaitable[tuple[bool, bytes]]:
        f = Ice.Future()
        with self._lock:
            proxy = self._objects[current.id]
            assert proxy is not None
            self._queue.add(BlobjectCall(proxy, f, bytes, current))
        return f

    def add(self, proxy: Ice.ObjectPrx):
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

    @override
    def ice_invoke(self, bytes: bytes, current: Ice.Current) -> tuple[bool, bytes]:
        with self._lock:
            proxy = self._objects[current.id]

        if len(current.facet) > 0:
            proxy = proxy.ice_facet(current.facet)

        try:
            if "_fwd" in current.ctx and current.ctx["_fwd"] == "o":
                proxy = proxy.ice_oneway()
                return proxy.ice_invoke(current.operation, current.mode, bytes, current.ctx)
            else:
                return proxy.ice_invoke(current.operation, current.mode, bytes, current.ctx)
        except Ice.Exception:
            raise

    def add(self, proxy: Ice.ObjectPrx):
        with self._lock:
            self._objects[proxy.ice_getIdentity()] = proxy.ice_facet("").ice_twoway().ice_router(None)

    def destroy(self):
        pass


class ServantLocatorI(Ice.ServantLocator):
    def __init__(self, blobject: Ice.Blobject):
        self._blobject = blobject

    @override
    def locate(self, current: Ice.Current) -> tuple[Ice.Object | None, object]:
        return self._blobject, None  # and the cookie

    @override
    def finished(self, current: Ice.Current, servant: Ice.Object, cookie: object):
        pass

    @override
    def deactivate(self, category: str):
        pass


class RouterI(Ice.Router):
    def __init__(self, communicator: Ice.Communicator, sync: bool):
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

    @override
    def getClientProxy(self, current: Ice.Current):
        return (self._blobjectProxy, True)

    @override
    def getServerProxy(self, current: Ice.Current):
        assert False

    @override
    def addProxies(self, proxies: list[Ice.ObjectPrx | None], current: Ice.Current) -> Sequence[Ice.ObjectPrx | None]:
        for p in proxies:
            assert p is not None
            self._blobject.add(p)
        return []

    def destroy(self):
        self._blobject.destroy()
