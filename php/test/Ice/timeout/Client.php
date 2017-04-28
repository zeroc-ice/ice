<?php
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

//
// We need to ensure that the memory limit is high enough
// to run this tests.
//
ini_set('memory_limit', '1024M');

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}

$NS = function_exists("Ice\\initialize");
require_once('Ice.php');
require_once('Test.php');

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

function allTests($communicator)
{
    global $NS;
    $ConnectTimeoutException = $NS ? "Ice\\ConnectTimeoutException" : "Ice_ConnectTimeoutException";
    $TimeoutException = $NS ? "Ice\\TimeoutException" : "Ice_TimeoutException";
    $InvocationTimeoutException = $NS ? "Ice\\InvocationTimeoutException" : "Ice_InvocationTimeoutException";
    $CloseGracefullyAndWait = constant($NS ? "Ice\\ConnectionClose::GracefullyWithWait" :
                                             "Ice_ConnectionClose::GracefullyWithWait");
    $InitializationData = $NS ? "Ice\\InitializationData" : "Ice_InitializationData";

    $sref = "timeout:default -p 12010";
    $timeout = $communicator->stringToProxy($sref);
    test($timeout);

    $timeout = $timeout->ice_checkedCast("::Test::Timeout");
    test($timeout);

    echo("testing connect timeout... ");
    flush();

    {
        //
        // Expect ConnectTimeoutException.
        //
        $to = $timeout->ice_timeout(100);
        $timeout->holdAdapter(500);
        try
        {
            $to->op();
            test(false);
        }
        catch(Exception $ex)
        {
            if(!($ex instanceof $ConnectTimeoutException))
            {
                echo($ex);
                test(false);
            }
        }
    }
    {
        //
        // Expect success.
        //
        $timeout->op(); // Ensure adapter is active.
        $to =$timeout->ice_timeout(1000);
        $timeout->holdAdapter(500);
        try
        {
            $to->op();
        }
        catch(Exception $ex)
        {
            test(false);
        }
    }
    echo("ok\n");

    // The sequence needs to be large enough to fill the write/recv buffers
    $seq = array_fill(0, 2000000, 0x01);
    echo("testing connection timeout... ");
    flush();
    {
        //
        // Expect TimeoutException.
        //
        $to = $timeout->ice_timeout(100)->ice_uncheckedCast("::Test::Timeout");
        $timeout->holdAdapter(500);
        try
        {
            $to->sendData($seq);
            test(false);
        }
        catch(Exception $ex)
        {
            // Expected.
            if(!($ex instanceof $TimeoutException))
            {
                echo($ex);
                test(false);
            }
        }
    }
    {
        //
        // Expect success.
        //
        $timeout->op(); // Ensure adapter is active.
        $to = $timeout->ice_timeout(1000)->ice_uncheckedCast("::Test::Timeout");
        $timeout->holdAdapter(500);
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
            $to->sleep(750);
            test(false);
        }
        catch(Exception $ex)
        {
            if(!($ex instanceof $InvocationTimeoutException))
            {
                echo($ex);
                test(false);
            }
        }

        $timeout->ice_ping();
        $to = $timeout->ice_invocationTimeout(500)->ice_checkedCast("::Test::Timeout");
        test($connection == $to->ice_getConnection());
        try
        {
            $to->sleep(100);
        }
        catch(Exception $ex)
        {
            test(false);
        }
        test($connection == $to->ice_getConnection());
    }
    {
        //
        // Backward compatible connection timeouts
        //
        $to = $timeout->ice_invocationTimeout(-2)->ice_timeout(250)->ice_uncheckedCast("::Test::Timeout");
        $con = $to->ice_getConnection();
        try
        {
            $to->sleep(750);
            test(false);
        }
        catch(Exception $ex)
        {
            try
            {
                $con->getInfo();
                test(false);
            }
            catch(Exception $ex)
            {
                // Connection got closed as well.
                if(!($ex instanceof $TimeoutException))
                {
                    echo($ex);
                    test(false);
                }
            }
        }
        $timeout->ice_ping();
    }
    echo("ok\n");

    echo("testing close timeout... ");
    flush();
    {
        $to = $timeout->ice_timeout(250)->ice_uncheckedCast("::Test::Timeout");
        $connection = $to->ice_getConnection();
        $timeout->holdAdapter(600);
        $connection->close($CloseGracefullyAndWait);
        try
        {
            $connection->getInfo(); // getInfo() doesn't throw in the closing state.
        }
        catch(Exception $ex)
        {
            test(false);
        }
        usleep(650 * 1000);
        try
        {
            $connection->getInfo();
            test(false);
        }
        catch(Exception $ex)
        {
            // Expected.
            test($ex->graceful);
        }
        $timeout->op(); // Ensure adapter is active.
    }
    echo("ok\n");

    echo("testing timeout overrides... ");
    flush();
    {
        //
        // Test Ice.Override.Timeout. This property overrides all
        // endpoint timeouts.
        //
        $initData = eval($NS ? "return new Ice\\InitializationData();" : "return new Ice_InitializationData();");
        $initData->properties = $communicator->getProperties()->clone();
        $initData->properties->setProperty("Ice.Override.Timeout", "150");
        $comm = eval($NS ? "return Ice\\initialize(\$initData);" : "return Ice_initialize(\$initData);");
        $to = $comm->stringToProxy($sref)->ice_checkedCast("::Test::Timeout");
        $timeout->holdAdapter(800);
        try
        {
            $to->sendData($seq);
            test(false);
        }
        catch(Exception $ex)
        {
            // Expected.
            if(!($ex instanceof $TimeoutException))
            {
                echo($ex);
                test(false);
            }
        }
        //
        // Calling ice_timeout() should have no effect.
        //
        $timeout->op(); // Ensure adapter is active.
        $to = $to->ice_timeout(1000)->ice_checkedCast("::Test::Timeout");
        $timeout->holdAdapter(800);
        try
        {
            $to->sendData($seq);
            test(false);
        }
        catch(Exception $ex)
        {
            // Expected TimeoutException.
            if(!($ex instanceof $TimeoutException))
            {
                echo($ex);
                test(false);
            }
        }
        $comm->destroy();
    }
    {
        //
        // Test Ice.Override.ConnectTimeout.
        //
        $initData = eval($NS ? "return new Ice\\InitializationData();" : "return new Ice_InitializationData();");
        $initData->properties = $communicator->getProperties()->clone();
        $initData->properties->setProperty("Ice.Override.ConnectTimeout", "250");
        $comm = eval($NS ? "return Ice\\initialize(\$initData);" : "return Ice_initialize(\$initData);");
        $to = $comm->stringToProxy($sref)->ice_uncheckedCast("::Test::Timeout");
        $timeout->holdAdapter(750);
        try
        {
            $to->op();
            test(false);
        }
        catch(Exception $ex)
        {
            // Expected.
            if(!($ex instanceof $ConnectTimeoutException))
            {
                echo($ex);
                test(false);
            }
        }
        //
        // Calling ice_timeout() should have no effect on the connect timeout.
        //
        $timeout->op(); // Ensure adapter is active.
        $timeout->holdAdapter(750);
        $to = $to->ice_timeout(1000)->ice_uncheckedCast("::Test::Timeout");
        try
        {
            $to->op();
            test(false);
        }
        catch(Exception $ex)
        {
            // Expected.
            if(!($ex instanceof $ConnectTimeoutException))
            {
                echo($ex);
                test(false);
            }

        }
        //
        // Verify that timeout set via ice_timeout() is still used for requests.
        //
        $timeout->op(); // Ensure adapter is active.
        $to = $to->ice_timeout(250)->ice_uncheckedCast("::Test::Timeout");
        $to->ice_getConnection(); // Establish connection.
        $timeout->holdAdapter(750);
        try
        {
            $to->sendData($seq);
            test(false);
        }
        catch(Exception $ex)
        {
            // Expected.
            if(!($ex instanceof $TimeoutException))
            {
                echo($ex);
                test(false);
            }
        }
        $comm->destroy();
    }
    {
        //
        // Test Ice.Override.CloseTimeout.
        //
        $initData = eval($NS ? "return new Ice\\InitializationData();" : "return new Ice_InitializationData();");
        $initData->properties = $communicator->getProperties()->clone();
        $initData->properties->setProperty("Ice.Override.CloseTimeout", "100");
        $comm = eval($NS ? "return Ice\\initialize(\$initData);" : "return Ice_initialize(\$initData);");
        $to = $comm->stringToProxy($sref)->ice_uncheckedCast("::Test::Timeout");
        $timeout->holdAdapter(800);

        $begin = microtime(true);
        $comm->destroy();
        test(microtime(true) - $begin < 0.7);
    }
    echo("ok\n");

    $timeout->shutdown();
    $communicator->destroy();
}


$initData = eval($NS ? "return new Ice\\InitializationData();" : "return new Ice_InitializationData();");

$initData->properties = eval($NS ? "return Ice\\createProperties(\$argv);" : "return Ice_createProperties(\$argv);");
//
// We need to send messages large enough to cause the transport
// buffers to fill up.
//
$initData->properties->setProperty("Ice.MessageSizeMax", "20000");

//
// For this test, we want to disable retries.
//
$initData->properties->setProperty("Ice.RetryIntervals", "-1");

//
// This test kills connections, so we don't want warnings.
//
$initData->properties->setProperty("Ice.Warn.Connections", "0");

//
// Limit the send buffer size, this test relies on the socket
// send() blocking after sending a given amount of data.
//
$initData->properties->setProperty("Ice.TCP.SndSize", "50000");

$communicator = eval($NS ? "return Ice\\initialize(\$initData);" :
                           "return Ice_initialize(\$initData);");

allTests($communicator);

exit();
?>
