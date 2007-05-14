#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def allTests(communicator)
    sref = "timeout:default -p 12010 -t 10000"
    obj = communicator.stringToProxy(sref)
    test(obj)

    timeout = Test::TimeoutPrx::checkedCast(obj)
    test(timeout)

    print "testing connect timeout... "
    STDOUT.flush
    #
    # Expect ConnectTimeoutException.
    #
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(500))
    to.holdAdapter(750)
    to.ice_getConnection().close(true) # Force a reconnect.
    begin
        to.op()
        test(false)
    rescue Ice::ConnectTimeoutException
        # Expected.
    end

    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(1000))
    to.holdAdapter(500)
    to.ice_getConnection().close(true) # Force a reconnect.
    begin
        to.op()
    rescue Ice::ConnectTimeoutException
        test(false)
    end
    puts "ok"

    print "testing read timeout... "
    STDOUT.flush
    #
    # Expect TimeoutException.
    #
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(500))
    begin
        to.sleep(750)
        test(false)
    rescue Ice::TimeoutException
        # Expected.
    end
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(1000))
    begin
        to.sleep(500)
    rescue Ice::TimeoutException
        test(false)
    end
    puts "ok"

    print "testing write timeout... "
    STDOUT.flush
    #
    # Expect TimeoutException.
    #
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(500))
    to.holdAdapter(750)
    seq = "\0" * 100000 # 100,000 entries
    begin
        to.sendData(seq)
        test(false)
    rescue Ice::TimeoutException
      # Expected.
    end
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(1000))
    to.holdAdapter(500)
    begin
        to.sendData(seq)
    rescue Ice::TimeoutException
        test(false)
    end
    puts "ok"

    print "testing timeout overrides... "
    STDOUT.flush
    #
    # Test Ice.Override.Timeout. This property overrides all
    # endpoint timeouts.
    #
    initData = Ice::InitializationData.new
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.Timeout", "500")
    comm = Ice.initialize(initData)
    to = Test::TimeoutPrx::checkedCast(comm.stringToProxy(sref))
    begin
        to.sleep(750)
        test(false)
    rescue Ice::TimeoutException
        # expected
    end
    #
    # Calling ice_timeout() should have no effect.
    #
    timeout.op() # Ensure adapter is active.
    to = Test::TimeoutPrx::checkedCast(to.ice_timeout(1000))
    begin
        to.sleep(750)
        test(false)
    rescue Ice::TimeoutException
        # Expected.
    end
    comm.destroy()

    #
    # Test Ice.Override.ConnectTimeout.
    #
    initData = Ice::InitializationData.new
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "750")
    comm = Ice.initialize(initData)
    timeout.holdAdapter(1000)
    to = Test::TimeoutPrx::uncheckedCast(comm.stringToProxy(sref))
    begin
        to.op()
        test(false)
    rescue Ice::ConnectTimeoutException
        # expected
    end

    #
    # Calling ice_timeout() should have no effect on the connect timeout.
    #
    timeout.op() # Ensure adapter is active.
    timeout.holdAdapter(1000)
    to = Test::TimeoutPrx::uncheckedCast(to.ice_timeout(1250))
    begin
        to.op()
        test(false)
    rescue Ice::ConnectTimeoutException
        # expected
    end

    #
    # Verify that timeout set via ice_timeout() is still used for requests.
    #
    to.op() # Force connection.
    begin
        to.sleep(1500)
        test(false)
    rescue Ice::TimeoutException:
        # Expected.
    end
    comm.destroy()
    puts "ok"

    return timeout
end
