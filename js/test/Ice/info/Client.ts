// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

function getTCPEndpointInfo(info: Ice.EndpointInfo): Ice.TCPEndpointInfo | null {
    for (let p = info; p; p = p.underlying) {
        if (p instanceof Ice.TCPEndpointInfo) {
            return p;
        }
    }
    return null;
}

function getTCPConnectionInfo(info: Ice.ConnectionInfo): Ice.TCPConnectionInfo | null {
    for (let p = info; p; p = p.underlying) {
        if (p instanceof Ice.TCPConnectionInfo) {
            return p;
        }
    }
    return null;
}

export class Client extends TestHelper {
    async allTests() {
        const out = this.getWriter();
        const communicator = this.communicator();
        const defaultHost = communicator.getProperties().getIceProperty("Ice.Default.Host");

        out.write("testing proxy endpoint information... ");
        const ref =
            "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:opaque -e 1.8 -t 100 -v ABCD";
        const p1 = new Ice.ObjectPrx(communicator, ref);

        let endpoints = p1.ice_getEndpoints();
        let endpoint = endpoints[0].getInfo();
        let ipEndpoint = getTCPEndpointInfo(endpoint);
        test(ipEndpoint != null);
        test(ipEndpoint!.host == "tcphost");
        test(ipEndpoint!.port == 10000);
        test(ipEndpoint!.timeout == 1200);
        test(ipEndpoint!.sourceAddress == "10.10.10.10");
        test(ipEndpoint!.compress);
        test(!ipEndpoint!.datagram());
        test(
            (ipEndpoint!.type() == Ice.TCPEndpointType && !ipEndpoint!.secure()) ||
                (ipEndpoint!.type() == Ice.WSEndpointType && !ipEndpoint!.secure()) ||
                (ipEndpoint!.type() == Ice.WSSEndpointType && ipEndpoint!.secure()),
        );

        test(
            (ipEndpoint!.type() == Ice.TCPEndpointType && endpoint instanceof Ice.TCPEndpointInfo) ||
                (ipEndpoint!.type() == Ice.WSEndpointType && endpoint instanceof Ice.WSEndpointInfo) ||
                (ipEndpoint!.type() == Ice.WSSEndpointType && endpoint instanceof Ice.WSEndpointInfo),
        );

        const opaqueEndpoint = endpoints[1].getInfo() as Ice.OpaqueEndpointInfo;
        test(opaqueEndpoint.rawEncoding.equals(new Ice.EncodingVersion(1, 8)));
        out.writeLine("ok");

        out.write("testing connection endpoint information... ");
        const testIntf = new Test.TestIntfPrx(communicator, `test:${this.getTestEndpoint()}`);
        const endpointPort = this.getTestPort(0);
        let conn = await testIntf.ice_getConnection();
        let ipinfo = getTCPEndpointInfo(conn.getEndpoint().getInfo());
        test(ipinfo != null);
        test(ipinfo!.port == endpointPort);
        test(!ipinfo!.compress);
        test(ipinfo!.host == defaultHost);

        let ctx = await testIntf.getEndpointInfoAsContext();
        test(ctx.get("host") == ipinfo!.host);
        test(ctx.get("compress") == "false");
        let port = ctx.get("port");
        test(port !== undefined);
        test(parseInt(port as string) > 0);
        out.writeLine("ok");

        out.write("testing connection information... ");

        conn = await testIntf.ice_getConnection();
        conn.setBufferSize(1024, 2048);

        const info = conn.getInfo();
        let ipConnectionInfo: Ice.TCPConnectionInfo | null = getTCPConnectionInfo(info);
        test(ipConnectionInfo != null);
        test(info.adapterName.length === 0);
        if (conn.type() != "ws" && conn.type() != "wss") {
            test(ipConnectionInfo!.localPort > 0);
        }
        test(ipConnectionInfo!.remotePort == endpointPort);
        if (defaultHost == "127.0.0.1") {
            test(ipConnectionInfo!.remoteAddress == defaultHost);
            if (conn.type() != "ws" && conn.type() != "wss") {
                test(ipConnectionInfo!.localAddress == defaultHost);
            }
        }
        test(ipConnectionInfo!.sndSize >= 2048);
        ctx = await testIntf.getConnectionInfoAsContext();

        test(ctx.get("incoming") == "true");
        test(ctx.get("adapterName") == "TestAdapter");
        if (conn.type() != "ws" && conn.type() != "wss") {
            test(ctx.get("remoteAddress") == ipConnectionInfo!.localAddress);
            test(ctx.get("localAddress") == ipConnectionInfo!.remoteAddress);
            test(parseInt(ctx.get("remotePort")!) === ipConnectionInfo!.localPort);
            test(parseInt(ctx.get("localPort")!) === ipConnectionInfo!.remotePort);
        }

        function getHeader(ctx: Map<string, string>, key: string): string | undefined {
            return ctx.get(key) || ctx.get(key.toLowerCase());
        }

        if (conn.type() == "ws" || conn.type() == "wss") {
            test(getHeader(ctx, "ws.Upgrade")!.toLowerCase() == "websocket");
            test(
                getHeader(ctx, "ws.Connection")!.indexOf("Upgrade") >= 0 ||
                    getHeader(ctx, "ws.Connection")!.indexOf("upgrade") >= 0,
            );
            test(getHeader(ctx, "ws.Sec-WebSocket-Protocol") == "ice.zeroc.com");
            test(getHeader(ctx, "ws.Sec-WebSocket-Version") == "13");
            test(getHeader(ctx, "ws.Sec-WebSocket-Key") !== null);
        }
        out.writeLine("ok");

        await testIntf.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator] = this.initialize(args);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
