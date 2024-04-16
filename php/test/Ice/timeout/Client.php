<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// We need to ensure that the memory limit is high enough
// to run this tests.
//
ini_set('memory_limit', '1024M');

require_once('Test.php');

function connect($prx)
{
    $nRetry = 10;
    while(--$nRetry > 0)
    {
        try
        {
            $prx->ice_getConnection(); // Establish connection.
            break;
        }
        catch(Exception $ex)
        {
            if(!($ex instanceof Ice\ConnectTimeoutException))
            {
                // Can sporadically occur with slow machines
                throw $ex;
            }
        }
    }
    return $prx->ice_getConnection(); // Establish connection.
}

function allTests($helper)
{
    $controller = $helper->communicator()->stringToProxy(sprintf("controller:%s", $helper->getTestEndpoint(1)));
    $controller = $controller->ice_checkedCast("::Test::Controller");
    test($controller);
    try
    {
        allTestsWithController($helper, $controller);
    }
    catch(Exception $ex)
    {
        // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent the test
        // from hanging on exit in case a connection which disables timeouts is still opened.
        $controller->resumeAdapter();
        throw $ex;
    }
}

function allTestsWithController($helper, $controller)
{
    $communicator = $helper->communicator();
    $sref = sprintf("timeout:%s", $helper->getTestEndpoint());
    $timeout = $communicator->stringToProxy($sref);
    test($timeout);

    $timeout = $timeout->ice_checkedCast("::Test::Timeout");
    test($timeout);

    // The sequence needs to be large enough to fill the write/recv buffers
    $seq = array_fill(0, 1000000, 0x01);
    echo("testing connection timeout... ");
    flush();
    {
        //
        // Expect TimeoutException.
        //
        $to = $timeout->ice_timeout(250)->ice_uncheckedCast("::Test::Timeout");
        connect($to);
        $controller->holdAdapter(-1);
        try
        {
            $to->sendData($seq);
            test(false);
        }
        catch(Ice\TimeoutException $ex)
        {
            // Expected.
        }
        $controller->resumeAdapter();
        $timeout->op(); // Ensure adapter is active.
    }
    {
        //
        // Expect success.
        //
        $to = $timeout->ice_timeout(2000)->ice_uncheckedCast("::Test::Timeout");
        $controller->holdAdapter(100);
        try
        {
            $data = array_fill(0, 1000000, 0x01);
            $to->sendData($data);
        }
        catch(Exception $ex)
        {
            test(false);
        }
    }
    echo("ok\n");

    echo("testing invocation timeout... ");
    flush();
    {
        $connection = $timeout->ice_getConnection();
        $to = $timeout->ice_invocationTimeout(100)->ice_uncheckedCast("::Test::Timeout");
        test($connection == $to->ice_getConnection());
        try
        {
            $to->sleep(500);
            test(false);
        }
        catch(Ice\InvocationTimeoutException $ex)
        {
        }

        $timeout->ice_ping();
        $to = $timeout->ice_invocationTimeout(1000)->ice_checkedCast("::Test::Timeout");
        test($connection == $to->ice_getConnection());
        $to->sleep(100);
        test($connection == $to->ice_getConnection());
    }
    {
        //
        // Backward compatible connection timeouts
        //
        $to = $timeout->ice_invocationTimeout(-2)->ice_timeout(250)->ice_uncheckedCast("::Test::Timeout");
        $con = connect($to);
        try
        {
            $to->sleep(750);
            test(false);
        }
        catch(Ice\TimeoutException $ex)
        {
        }
        $timeout->ice_ping();
    }
    echo("ok\n");

    $controller->shutdown();
    $communicator->destroy();
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $properties = $this->createTestProperties($args);
            //
            // For this test, we want to disable retries.
            //
            $properties->setProperty("Ice.RetryIntervals", "-1");

            //
            // This test kills connections, so we don't want warnings.
            //
            $properties->setProperty("Ice.Warn.Connections", "0");

            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            $properties->setProperty("Ice.TCP.SndSize", "50000");

            $communicator = $this->initialize($properties);
            allTests($this);
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
        }
    }
}
?>
