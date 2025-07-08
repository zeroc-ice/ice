# Copyright (c) ZeroC, Inc.

import Ice
import Test
import sys


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def allTests(helper, communicator, collocated):
    proxy = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint()}")

    sys.stdout.write("testing current.adapter... ")
    sys.stdout.flush()
    test(proxy.getAdapterName() == "TestAdapter")
    print("ok")

    sys.stdout.write("testing current.con... ")
    sys.stdout.flush()
    if collocated:
        test(proxy.getConnection() == "")
    else:
        test("IcePy.Connection" in proxy.getConnection())
    print("ok")

    sys.stdout.write("testing current.id... ")
    sys.stdout.flush()
    test(Ice.Identity("test", "") == proxy.getIdentity())
    print("ok")

    sys.stdout.write("testing current.facet... ")
    sys.stdout.flush()
    test("foo" == Test.TestIntfPrx.uncheckedCast(proxy.ice_facet("foo")).getFacet())
    print("ok")

    sys.stdout.write("testing current.operation... ")
    sys.stdout.flush()
    test("getOperation" == proxy.getOperation())
    print("ok")

    sys.stdout.write("testing current.mode... ")
    sys.stdout.flush()
    test("Ice.OperationMode.Normal" == proxy.getMode())
    print("ok")

    sys.stdout.write("testing current.ctx... ")
    sys.stdout.flush()
    ctx = {"foo": "bar"}
    test(ctx == proxy.getContext(ctx))
    print("ok")

    sys.stdout.write("testing current.requestId... ")
    sys.stdout.flush()
    if collocated:
        test(7 == proxy.getRequestId())
    else:
        proxy.ice_getConnection().close().result()
        test(1 == proxy.getRequestId())
    print("ok")

    sys.stdout.write("testing current.encoding... ")
    sys.stdout.flush()
    test(str(proxy.ice_getEncodingVersion()) == proxy.getEncoding())
    print("ok")

    proxy.shutdown()

    communicator.shutdown()
    communicator.waitForShutdown()
