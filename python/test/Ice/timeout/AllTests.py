# Copyright (c) ZeroC, Inc.

import sys

from generated.test.Ice.timeout import Test
from TestHelper import TestHelper, test

import Ice


def connect(prx: Ice.ObjectPrx) -> Ice.Connection | None:
    # Establish connection with the given proxy (which might have a timeout
    # set and might sporadically fail on connection establishment if it's
    # too slow). The loop ensures that the connection is established by retrying
    # in case we can a ConnectTimeoutException
    nRetry = 10
    while --nRetry > 0:
        try:
            prx.ice_getConnection()
            break
        except Ice.ConnectTimeoutException:
            # Can sporadically occur with slow machines
            pass
    assert prx.ice_getConnection()  # Establish connection


def allTests(helper: TestHelper, communicator: Ice.Communicator):
    controller = Test.ControllerPrx(communicator, f"controller:{helper.getTestEndpoint(num=1)}")
    connect(controller)

    try:
        allTestsWithController(helper, communicator, controller)
    except Exception:
        # Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
        # the test from hanging on exit in case a connection which disables timeouts is still opened.
        controller.resumeAdapter()
        raise


def allTestsWithController(helper: TestHelper, communicator: Ice.Communicator, controller: Test.ControllerPrx):
    timeout = Test.TimeoutPrx(communicator, f"timeout:{helper.getTestEndpoint()}")

    sys.stdout.write("testing invocation timeout... ")
    sys.stdout.flush()
    connection = timeout.ice_getConnection()
    to = timeout.ice_invocationTimeout(100)
    test(connection == to.ice_getConnection())
    try:
        to.sleep(1000)
        test(False)
    except Ice.InvocationTimeoutException:
        pass
    timeout.ice_ping()
    to = timeout.ice_invocationTimeout(1000)
    test(connection == to.ice_getConnection())
    try:
        to.sleep(100)
    except Ice.InvocationTimeoutException:
        test(False)
    test(connection == to.ice_getConnection())
    print("ok")

    controller.shutdown()
