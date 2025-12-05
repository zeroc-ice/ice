# Copyright (c) ZeroC, Inc.

import sys
from typing import Any, cast

from generated.test.Ice.adapterDeactivation import Test
from TestHelper import TestHelper

import Ice


def test(b: Any) -> None:
    if not b:
        raise RuntimeError("test assertion failed")


def allTests(helper: TestHelper, communicator: Ice.Communicator) -> Test.TestIntfPrx:
    obj = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint()}")

    sys.stdout.write("creating/destroying/recreating object adapter... ")
    sys.stdout.flush()
    adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default")
    try:
        communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default")
        test(False)
    except Ice.LocalException:
        pass
    adapter.destroy()

    adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default")
    adapter.destroy()
    print("ok")

    sys.stdout.write("creating/activating/deactivating object adapter in one operation... ")
    sys.stdout.flush()
    obj.transient()
    print("ok")

    sys.stdout.write("testing object adapter deactivation... ")
    sys.stdout.flush()
    adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default")
    adapter.activate()
    test(not adapter.isDeactivated())
    adapter.deactivate()
    test(adapter.isDeactivated())
    adapter.destroy()
    print("ok")

    sys.stdout.write("testing connection closure... ")
    sys.stdout.flush()
    for _ in range(10):
        initData = Ice.InitializationData()
        initData.properties = communicator.getProperties().clone()
        comm = Ice.Communicator(initData=initData)
        prx = comm.stringToProxy("test:{0}".format(helper.getTestEndpoint()))
        assert prx is not None
        prx.ice_pingAsync()
        comm.destroy()
    print("ok")

    sys.stdout.write("testing object adapter published endpoints... ")
    sys.stdout.flush()

    communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000")
    adapter = communicator.createObjectAdapter("PAdapter")
    test(len(adapter.getPublishedEndpoints()) == 1)
    endpt = adapter.getPublishedEndpoints()[0]
    test(str(endpt) == "tcp -h localhost -p 12345 -t 30000")
    prx = communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000")
    assert prx is not None
    adapter.setPublishedEndpoints(prx.ice_getEndpoints())
    test(len(adapter.getPublishedEndpoints()) == 2)
    ident = Ice.Identity()
    ident.name = "dummy"
    test(adapter.createProxy(ident).ice_getEndpoints() == prx.ice_getEndpoints())
    test(adapter.getPublishedEndpoints() == prx.ice_getEndpoints())
    adapter.destroy()
    test(len(adapter.getPublishedEndpoints()) == 0)

    print("ok")

    if obj.ice_getConnection():
        sys.stdout.write("testing object adapter with bi-dir connection... ")
        sys.stdout.flush()

        test(communicator.getDefaultObjectAdapter() is None)
        cachedConnection = obj.ice_getCachedConnection()
        assert cachedConnection is not None
        assert cachedConnection.getAdapter() is None

        adapter = communicator.createObjectAdapter("")

        communicator.setDefaultObjectAdapter(adapter)
        # TODO: compare underlying object adapter objects
        test(communicator.getDefaultObjectAdapter() is not None)

        # create new connection
        cachedConnection = obj.ice_getCachedConnection()
        assert cachedConnection is not None
        cast(Ice.Future[None], cachedConnection.close()).result()
        obj.ice_ping()

        # TODO: compare underlying object adapter objects
        cachedConnection = obj.ice_getCachedConnection()
        assert cachedConnection is not None
        assert cachedConnection.getAdapter() is not None
        communicator.setDefaultObjectAdapter(None)

        # create new connection
        cachedConnection = obj.ice_getCachedConnection()
        assert cachedConnection is not None
        cast(Ice.Future[None], cachedConnection.close()).result()
        obj.ice_ping()

        cachedConnection = obj.ice_getCachedConnection()
        assert cachedConnection is not None
        assert cachedConnection.getAdapter() is None
        cachedConnection.setAdapter(adapter)
        # TODO: compare underlying object adapter objects
        cachedConnection = obj.ice_getCachedConnection()
        assert cachedConnection is not None
        assert cachedConnection.getAdapter() is not None
        cachedConnection.setAdapter(None)

        adapter.destroy()
        try:
            con = obj.ice_getConnection()
            assert con is not None
            con.setAdapter(adapter)
            test(False)
        except Ice.ObjectAdapterDestroyedException:
            pass
        print("ok")

    sys.stdout.write("testing object adapter with router... ")
    sys.stdout.flush()
    routerId = Ice.Identity()
    routerId.name = "router"
    router = Ice.RouterPrx.uncheckedCast(obj.ice_identity(routerId).ice_connectionId("rc"))
    adapter = communicator.createObjectAdapterWithRouter("", router)
    test(len(adapter.getPublishedEndpoints()) == 1)
    test(str(adapter.getPublishedEndpoints()[0]) == "tcp -h localhost -p 23456 -t 30000")
    try:
        adapter.setPublishedEndpoints(router.ice_getEndpoints())
        test(False)
    except Exception:
        # Expected.
        pass
    adapter.destroy()

    try:
        routerId.name = "test"
        router = Ice.RouterPrx.uncheckedCast(obj.ice_identity(routerId))
        communicator.createObjectAdapterWithRouter("", router)
        test(False)
    except Ice.OperationNotExistException:
        # Expected: the "test" object doesn't implement Ice::Router!
        pass
    print("ok")

    sys.stdout.write("deactivating object adapter in the server... ")
    sys.stdout.flush()
    obj.deactivate()
    print("ok")

    sys.stdout.write("testing whether server is gone... ")
    sys.stdout.flush()
    if obj.ice_getConnection() is None:  # collocated
        obj.ice_ping()
        print("ok")
    else:
        try:
            obj.ice_invocationTimeout(100).ice_ping()  # Use timeout to speed up testing on Windows
            test(False)
        except Ice.LocalException:
            print("ok")

    return obj
