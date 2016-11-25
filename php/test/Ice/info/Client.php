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
require_once ($NS ? 'Ice_ns.php' : 'Ice.php');
require_once 'Test.php';

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

function getTCPEndpointInfo($i)
{
    global $NS;
    $tcpEndpointInfoClass = $NS ? "Ice\\TCPEndpointInfo" : "Ice_TCPEndpointInfo";
    while($i)
    {
        if($i instanceof $tcpEndpointInfoClass)
        {
            return $i;
        }
        $i = $i->underlying;
    }
}

function getTCPConnectionInfo($i)
{
    global $NS;
    $ipConnectionInfoClass = $NS ? "Ice\\TCPConnectionInfo" : "Ice_TCPConnectionInfo";
    while($i)
    {
        if($i instanceof $ipConnectionInfoClass)
        {
            return $i;
        }
        $i = $i->underlying;
    }
}

function allTests($communicator)
{
    global $NS;

    $tcpEndpointType = $NS ? constant("Ice\\TCPEndpointType") : constant("Ice_TCPEndpointType");
    $tcpEndpointInfoClass = $NS ? "Ice\\TCPEndpointInfo" : "Ice_TCPEndpointInfo";
    $udpEndpointType = $NS ? constant("Ice\\UDPEndpointType") : constant("Ice_UDPEndpointType");
    $udpEndpointInfoClass = $NS ? "Ice\\UDPEndpointInfo" : "Ice_UDPEndpointInfo";
    $sslEndpointType = $NS ? constant("Ice\\SSLEndpointType") : constant("Ice_SSLEndpointType");
    $sslEndpointInfoClass = $NS ? "Ice\\SSLEndpointInfo" : "Ice_SSLEndpointInfo";
    $wsEndpointType = $NS ? constant("Ice\\WSEndpointType") : constant("Ice_WSEndpointType");
    $wsEndpointInfoClass = $NS ? "Ice\\WSEndpointInfo" : "Ice_WSEndpointInfo";
    $wssEndpointType = $NS ? constant("Ice\\WSSEndpointType") : constant("Ice_WSSEndpointType");
    $wssEndpointInfoClass = $NS ? "Ice\\WSSEndpointInfo" : "Ice_WSSEndpointInfo";
    $protocolVersionClass = $NS ? "Ice\\ProtocolVersion" : "Ice_ProtocolVersion";
    $encodingVersionClass = $NS ? "Ice\\EncodingVersion" : "Ice_EncodingVersion";

    echo "testing proxy endpoint information... ";
    flush();
    {
        $p1 = $communicator->stringToProxy(
                "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" .
                "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" .
                "opaque -e 1.8 -t 100 -v ABCD");

        $endps = $p1->ice_getEndpoints();

        $endpoint = $endps[0]->getInfo();
        $tcpEndpoint = getTCPEndpointInfo($endpoint);
        test($tcpEndpoint instanceof $tcpEndpointInfoClass);
        test($tcpEndpoint->host == "tcphost");
        test($tcpEndpoint->port == 10000);
        test($tcpEndpoint->timeout == 1200);
        test($tcpEndpoint->sourceAddress == "10.10.10.10");
        test($tcpEndpoint->compress);
        test(!$tcpEndpoint->datagram());
        test(($tcpEndpoint->type() == $tcpEndpointType && !$tcpEndpoint->secure()) ||
             ($tcpEndpoint->type() == $sslEndpointType && $tcpEndpoint->secure()) ||
             ($tcpEndpoint->type() == $wsEndpointType && !$tcpEndpoint->secure()) ||
             ($tcpEndpoint->type() == $wssEndpointType && $tcpEndpoint->secure()));
        test(($tcpEndpoint->type() == $tcpEndpointType && ($endpoint instanceof $tcpEndpointInfoClass)) ||
             ($tcpEndpoint->type() == $sslEndpointType && ($endpoint instanceof $sslEndpointInfoClass)) ||
             ($tcpEndpoint->type() == $wsEndpointType && ($endpoint instanceof $wsEndpointInfoClass)) ||
             ($tcpEndpoint->type() == $wssEndpointType && ($endpoint instanceof $wsEndpointInfoClass)));

        $udpEndpoint = $endps[1]->getInfo();
        test($udpEndpoint instanceof $udpEndpointInfoClass);
        test($udpEndpoint->host == "udphost");
        test($udpEndpoint->port == 10001);
        test($udpEndpoint->sourceAddress == "10.10.10.10");
        test($udpEndpoint->mcastInterface == "eth0");
        test($udpEndpoint->mcastTtl == 5);
        test($udpEndpoint->timeout == -1);
        test(!$udpEndpoint->compress);
        test(!$udpEndpoint->secure());
        test($udpEndpoint->datagram());
        test($udpEndpoint->type() == $udpEndpointType);

        $opaqueEndpoint = $endps[2]->getInfo();
        test($opaqueEndpoint);
    }
    echo "ok\n";

    $defaultHost = $communicator->getProperties()->getProperty("Ice.Default.Host");
    $base = $communicator->stringToProxy("test:default -p 12010:udp -p 12010");
    $testIntf = $base->ice_checkedCast("::Test::TestIntf");

    echo "test connection endpoint information... ";
    flush();
    {
        $tcpinfo = getTCPEndpointInfo($base->ice_getConnection()->getEndpoint()->getInfo());
        test($tcpinfo instanceof $tcpEndpointInfoClass);
        test($tcpinfo->port == 12010);
        test(!$tcpinfo->compress);
        test($tcpinfo->host == $defaultHost);

        $ctx = $testIntf->getEndpointInfoAsContext();
        test($ctx["host"] == $tcpinfo->host);
        test($ctx["compress"] == "false");
        test($ctx["port"] > 0);

        $udpinfo = $base->ice_datagram()->ice_getConnection()->getEndpoint()->getInfo();
        test($udpinfo instanceof $udpEndpointInfoClass);
        test($udpinfo->port == 12010);
        test($udpinfo->host == $defaultHost);
    }
    echo "ok\n";

    echo "testing connection information... ";
    flush();
    {
        $ipConnectionInfoClass = $NS ? "Ice\\TCPConnectionInfo" : "Ice_TCPConnectionInfo";
        $wsConnectionInfoClass = $NS ? "Ice\\WSConnectionInfo" : "Ice_WSConnectionInfo";

        $connection = $base->ice_getConnection();
        $connection->setBufferSize(1024, 2048);

        $info = $connection->getInfo();
        $tcpinfo = getTCPConnectionInfo($info);
        test($tcpinfo instanceof $ipConnectionInfoClass);
        test(!$info->incoming);
        test(strlen($info->adapterName) == 0);
        test($tcpinfo->remotePort == 12010);
        if($defaultHost == "127.0.0.1")
        {
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

        if($base->ice_getConnection()->type() == "ws" || $base->ice_getConnection()->type() == "wss")
        {
            test($info instanceof $wsConnectionInfoClass);

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

$communicator = Ice_initialize($argv);
$server = allTests($communicator);
$server->shutdown();
$communicator->destroy();

exit();
?>
