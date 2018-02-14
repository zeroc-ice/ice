# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def allTests(communicator)
    print "testing setACM/getACM... "
    STDOUT.flush

    ref = "communicator:default -p 12010"
    com = Test::RemoteCommunicatorPrx::uncheckedCast(communicator.stringToProxy(ref))

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

    proxy.waitForHeartbeat(2)

    adapter.deactivate()
    testCommunicator.destroy()
    puts "ok"

    print "shutting down... "
    STDOUT.flush
    com.shutdown()
    puts "ok"
end
