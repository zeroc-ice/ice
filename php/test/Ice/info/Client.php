<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

require_once('Test.php');

function getTCPEndpointInfo($i)
{
    while ($i) {
        if ($i instanceof Ice\TCPEndpointInfo) {
            return $i;
        }
        $i = $i->underlying;
    }
}

function getTCPConnectionInfo($i)
{
    while ($i) {
        if ($i instanceof Ice\TCPConnectionInfo) {
            return $i;
        }
        $i = $i->underlying;
    }
}

function allTests($helper)
{
    $communicator = $helper->communicator();

    echo "testing proxy endpoint information... ";
    flush(); {
        $p1 = $communicator->stringToProxy(
            "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" .
                "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" .
                "opaque -e 1.8 -t 100 -v ABCD"
        );

        $endps = $p1->ice_getEndpoints();
        $port = $helper->getTestPort();
        $endpoint = $endps[0]->getInfo();
        $tcpEndpoint = getTCPEndpointInfo($endpoint);
        test($tcpEndpoint instanceof Ice\TCPEndpointInfo);
        test($tcpEndpoint->host == "tcphost");
        test($tcpEndpoint->port == 10000);
        test($tcpEndpoint->timeout == 1200);
        test($tcpEndpoint->sourceAddress == "10.10.10.10");
        test($tcpEndpoint->compress);
        test(!$tcpEndpoint->datagram());
        test(($tcpEndpoint->type() == Ice\TCPEndpointType && !$tcpEndpoint->secure()) ||
            ($tcpEndpoint->type() == Ice\SSLEndpointType && $tcpEndpoint->secure()) ||
            ($tcpEndpoint->type() == Ice\WSEndpointType && !$tcpEndpoint->secure()) ||
            ($tcpEndpoint->type() == Ice\WSSEndpointType && $tcpEndpoint->secure()));
        test(($tcpEndpoint->type() == Ice\TCPEndpointType && ($endpoint instanceof Ice\TCPEndpointInfo)) ||
            ($tcpEndpoint->type() == Ice\SSLEndpointType && ($endpoint instanceof Ice\SSLEndpointInfo)) ||
            ($tcpEndpoint->type() == Ice\WSEndpointType && ($endpoint instanceof Ice\WSEndpointInfo)) ||
            ($tcpEndpoint->type() == Ice\WSSEndpointType && ($endpoint instanceof Ice\WSEndpointInfo)));

        $udpEndpoint = $endps[1]->getInfo();
        test($udpEndpoint instanceof Ice\UDPEndpointInfo);
        test($udpEndpoint->host == "udphost");
        test($udpEndpoint->port == 10001);
        test($udpEndpoint->sourceAddress == "10.10.10.10");
        test($udpEndpoint->mcastInterface == "eth0");
        test($udpEndpoint->mcastTtl == 5);
        test($udpEndpoint->timeout == -1);
        test(!$udpEndpoint->compress);
        test(!$udpEndpoint->secure());
        test($udpEndpoint->datagram());
        test($udpEndpoint->type() == Ice\UDPEndpointType);

        $opaqueEndpoint = $endps[2]->getInfo();
        test($opaqueEndpoint);
    }
    echo "ok\n";

    $defaultHost = $communicator->getProperties()->getIceProperty("Ice.Default.Host");
    $testIntf = Test\TestIntfPrxHelper::createProxy(
        $communicator,
        sprintf("test:%s:%s", $helper->getTestEndpoint(), $helper->getTestEndpoint("udp"))
    );
    $testPort = $helper->getTestPort();
    echo "test connection endpoint information... ";
    flush(); {
        $tcpinfo = getTCPEndpointInfo($testIntf->ice_getConnection()->getEndpoint()->getInfo());
        test($tcpinfo instanceof Ice\TCPEndpointInfo);
        test($tcpinfo->port == $testPort);
        test(!$tcpinfo->compress);
        test($tcpinfo->host == $defaultHost);

        $ctx = $testIntf->getEndpointInfoAsContext();
        test($ctx["host"] == $tcpinfo->host);
        test($ctx["compress"] == "false");
        test($ctx["port"] > 0);

        $udpinfo = $testIntf->ice_datagram()->ice_getConnection()->getEndpoint()->getInfo();
        test($udpinfo instanceof Ice\UDPEndpointInfo);
        test($udpinfo->port == $testPort);
        test($udpinfo->host == $defaultHost);
    }
    echo "ok\n";

    echo "testing connection information... ";
    flush(); {
        $port = $helper->getTestPort();

        $connection = $testIntf->ice_getConnection();
        $connection->setBufferSize(1024, 2048);

        $info = $connection->getInfo();
        $tcpinfo = getTCPConnectionInfo($info);
        test($tcpinfo instanceof Ice\TCPConnectionInfo);
        test(!$info->incoming);
        test(strlen($info->adapterName) == 0);
        test($tcpinfo->remotePort == $port);
        if ($defaultHost == "127.0.0.1") {
            test($tcpinfo->remoteAddress == $defaultHost);
            test($tcpinfo->localAddress == $defaultHost);
        }
        test($tcpinfo->rcvSize >= 1024);
        test($tcpinfo->sndSize >= 2048);

        $ctx = $testIntf->getConnectionInfoAsContext();
        test($ctx["incoming"] == "true");
        test($ctx["adapterName"] == "TestAdapter");
        test($ctx["remoteAddress"] == $tcpinfo->localAddress);
        test($ctx["localAddress"] == $tcpinfo->remoteAddress);
        test($ctx["remotePort"] == $tcpinfo->localPort);
        test($ctx["localPort"] == $tcpinfo->remotePort);

        if ($testIntf->ice_getConnection()->type() == "ws" || $testIntf->ice_getConnection()->type() == "wss") {
            test($info instanceof Ice\WSConnectionInfo);

            test($info->headers["Upgrade"] == "websocket");
            test($info->headers["Connection"] == "Upgrade");
            test($info->headers["Sec-WebSocket-Protocol"] == "ice.zeroc.com");
            test(isset($info->headers["Sec-WebSocket-Accept"]));

            test($ctx["ws.Upgrade"] == "websocket");
            test($ctx["ws.Connection"] == "Upgrade");
            test($ctx["ws.Sec-WebSocket-Protocol"] == "ice.zeroc.com");
            test($ctx["ws.Sec-WebSocket-Version"] == "13");
            test(isset($ctx["ws.Sec-WebSocket-Key"]));
        }
    }
    echo "ok\n";

    return $testIntf;
}

class Client extends TestHelper
{
    function run($args)
    {
        try {
            $communicator = $this->initialize($args);
            $proxy = allTests($this);
            $proxy->shutdown();
            $communicator->destroy();
        } catch (Exception $ex) {
            $communicator->destroy();
            throw $ex;
        }
    }
}
