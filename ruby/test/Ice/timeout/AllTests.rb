# Copyright (c) ZeroC, Inc.

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

def allTests(helper, communicator)
    controller = Test::ControllerPrx.new(communicator, "controller:#{helper.getTestEndpoint(num:1)}")
    connect(controller)
    begin
        allTestsWithController(helper, communicator, controller)
    rescue
        # Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
        # the test from hanging on exit in case a connection which disables timeouts is still opened.
        controller.resumeAdapter()
        raise
    end
end

def allTestsWithController(helper, communicator, controller)
    sref = "timeout:#{helper.getTestEndpoint()}"
    timeout = Test::TimeoutPrx.new(communicator, sref)

    print "testing invocation timeout... "
    STDOUT.flush
    connection = timeout.ice_getConnection()
    to = timeout.ice_invocationTimeout(100)
    test(connection == to.ice_getConnection())
    begin
        to.sleep(1000)
        test(false)
    rescue Ice::InvocationTimeoutException
        # Expected
    end
    timeout.ice_ping();

    to = timeout.ice_invocationTimeout(1000)
    test(connection == to.ice_getConnection())
    begin
        to.sleep(100)
    rescue Ice::InvocationTimeoutException
        test(false)
    end
    test(connection == to.ice_getConnection())
    puts "ok"

    controller.shutdown()
end
