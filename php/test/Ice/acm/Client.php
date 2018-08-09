<?php
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

require_once("Test.php");

function testSetACM($helper, $com)
{
    global $NS;
    $CloseOnIdleForceful = constant($NS ? "Ice\\ACMClose::CloseOnIdleForceful" :
                                          "Ice_ACMClose::CloseOnIdleForceful");

    $CloseOnInvocationAndIdle = constant($NS ? "Ice\\ACMClose::CloseOnInvocationAndIdle" :
                                               "Ice_ACMClose::CloseOnInvocationAndIdle");

    $HeartbeatOnIdle = constant($NS ? "Ice\\ACMHeartbeat::HeartbeatOnIdle" :
                                      "Ice_ACMHeartbeat::HeartbeatOnIdle");

    $HeartbeatAlways = constant($NS ? "Ice\\ACMHeartbeat::HeartbeatAlways" :
                                      "Ice_ACMHeartbeat::HeartbeatAlways");

    $None = constant($NS ? "Ice\\None" : "Ice_Unset");

    echo "testing setACM/getACM... ";
    flush();

    $adapter = $com->createObjectAdapter(-1, -1, -1);

    $communicator = $helper->communicator();
    $properties = $communicator->getProperties()->clone();
    $properties->setProperty("Ice.ACM.Timeout", "1");
    $properties->setProperty("Ice.ACM.Client.Timeout", "15");
    $properties->setProperty("Ice.ACM.Client.Close", "4");
    $properties->setProperty("Ice.ACM.Client.Heartbeat", "2");

    $communicator = $helper->initialize($properties);

    $proxy = $communicator->stringToProxy(
        $adapter->getTestIntf()->ice_toString())->ice_uncheckedCast("::Test::TestIntf");
    $proxy->ice_getConnection();

    $acm = $proxy->ice_getCachedConnection()->getACM();
    test($acm->timeout == 15);
    test($acm->close == $CloseOnIdleForceful);
    test($acm->heartbeat == $HeartbeatOnIdle);

    $proxy->ice_getCachedConnection()->setACM($None, $None, $None);
    $acm = $proxy->ice_getCachedConnection()->getACM();
    test($acm->timeout == 15);
    test($acm->close == $CloseOnIdleForceful);
    test($acm->heartbeat == $HeartbeatOnIdle);

    $proxy->ice_getCachedConnection()->setACM(1, $CloseOnInvocationAndIdle, $HeartbeatAlways);
    $acm = $proxy->ice_getCachedConnection()->getACM();
    test($acm->timeout == 1);
    test($acm->close == $CloseOnInvocationAndIdle);
    test($acm->heartbeat == $HeartbeatAlways);

    $proxy->startHeartbeatCount();
    $proxy->waitForHeartbeatCount(2);

    $adapter->deactivate();
    $communicator->destroy();
    echo "ok\n";
}

function testHeartbeatManual($helper, $com)
{
    echo "testing manual heartbeats... ";
    flush();

    $adapter = $com->createObjectAdapter(10, -1, 0);

    $communicator = $helper->communicator();
    $properties = $communicator->getProperties()->clone();
    $properties->setProperty("Ice.ACM.Timeout", "10");
    $properties->setProperty("Ice.ACM.Client.Timeout", "10");
    $properties->setProperty("Ice.ACM.Client.Close", "0");
    $properties->setProperty("Ice.ACM.Client.Heartbeat", "0");
    $communicator = $helper->initialize($properties);
    $proxy = $communicator->stringToProxy(
        $adapter->getTestIntf()->ice_toString())->ice_uncheckedCast("::Test::TestIntf");
    $con = $proxy->ice_getConnection();

    $proxy->startHeartbeatCount();
    $con->heartbeat();
    $con->heartbeat();
    $con->heartbeat();
    $con->heartbeat();
    $con->heartbeat();
    $proxy->waitForHeartbeatCount(5);

    $adapter->deactivate();
    $communicator->destroy();
    echo "ok\n";
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            $ref = sprintf("communicator:%s", $this->getTestEndpoint());
            $com = $communicator->stringToProxy($ref)->ice_uncheckedCast("::Test::RemoteCommunicator");

            testSetACM($this, $com);
            testHeartbeatManual($this, $com);

            $com->shutdown();
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
