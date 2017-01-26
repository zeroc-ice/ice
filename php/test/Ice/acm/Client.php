<?php
// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

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

    echo "testing setACM/getACM... ";
    flush();

    $ref = "communicator:default -p 12010";
    $com = $communicator->stringToProxy($ref)->ice_uncheckedCast("::Test::RemoteCommunicator");

    $adapter = $com->createObjectAdapter(-1, -1, -1);

    $initData = $NS ? eval("return new Ice\\InitializationData;") : new Ice_InitializationData;
    $initData->properties = $communicator->getProperties()->clone();
    $initData->properties->setProperty("Ice.ACM.Timeout", "1");
    $initData->properties->setProperty("Ice.ACM.Client.Timeout", "15");
    $initData->properties->setProperty("Ice.ACM.Client.Close", "4");
    $initData->properties->setProperty("Ice.ACM.Client.Heartbeat", "2");
    
    $testCommunicator = $NS ? eval("return Ice\\initialize(\$initData);") : 
                              eval("return Ice_initialize(\$initData);");
    
    
    $proxy = $testCommunicator->stringToProxy($adapter->getTestIntf()->ice_toString())->ice_uncheckedCast(
        "::Test::TestIntf");
    $proxy->ice_getConnection();

    $CloseOnIdleForceful =
        $NS ? constant("Ice\\ACMClose::CloseOnIdleForceful") : constant("Ice_ACMClose::CloseOnIdleForceful");
    $CloseOnInvocationAndIdle =
        $NS ? constant("Ice\\ACMClose::CloseOnInvocationAndIdle") : constant("Ice_ACMClose::CloseOnInvocationAndIdle");
    $HeartbeatOnIdle =
        $NS ? constant("Ice\\ACMHeartbeat::HeartbeatOnIdle") : constant("Ice_ACMHeartbeat::HeartbeatOnIdle");
    $HeartbeatAlways =
        $NS ? constant("Ice\\ACMHeartbeat::HeartbeatAlways") : constant("Ice_ACMHeartbeat::HeartbeatAlways");

    $acm = $proxy->ice_getCachedConnection()->getACM();
    test($acm->timeout == 15);
    test($acm->close == $CloseOnIdleForceful);
    test($acm->heartbeat == $HeartbeatOnIdle);

    $proxy->ice_getCachedConnection()->setACM(Ice_Unset, Ice_Unset, Ice_Unset);
    $acm = $proxy->ice_getCachedConnection()->getACM();
    test($acm->timeout == 15);
    test($acm->close == $CloseOnIdleForceful);
    test($acm->heartbeat == $HeartbeatOnIdle);

    $proxy->ice_getCachedConnection()->setACM(1, $CloseOnInvocationAndIdle, $HeartbeatAlways);
    $acm = $proxy->ice_getCachedConnection()->getACM();
    test($acm->timeout == 1);
    test($acm->close == $CloseOnInvocationAndIdle);
    test($acm->heartbeat == $HeartbeatAlways);

    $proxy->waitForHeartbeat(2);

    $adapter->deactivate();
    $testCommunicator->destroy();
    echo "ok\n";

    echo "shutting down... ";
    flush();
    $com->shutdown();
    echo "ok\n";
}

$communicator = $NS ? eval("return Ice\\initialize(\$argv);") : 
                      eval("return Ice_initialize(\$argv);");

allTests($communicator);
$communicator->destroy();

exit();
?>
