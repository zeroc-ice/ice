# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, random, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class GetAdapterNameCB:
    def __init__(self):
        self._name = ""
        self._cond = threading.Condition()

    def response(self, name):
        self._cond.acquire()
        self._name = name
        self._cond.notify()
        self._cond.release()

    def exception(self, ex):
        test(False)

    def getResult(self):
        self._cond.acquire()
        try:
            while self._name == "":
                self._cond.wait(5.0)
            if self._name != "":
                return self._name
            else:
                return ""
        finally:
            self._cond.release()

def getAdapterNameWithAMI(proxy):
    cb = GetAdapterNameCB()
    proxy.begin_getAdapterName(cb.response, cb.exception)
    return cb.getResult()
    
def createTestIntfPrx(adapters):
    endpoints = []
    test = None
    for p in adapters:
        test = p.getTestIntf()
        edpts = test.ice_getEndpoints()
        endpoints.extend(edpts)
    return Test.TestIntfPrx.uncheckedCast(test.ice_endpoints(endpoints))

def deactivate(com, adapters):
    for p in adapters:
        com.deactivateObjectAdapter(p)

def allTests(communicator):
    ref = "communicator:default -p 12010"
    com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref))

    sys.stdout.write("testing binding with single endpoint... ")
    sys.stdout.flush()

    adapter = com.createObjectAdapter("Adapter", "default")

    test1 = adapter.getTestIntf()
    test2 = adapter.getTestIntf()
    test(test1.ice_getConnection() == test2.ice_getConnection())

    test1.ice_ping()
    test2.ice_ping()
    
    com.deactivateObjectAdapter(adapter)
    
    test3 = Test.TestIntfPrx.uncheckedCast(test1)
    test(test3.ice_getConnection() == test1.ice_getConnection())
    test(test3.ice_getConnection() == test2.ice_getConnection())

    try:
        test3.ice_ping()
        test(False)
    except Ice.ConnectionRefusedException:
        pass

    print("ok")

    sys.stdout.write("testing binding with multiple endpoints... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("Adapter11", "default"))
    adapters.append(com.createObjectAdapter("Adapter12", "default"))
    adapters.append(com.createObjectAdapter("Adapter13", "default"))

    #
    # Ensure that when a connection is opened it's reused for new
    # proxies and that all endpoints are eventually tried.
    #
    names = ["Adapter11", "Adapter12", "Adapter13"]
    while len(names) > 0:
        adpts = adapters[:]

        test1 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test2 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test3 = createTestIntfPrx(adpts)

        test(test1.ice_getConnection() == test2.ice_getConnection())
        test(test2.ice_getConnection() == test3.ice_getConnection())

        name = test1.getAdapterName()
        if names.count(name) > 0:
            names.remove(name)
        test1.ice_getConnection().close(False)

    #
    # Ensure that the proxy correctly caches the connection (we
    # always send the request over the same connection.)
    #
    for a in adapters:
        a.getTestIntf().ice_ping()

    t = createTestIntfPrx(adapters)
    name = t.getAdapterName()
    i = 0
    nRetry = 5
    while i < nRetry and t.getAdapterName() == name:
        i = i + 1
    test(i == nRetry)
    
    for a in adapters:
        a.getTestIntf().ice_getConnection().close(False)
        
    #
    # Deactivate an adapter and ensure that we can still
    # establish the connection to the remaining adapters.
    #
    com.deactivateObjectAdapter(adapters[0])
    names.append("Adapter12")
    names.append("Adapter13")
    while len(names) > 0:
        adpts = adapters[:]

        test1 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test2 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test3 = createTestIntfPrx(adpts)

        test(test1.ice_getConnection() == test2.ice_getConnection())
        test(test2.ice_getConnection() == test3.ice_getConnection())

        name = test1.getAdapterName()
        if names.count(name) > 0:
            names.remove(name)
        test1.ice_getConnection().close(False)

    #
    # Deactivate an adapter and ensure that we can still
    # establish the connection to the remaining adapters.
    #
    com.deactivateObjectAdapter(adapters[2])    
    t = createTestIntfPrx(adapters)
    test(t.getAdapterName() == "Adapter12")

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing binding with multiple endpoints and AMI... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("AdapterAMI11", "default"))
    adapters.append(com.createObjectAdapter("AdapterAMI12", "default"))
    adapters.append(com.createObjectAdapter("AdapterAMI13", "default"))

    #
    # Ensure that when a connection is opened it's reused for new
    # proxies and that all endpoints are eventually tried.
    #
    names = ["AdapterAMI11", "AdapterAMI12", "AdapterAMI13"]
    while len(names) > 0:
        adpts = adapters[:]

        test1 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test2 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test3 = createTestIntfPrx(adpts)

        test(test1.ice_getConnection() == test2.ice_getConnection())
        test(test2.ice_getConnection() == test3.ice_getConnection())

        name = getAdapterNameWithAMI(test1)
        if names.count(name) > 0:
            names.remove(name)
        test1.ice_getConnection().close(False)

    #
    # Ensure that the proxy correctly caches the connection (we
    # always send the request over the same connection.)
    #
    for a in adapters:
        a.getTestIntf().ice_ping()

    t = createTestIntfPrx(adapters)
    name = getAdapterNameWithAMI(t)
    i = 0
    nRetry = 5
    while i < nRetry and getAdapterNameWithAMI(t) == name:
        i = i + 1
    test(i == nRetry)
    
    for a in adapters:
        a.getTestIntf().ice_getConnection().close(False)
        
    #
    # Deactivate an adapter and ensure that we can still
    # establish the connection to the remaining adapters.
    #
    com.deactivateObjectAdapter(adapters[0])
    names.append("AdapterAMI12")
    names.append("AdapterAMI13")
    while len(names) > 0:
        adpts = adapters[:]

        test1 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test2 = createTestIntfPrx(adpts)
        random.shuffle(adpts)
        test3 = createTestIntfPrx(adpts)

        test(test1.ice_getConnection() == test2.ice_getConnection())
        test(test2.ice_getConnection() == test3.ice_getConnection())

        name = getAdapterNameWithAMI(test1)
        if names.count(name) > 0:
            names.remove(name)
        test1.ice_getConnection().close(False)

    #
    # Deactivate an adapter and ensure that we can still
    # establish the connection to the remaining adapters.
    #
    com.deactivateObjectAdapter(adapters[2])    
    t = createTestIntfPrx(adapters)
    test(getAdapterNameWithAMI(t) == "AdapterAMI12")

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing random endpoint selection... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("Adapter21", "default"))
    adapters.append(com.createObjectAdapter("Adapter22", "default"))
    adapters.append(com.createObjectAdapter("Adapter23", "default"))

    t = createTestIntfPrx(adapters)
    test(t.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)

    names = ["Adapter21", "Adapter22", "Adapter23"]
    while len(names) > 0:
        name = t.getAdapterName()
        if names.count(name) > 0:
            names.remove(name)
        t.ice_getConnection().close(False)

    t = Test.TestIntfPrx.uncheckedCast(t.ice_endpointSelection(Ice.EndpointSelectionType.Random))
    test(t.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)

    names.append("Adapter21")
    names.append("Adapter22")
    names.append("Adapter23")
    while len(names) > 0:
        name = t.getAdapterName()
        if names.count(name) > 0:
            names.remove(name)
        t.ice_getConnection().close(False)

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing ordered endpoint selection... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("Adapter31", "default"))
    adapters.append(com.createObjectAdapter("Adapter32", "default"))
    adapters.append(com.createObjectAdapter("Adapter33", "default"))

    t = createTestIntfPrx(adapters)
    t = Test.TestIntfPrx.uncheckedCast(t.ice_endpointSelection(Ice.EndpointSelectionType.Ordered))
    test(t.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered)
    nRetry = 5

    #
    # Ensure that endpoints are tried in order by deactivating the adapters
    # one after the other.
    #
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter31":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[0])
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter32":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[1])
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter33":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[2])

    try:
        t.getAdapterName()
    except Ice.ConnectionRefusedException:
        pass

    endpoints = t.ice_getEndpoints()

    adapters = []

    #
    # Now, re-activate the adapters with the same endpoints in the opposite
    # order.
    # 
    adapters.append(com.createObjectAdapter("Adapter36", endpoints[2].toString()))
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter36":
        i = i + 1
    test(i == nRetry)
    t.ice_getConnection().close(False)
    adapters.append(com.createObjectAdapter("Adapter35", endpoints[1].toString()))
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter35":
        i = i + 1
    test(i == nRetry)
    t.ice_getConnection().close(False)
    adapters.append(com.createObjectAdapter("Adapter34", endpoints[0].toString()))
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter34":
        i = i + 1
    test(i == nRetry)

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing per request binding with single endpoint... ")
    sys.stdout.flush()

    adapter = com.createObjectAdapter("Adapter41", "default")

    test1 = Test.TestIntfPrx.uncheckedCast(adapter.getTestIntf().ice_connectionCached(False))
    test2 = Test.TestIntfPrx.uncheckedCast(adapter.getTestIntf().ice_connectionCached(False))
    test(not test1.ice_isConnectionCached())
    test(not test2.ice_isConnectionCached())
    test(test1.ice_getConnection() == test2.ice_getConnection())

    test1.ice_ping()

    com.deactivateObjectAdapter(adapter)

    test3 = Test.TestIntfPrx.uncheckedCast(test1)
    try:
        test(test3.ice_getConnection() == test1.ice_getConnection())
        test(False)
    except Ice.ConnectionRefusedException:
        pass

    print("ok")

    sys.stdout.write("testing per request binding with multiple endpoints... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("Adapter51", "default"))
    adapters.append(com.createObjectAdapter("Adapter52", "default"))
    adapters.append(com.createObjectAdapter("Adapter53", "default"))

    t = Test.TestIntfPrx.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(False))
    test(not t.ice_isConnectionCached())

    names = ["Adapter51", "Adapter52", "Adapter53"]
    while len(names) > 0:
        name = t.getAdapterName()
        if names.count(name) > 0:
            names.remove(name)

    com.deactivateObjectAdapter(adapters[0])

    names.append("Adapter52")
    names.append("Adapter53")
    while len(names) > 0:
        name = t.getAdapterName()
        if names.count(name) > 0:
            names.remove(name)

    com.deactivateObjectAdapter(adapters[2])

    test(t.getAdapterName() == "Adapter52")

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing per request binding with multiple endpoints and AMI... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("AdapterAMI51", "default"))
    adapters.append(com.createObjectAdapter("AdapterAMI52", "default"))
    adapters.append(com.createObjectAdapter("AdapterAMI53", "default"))

    t = Test.TestIntfPrx.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(False))
    test(not t.ice_isConnectionCached())

    names = ["AdapterAMI51", "AdapterAMI52", "AdapterAMI53"]
    while len(names) > 0:
        name = getAdapterNameWithAMI(t)
        if names.count(name) > 0:
            names.remove(name)

    com.deactivateObjectAdapter(adapters[0])

    names.append("AdapterAMI52")
    names.append("AdapterAMI53")
    while len(names) > 0:
        name = getAdapterNameWithAMI(t)
        if names.count(name) > 0:
            names.remove(name)

    com.deactivateObjectAdapter(adapters[2])

    test(getAdapterNameWithAMI(t) == "AdapterAMI52")

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing per request binding and ordered endpoint selection... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("Adapter61", "default"))
    adapters.append(com.createObjectAdapter("Adapter62", "default"))
    adapters.append(com.createObjectAdapter("Adapter63", "default"))

    t = createTestIntfPrx(adapters)
    t = Test.TestIntfPrx.uncheckedCast(t.ice_endpointSelection(Ice.EndpointSelectionType.Ordered))
    test(t.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered)
    t = Test.TestIntfPrx.uncheckedCast(t.ice_connectionCached(False))
    test(not t.ice_isConnectionCached())
    nRetry = 5

    #
    # Ensure that endpoints are tried in order by deactiving the adapters
    # one after the other.
    #
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter61":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[0])
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter62":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[1])
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter63":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[2])

    try:
        t.getAdapterName()
    except Ice.ConnectionRefusedException:
        pass

    endpoints = t.ice_getEndpoints()

    adapters = []

    #
    # Now, re-activate the adapters with the same endpoints in the opposite
    # order.
    # 
    adapters.append(com.createObjectAdapter("Adapter66", endpoints[2].toString()))
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter66":
        i = i + 1
    test(i == nRetry)
    adapters.append(com.createObjectAdapter("Adapter65", endpoints[1].toString()))
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter65":
        i = i + 1
    test(i == nRetry)
    adapters.append(com.createObjectAdapter("Adapter64", endpoints[0].toString()))
    i = 0
    while i < nRetry and t.getAdapterName() == "Adapter64":
        i = i + 1
    test(i == nRetry)

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing per request binding and ordered endpoint selection and AMI... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("AdapterAMI61", "default"))
    adapters.append(com.createObjectAdapter("AdapterAMI62", "default"))
    adapters.append(com.createObjectAdapter("AdapterAMI63", "default"))

    t = createTestIntfPrx(adapters)
    t = Test.TestIntfPrx.uncheckedCast(t.ice_endpointSelection(Ice.EndpointSelectionType.Ordered))
    test(t.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered)
    t = Test.TestIntfPrx.uncheckedCast(t.ice_connectionCached(False))
    test(not t.ice_isConnectionCached())
    nRetry = 5

    #
    # Ensure that endpoints are tried in order by deactiving the adapters
    # one after the other.
    #
    i = 0
    while i < nRetry and getAdapterNameWithAMI(t) == "AdapterAMI61":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[0])
    i = 0
    while i < nRetry and getAdapterNameWithAMI(t) == "AdapterAMI62":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[1])
    i = 0
    while i < nRetry and getAdapterNameWithAMI(t) == "AdapterAMI63":
        i = i + 1
    test(i == nRetry)
    com.deactivateObjectAdapter(adapters[2])

    try:
        t.getAdapterName()
    except Ice.ConnectionRefusedException:
        pass

    endpoints = t.ice_getEndpoints()

    adapters = []

    #
    # Now, re-activate the adapters with the same endpoints in the opposite
    # order.
    # 
    adapters.append(com.createObjectAdapter("AdapterAMI66", endpoints[2].toString()))
    i = 0
    while i < nRetry and getAdapterNameWithAMI(t) == "AdapterAMI66":
        i = i + 1
    test(i == nRetry)
    adapters.append(com.createObjectAdapter("AdapterAMI65", endpoints[1].toString()))
    i = 0
    while i < nRetry and getAdapterNameWithAMI(t) == "AdapterAMI65":
        i = i + 1
    test(i == nRetry)
    adapters.append(com.createObjectAdapter("AdapterAMI64", endpoints[0].toString()))
    i = 0
    while i < nRetry and getAdapterNameWithAMI(t) == "AdapterAMI64":
        i = i + 1
    test(i == nRetry)

    deactivate(com, adapters)

    print("ok")

    sys.stdout.write("testing endpoint mode filtering... ")
    sys.stdout.flush()

    adapters = []
    adapters.append(com.createObjectAdapter("Adapter71", "default"))
    adapters.append(com.createObjectAdapter("Adapter72", "udp"))

    t = createTestIntfPrx(adapters)
    test(t.getAdapterName() == "Adapter71")

    testUDP = Test.TestIntfPrx.uncheckedCast(t.ice_datagram())
    test(t.ice_getConnection() != testUDP.ice_getConnection())
    try:
        testUDP.getAdapterName()
    except Ice.TwowayOnlyException:
        pass

    print("ok")

    if(len(communicator.getProperties().getProperty("Ice.Plugin.IceSSL")) > 0):
        sys.stdout.write("testing unsecure vs. secure endpoints... ")
        sys.stdout.flush()

        adapters = []
        adapters.append(com.createObjectAdapter("Adapter81", "ssl"))
        adapters.append(com.createObjectAdapter("Adapter82", "tcp"))
        
        t = createTestIntfPrx(adapters)
        for i in range(0, 5):
            test(t.getAdapterName() == "Adapter82")
            t.ice_getConnection().close(False)

        testSecure = Test.TestIntfPrx.uncheckedCast(t.ice_secure(True))
        test(testSecure.ice_isSecure())
        testSecure = Test.TestIntfPrx.uncheckedCast(t.ice_secure(False))
        test(not testSecure.ice_isSecure())
        testSecure = Test.TestIntfPrx.uncheckedCast(t.ice_secure(True))
        test(testSecure.ice_isSecure())
        test(t.ice_getConnection() != testSecure.ice_getConnection())

        com.deactivateObjectAdapter(adapters[1])

        for i in range(0, 5):
            test(t.getAdapterName() == "Adapter81")
            t.ice_getConnection().close(False)

        com.createObjectAdapter("Adapter83", (t.ice_getEndpoints()[1]).toString()) # Reactive tcp OA.

        for i in range(0, 5):
            test(t.getAdapterName() == "Adapter83")
            t.ice_getConnection().close(False)

        com.deactivateObjectAdapter(adapters[0])
        try:
            testSecure.ice_ping()
            test(False)
        except Ice.ConnectionRefusedException:
            pass

        deactivate(com, adapters)

        print("ok")

    com.shutdown()
