# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def connect(prx)
    nRetry = 10
    while nRetry > 0 do
        nRetry -=1
        begin
            prx.ice_getConnection() # Establish connection.
            break
        rescue Ice::ConnectTimeoutException
            # Can sporadically occur with slow machines
        end
    end
    return prx.ice_getConnection()
end

def allTests(communicator)
    sref = "timeout:default -p 12010"
    obj = communicator.stringToProxy(sref)
    test(obj)

    timeout = Test::TimeoutPrx::checkedCast(obj)
    test(timeout)

    controller = Test::ControllerPrx::checkedCast(communicator.stringToProxy("controller:default -p 12011"))
    test(controller)

    print "testing connect timeout... "
    STDOUT.flush
    #
    # Expect ConnectTimeoutException.
    #
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(100))
    controller.holdAdapter(-1)
    begin
        to.op()
        test(false)
    rescue Ice::ConnectTimeoutException
        # Expected.
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.

    #
    # Expect success.
    #
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(-1))
    controller.holdAdapter(100)
    begin
        to.op()
    rescue Ice::ConnectTimeoutException
        test(false)
    end
    puts "ok"

    print "testing connection timeout... "
    STDOUT.flush
    #
    # Expect TimeoutException.
    #
    seq = "\0" * 10000000 # 10,000,000 entries
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(250))
    connect(to)
    controller.holdAdapter(-1)
    begin
        to.sendData(seq)
        test(false)
    rescue Ice::TimeoutException
      # Expected.
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    #
    # Expect success.
    #
    to = Test::TimeoutPrx::uncheckedCast(obj.ice_timeout(2000))
    controller.holdAdapter(100)
    begin
        seq2 = "\0" * 1000000 # 1,000,000 entries
        to.sendData(seq2)
    rescue Ice::TimeoutException
        test(false)
    end
    puts "ok"

    print "testing invocation timeout... "
    STDOUT.flush
    connection = obj.ice_getConnection()
    to = Test::TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(100))
    test(connection == to.ice_getConnection())
    begin
        to.sleep(500)
        test(false)
    rescue Ice::InvocationTimeoutException
        # Expected
    end
    obj.ice_ping();

    to = Test::TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(1000))
    test(connection == to.ice_getConnection())
    begin
        to.sleep(100)
    rescue Ice::InvocationTimeoutException
        test(false)
    end
    test(connection == to.ice_getConnection())
    puts "ok"

    print "testing close timeout... "
    STDOUT.flush
    to = Test::TimeoutPrx.uncheckedCast(obj.ice_timeout(250))
    connection = connect(to);
    controller.holdAdapter(-1)
    connection.close(Ice::ConnectionClose::GracefullyWithWait)
    begin
        connection.getInfo() # getInfo() doesn't throw in the closing state.
    rescue Ice::LocalException
        test(false)
    end
    while(true)
        begin
            connection.getInfo()
            sleep(0.01)
        rescue Ice::ConnectionManuallyClosedException => ex
            # Expected.
            test(ex.graceful)
            break
        end
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    puts "ok"

    print "testing timeout overrides... "
    STDOUT.flush
    #
    # Test Ice.Override.Timeout. This property overrides all
    # endpoint timeouts.
    #
    initData = Ice::InitializationData.new
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250")
    initData.properties.setProperty("Ice.Override.Timeout", "100")
    comm = Ice.initialize(initData)
    to = Test::TimeoutPrx::uncheckedCast(comm.stringToProxy(sref))
    connect(to)
    controller.holdAdapter(-1)
    begin
        to.sendData(seq)
        test(false)
    rescue Ice::TimeoutException
        # expected
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    #
    # Calling ice_timeout() should have no effect.
    #
    to = Test::TimeoutPrx::uncheckedCast(to.ice_timeout(1000))
    connect(to)
    controller.holdAdapter(-1)
    begin
        to.sendData(seq)
        test(false)
    rescue Ice::TimeoutException
        # Expected.
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    comm.destroy()

    #
    # Test Ice.Override.ConnectTimeout.
    #
    initData = Ice::InitializationData.new
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250")
    comm = Ice.initialize(initData)
    controller.holdAdapter(-1)
    to = Test::TimeoutPrx::uncheckedCast(comm.stringToProxy(sref))
    begin
        to.op()
        test(false)
    rescue Ice::ConnectTimeoutException
        # expected
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.

    #
    # Calling ice_timeout() should have no effect on the connect timeout.
    #
    controller.holdAdapter(-1)
    to = Test::TimeoutPrx::uncheckedCast(to.ice_timeout(1000))
    begin
        to.op()
        test(false)
    rescue Ice::ConnectTimeoutException
        # expected
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.

    #
    # Verify that timeout set via ice_timeout() is still used for requests.
    #
    to = Test::TimeoutPrx::uncheckedCast(to.ice_timeout(250))
    connect(to)
    controller.holdAdapter(-1)
    begin
        to.sendData(seq)
        test(false)
    rescue Ice::TimeoutException
        # Expected.
    end
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    comm.destroy()

    #
    # Test Ice.Override.CloseTimeout.
    #
    initData = Ice::InitializationData.new
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.CloseTimeout", "100")
    comm = Ice.initialize(initData)
    connection = comm.stringToProxy(sref).ice_getConnection();
    controller.holdAdapter(-1);
    now = Time.now
    comm.destroy();
    test((Time.now - now) < 1.0);
    controller.resumeAdapter()

    puts "ok"

    controller.shutdown()
end
