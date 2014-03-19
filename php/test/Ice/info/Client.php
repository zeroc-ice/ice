<?
// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        die("\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n");
    }
}

function allTests($communicator)
{
    global $NS;

    $ipEndpointInfoClass = $NS ? "Ice\\IPEndpointInfo" : "Ice_IPEndpointInfo";
    $tcpEndpointType = $NS ? constant("Ice\\TCPEndpointType") : constant("Ice_TCPEndpointType");
    $tcpEndpointInfoClass = $NS ? "Ice\\TCPEndpointInfo" : "Ice_TCPEndpointInfo";
    $udpEndpointType = $NS ? constant("Ice\\UDPEndpointType") : constant("Ice_UDPEndpointType");
    $udpEndpointInfoClass = $NS ? "Ice\\UDPEndpointInfo" : "Ice_UDPEndpointInfo";
    $sslEndpointType = 2;
    $protocolVersionClass = $NS ? "Ice\\ProtocolVersion" : "Ice_ProtocolVersion";
    $encodingVersionClass = $NS ? "Ice\\EncodingVersion" : "Ice_EncodingVersion";

    echo "testing proxy endpoint information... ";
    flush();
    {
        $p1 = $communicator->stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z:" .
                                           "udp -h udphost -p 10001 --interface eth0 --ttl 5:" .
                                           "opaque -e 1.8 -t 100 -v ABCD");

        $endps = $p1->ice_getEndpoints();

        $ipEndpoint = $endps[0]->getInfo();
        test($ipEndpoint instanceof $ipEndpointInfoClass);
        test($ipEndpoint->host == "tcphost");
        test($ipEndpoint->port == 10000);
        test($ipEndpoint->timeout == 1200);
        test($ipEndpoint->compress);
        test(!$ipEndpoint->datagram());
        test($ipEndpoint->type() == $tcpEndpointType && !$ipEndpoint->secure() ||
             $ipEndpoint->type() == $sslEndpointType && $ipEndpoint->secure());
        test($ipEndpoint->type() == $tcpEndpointType && ($ipEndpoint instanceof $tcpEndpointInfoClass)  ||
             $ipEndpoint->type() == $sslEndpointType);

        $udpEndpoint = $endps[1]->getInfo();
        test($udpEndpoint instanceof $udpEndpointInfoClass);
        test($udpEndpoint->host == "udphost");
        test($udpEndpoint->port == 10001);
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
        $ipinfo = $base->ice_getConnection()->getEndpoint()->getInfo();
        test($ipinfo instanceof $ipEndpointInfoClass);
        test($ipinfo->port == 12010);
        test(!$ipinfo->compress);
        test($ipinfo->host == $defaultHost);

        $ctx = $testIntf->getEndpointInfoAsContext();
        test($ctx["host"] == $ipinfo->host);
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
        $ipConnectionInfoClass = $NS ? "Ice\\IPConnectionInfo" : "Ice_IPConnectionInfo";

        $info = $base->ice_getConnection()->getInfo();
        test($info instanceof $ipConnectionInfoClass);
        test(!$info->incoming);
        test(strlen($info->adapterName) == 0);
        test($info->remotePort == 12010);
        test($info->remoteAddress == $defaultHost);
        test($info->localAddress == $defaultHost);

        $ctx = $testIntf->getConnectionInfoAsContext();
        test($ctx["incoming"] == "true");
        test($ctx["adapterName"] == "TestAdapter");
        test($ctx["remoteAddress"] == $info->localAddress);
        test($ctx["localAddress"] == $info->remoteAddress);
        test($ctx["remotePort"] == $info->localPort);
        test($ctx["localPort"] == $info->remotePort);
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
