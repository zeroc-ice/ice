#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice, Test, sys, TestI

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(helper, communicator):

    ref = "factory:{0} -t 10000".format(helper.getTestEndpoint())
    factory = Test.RemoteCommunicatorFactoryPrx.checkedCast(communicator.stringToProxy(ref))

    sys.stdout.write("testing thread hooks... ")
    sys.stdout.flush()

    #
    # Set the maximum size of the server-side thread pool in the new communicator to 5.
    #
    props = {}
    props["Ice.ThreadPool.Server.SizeMax"] = "5"
    com = factory.createCommunicator(props)

    obj = com.getObject()

    startCount = com.getThreadHookStartCount()

    #
    # Start 5 async invocations that sleep for a little while to force new threads to be created.
    #
    reqs = []
    for i in range(0, 5):
        reqs.append(obj.sleepAsync(100))
    for f in reqs:
        f.result()

    #
    # The remote thread hook should detect at least 4 more threads. There could be more for other Ice threads.
    #
    test(com.getThreadHookStartCount() - startCount >= 4)
    test(com.getThreadStartCount() - startCount >= 4)

    #
    # Destroy the remote communicator to force the destruction of the thread pool.
    #
    com.destroy()

    #
    # Finally, make sure we detected the same number of stops as starts.
    #
    test(com.getThreadHookStopCount() == com.getThreadHookStartCount())
    test(com.getThreadStopCount() == com.getThreadStartCount())
    test(com.getThreadHookStartCount() == com.getThreadStartCount())

    print("ok")

    factory.shutdown()
