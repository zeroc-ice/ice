# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def testSetACM(communicator, com)
    print "testing setACM/getACM... "
    STDOUT.flush

    adapter = com.createObjectAdapter(-1, -1, -1)

    initData = Ice::InitializationData.new
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.ACM.Timeout", "1")
    initData.properties.setProperty("Ice.ACM.Client.Timeout", "15")
    initData.properties.setProperty("Ice.ACM.Client.Close", "4")
    initData.properties.setProperty("Ice.ACM.Client.Heartbeat", "2")
    testCommunicator = Ice::initialize(initData)
    proxy = Test::TestIntfPrx::uncheckedCast(testCommunicator.stringToProxy(adapter.getTestIntf().ice_toString()))
    proxy.ice_getConnection()

    begin
        proxy.ice_getCachedConnection().setACM(-19, Ice::Unset, Ice::Unset)
        test(false)
    rescue
    end

    acm = proxy.ice_getCachedConnection().getACM()
    test(acm.timeout == 15)
    test(acm.close == Ice::ACMClose::CloseOnIdleForceful)
    test(acm.heartbeat == Ice::ACMHeartbeat::HeartbeatOnIdle)

    proxy.ice_getCachedConnection().setACM(Ice::Unset, Ice::Unset, Ice::Unset)
    acm = proxy.ice_getCachedConnection().getACM()
    test(acm.timeout == 15)
    test(acm.close == Ice::ACMClose::CloseOnIdleForceful)
    test(acm.heartbeat == Ice::ACMHeartbeat::HeartbeatOnIdle)

    proxy.ice_getCachedConnection().setACM(1, Ice::ACMClose::CloseOnInvocationAndIdle,
                                           Ice::ACMHeartbeat::HeartbeatAlways)
    acm = proxy.ice_getCachedConnection().getACM()
    test(acm.timeout == 1)
    test(acm.close == Ice::ACMClose::CloseOnInvocationAndIdle)
    test(acm.heartbeat == Ice::ACMHeartbeat::HeartbeatAlways)

    proxy.startHeartbeatCount()
    proxy.waitForHeartbeatCount(2)

    adapter.deactivate()
    testCommunicator.destroy()
    puts "ok"
end

def testHeartbeatManual(communicator, com)
    print "testing manual heartbeats... "
    STDOUT.flush

    adapter = com.createObjectAdapter(10, -1, 0)

    initData = Ice::InitializationData.new
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.ACM.Timeout", "10")
    initData.properties.setProperty("Ice.ACM.Client.Timeout", "10")
    initData.properties.setProperty("Ice.ACM.Client.Close", "0")
    initData.properties.setProperty("Ice.ACM.Client.Heartbeat", "0")
    testCommunicator = Ice::initialize(initData)
    proxy = Test::TestIntfPrx::uncheckedCast(testCommunicator.stringToProxy(adapter.getTestIntf().ice_toString()))
    con = proxy.ice_getConnection()

    proxy.startHeartbeatCount()
    con.heartbeat()
    con.heartbeat()
    con.heartbeat()
    con.heartbeat()
    con.heartbeat()
    proxy.waitForHeartbeatCount(5)

    adapter.deactivate()
    testCommunicator.destroy()
    puts "ok"
end

def allTests(helper, communicator)
    ref = "communicator:#{helper.getTestEndpoint()}"
    com = Test::RemoteCommunicatorPrx::uncheckedCast(communicator.stringToProxy(ref))

    testSetACM(communicator, com)
    testHeartbeatManual(communicator, com)

    com.shutdown()
end
