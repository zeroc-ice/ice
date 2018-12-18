// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated"
import {TestHelper} from "../../../Common/TestHelper"
const test = TestHelper.test;

function getTCPEndpointInfo(info:Ice.EndpointInfo)
{
    for(let p = info; p; p = p.underlying)
    {
        if(p instanceof Ice.TCPEndpointInfo)
        {
            return p;
        }
    }
    return null;
}

function getTCPConnectionInfo(info:Ice.ConnectionInfo)
{
    for(let p = info; p; p = p.underlying)
    {
        if(p instanceof Ice.TCPConnectionInfo)
        {
            return p;
        }
    }
    return null;
}

export class Client extends TestHelper
{
    async allTests()
    {
        const out = this.getWriter();
        const communicator = this.communicator();
        const defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");

        out.write("testing proxy endpoint information... ");
        {
            const ref =
                    "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:opaque -e 1.8 -t 100 -v ABCD";
            const p1 = communicator.stringToProxy(ref);

            const endps = p1.ice_getEndpoints();
            const endpoint = endps[0].getInfo();
            const ipEndpoint = getTCPEndpointInfo(endpoint);
            test(ipEndpoint.host == "tcphost");
            test(ipEndpoint.port == 10000);
            test(ipEndpoint.timeout == 1200);
            test(ipEndpoint.sourceAddress == "10.10.10.10");
            test(ipEndpoint.compress);
            test(!ipEndpoint.datagram());
            test(ipEndpoint.type() == Ice.TCPEndpointType && !ipEndpoint.secure() ||
                    ipEndpoint.type() == Ice.WSEndpointType && !ipEndpoint.secure() ||
                    ipEndpoint.type() == Ice.WSSEndpointType && ipEndpoint.secure());

            test(ipEndpoint.type() == Ice.TCPEndpointType && endpoint instanceof Ice.TCPEndpointInfo ||
                    ipEndpoint.type() == Ice.WSEndpointType && endpoint instanceof Ice.WSEndpointInfo ||
                    ipEndpoint.type() == Ice.WSSEndpointType && endpoint instanceof Ice.WSEndpointInfo);

            const opaqueEndpoint = endps[1].getInfo() as Ice.OpaqueEndpointInfo;
            test(opaqueEndpoint.rawEncoding.equals(new Ice.EncodingVersion(1, 8)));
        }
        out.writeLine("ok");

        const base = communicator.stringToProxy("test:" + this.getTestEndpoint());
        const testIntf = Test.TestIntfPrx.uncheckedCast(base);

        out.write("testing connection endpoint information... ");
        {
            const endpointPort = this.getTestPort(0);
            let conn = await base.ice_getConnection();
            let ipinfo = getTCPEndpointInfo(conn.getEndpoint().getInfo());
            test(ipinfo.port == endpointPort);
            test(!ipinfo.compress);
            test(ipinfo.host == defaultHost);

            let ctx = await testIntf.getEndpointInfoAsContext();
            test(ctx.get("host") == ipinfo.host);
            test(ctx.get("compress") == "false");
            test(parseInt(ctx.get("port")) > 0);
        }
        out.writeLine("ok");

        out.write("testing connection information... ");
        {
            const endpointPort = this.getTestPort(0);
            let conn = await base.ice_getConnection();
            conn = await base.ice_getConnection();
            conn.setBufferSize(1024, 2048);

            const info:Ice.ConnectionInfo = conn.getInfo();
            let ipinfo:Ice.TCPConnectionInfo = getTCPConnectionInfo(info);
            test(!info.incoming);
            test(info.adapterName.length === 0);
            if(conn.type() != "ws" && conn.type() != "wss")
            {
                test(ipinfo.localPort > 0);
            }
            test(ipinfo.remotePort == endpointPort);
            if(defaultHost == "127.0.0.1")
            {
                test(ipinfo.remoteAddress == defaultHost);
                if(conn.type() != "ws" && conn.type() != "wss")
                {
                    test(ipinfo.localAddress == defaultHost);
                }
            }
            test(ipinfo.sndSize >= 2048);
            const ctx:Map<string, string> = await testIntf.getConnectionInfoAsContext();

            test(ctx.get("incoming") == "true");
            test(ctx.get("adapterName") == "TestAdapter");
            if(conn.type() != "ws" && conn.type() != "wss")
            {
                test(ctx.get("remoteAddress") == ipinfo.localAddress);
                test(ctx.get("localAddress") == ipinfo.remoteAddress);
                test(parseInt(ctx.get("remotePort")) === ipinfo.localPort);
                test(parseInt(ctx.get("localPort")) === ipinfo.remotePort);
            }

            if(conn.type() == "ws" || conn.type() == "wss")
            {
                test(ctx.get("ws.Upgrade").toLowerCase() == "websocket");
                test(ctx.get("ws.Connection").indexOf("Upgrade") >= 0);
                test(ctx.get("ws.Sec-WebSocket-Protocol") == "ice.zeroc.com");
                test(ctx.get("ws.Sec-WebSocket-Version") == "13");
                test(ctx.get("ws.Sec-WebSocket-Key") !== null);
            }
        }
        out.writeLine("ok");

        await testIntf.shutdown();
    }

    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        try
        {
            [communicator] = this.initialize(args);
            await this.allTests();
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }
}
