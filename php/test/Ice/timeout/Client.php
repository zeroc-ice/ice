<?php
// Copyright (c) ZeroC, Inc.

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
    $communicator = $helper->communicator();
    $controller = Test\ControllerPrxHelper::createProxy(
        $communicator,
        sprintf("controller:%s", $helper->getTestEndpoint(1)));

    connect($controller);

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
    $timeout = Test\TimeoutPrxHelper::createProxy($communicator, $sref);

    echo("testing invocation timeout... ");
    flush();
    {
        $connection = $timeout->ice_getConnection();
        $to = $timeout->ice_invocationTimeout(100);
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
        $to = $timeout->ice_invocationTimeout(1000);
        test($connection == $to->ice_getConnection());
        $to->sleep(100);
        test($connection == $to->ice_getConnection());
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
