// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

class Middleware extends Ice.Object {
    async dispatch(request: Ice.IncomingRequest): Promise<Ice.OutgoingResponse> {
        if (request.current.operation === "shutdown") {
            return this._next.dispatch(request);
        } else {
            this._inLog.push(this._name);
            const response = await this._next.dispatch(request);
            this._outLog.push(this._name);
            return response;
        }
    }

    constructor(next: Ice.Object, name: string, inLog: string[], outLog: string[]) {
        super();
        this._next = next;
        this._name = name;
        this._inLog = inLog;
        this._outLog = outLog;
    }

    private _next: Ice.Object;
    private _name: string;
    private _inLog: string[];
    private _outLog: string[];
}

class MyObjectI extends Test.MyObject {
    constructor() {
        super();
    }

    getName(current: Ice.Current): string {
        return "Foo";
    }

    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}

export class Server extends TestHelper {
    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        let echo: Test.EchoPrx | null = null;
        const inLog: string[] = [];
        const outLog: string[] = [];
        try {
            [communicator] = this.initialize(args);
            echo = new Test.EchoPrx(communicator, "__echo:" + this.getTestEndpoint());

            let adapter = await communicator.createObjectAdapter("");

            await echo.setConnection();
            echo.ice_getCachedConnection().setAdapter(adapter);
            this.serverReady();

            adapter.add(new MyObjectI(), new Ice.Identity("test"));
            adapter
                .use((next: Ice.Object) => new Middleware(next, "A", inLog, outLog))
                .use((next: Ice.Object) => new Middleware(next, "B", inLog, outLog))
                .use((next: Ice.Object) => new Middleware(next, "C", inLog, outLog));
            await communicator.waitForShutdown();
        } finally {
            const out = this.getWriter();
            out.write("testing middleware execution order... ");

            test(inLog.length === 3);
            test(inLog[0] === "A");
            test(inLog[1] === "B");
            test(inLog[2] === "C");

            test(outLog.length === 3);
            test(outLog[0] === "C");
            test(outLog[1] === "B");
            test(outLog[2] === "A");
            out.writeLine("ok");

            if (echo) {
                await echo.shutdown();
            }

            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
