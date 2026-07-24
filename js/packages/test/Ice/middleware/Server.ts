// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper, test } from "../../Common/TestHelper.js";

class Middleware extends Ice.Object {
    override async dispatch(request: Ice.IncomingRequest): Promise<Ice.OutgoingResponse> {
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

class NullLogger implements Ice.Logger {
    print(_message: string): void {}

    trace(_category: string, _message: string): void {}

    warning(_message: string): void {}

    error(_message: string): void {}

    cloneWithPrefix(_prefix: string): Ice.Logger {
        return this;
    }
}

class MyObjectI extends Test.MyObject {
    constructor() {
        super();
    }

    getName(): string {
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
            echo = new Test.EchoPrx(communicator, `__echo:${this.getTestEndpoint()}`);

            const adapter = await communicator.createObjectAdapter("");

            await echo.setConnection();
            echo.ice_getCachedConnection()!.setAdapter(adapter);
            this.serverReady();

            adapter.add(new MyObjectI(), new Ice.Identity("test"));
            adapter
                .use((next: Ice.Object) => new Middleware(next, "A", inLog, outLog))
                .use((next: Ice.Object) => new Middleware(next, "B", inLog, outLog))
                .use((next: Ice.Object) => new Middleware(next, "C", inLog, outLog));
            await communicator.waitForShutdown();

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

            await this.testMiddlewareFactoryException(args);
        } finally {
            if (echo) {
                await echo.shutdown();
            }

            if (communicator) {
                await communicator.destroy();
            }
        }
    }

    // Verifies a middleware factory exception makes all dispatches fail with a generic UnknownException.
    async testMiddlewareFactoryException(args: string[]) {
        const out = this.getWriter();
        out.write("testing middleware factory exception... ");

        // Use a separate communicator with a null logger: the pipeline creation failure is logged as an error.
        const initData = new Ice.InitializationData();
        [initData.properties] = this.createTestProperties(args);
        initData.logger = new NullLogger();
        const [communicator] = this.initialize(initData);
        try {
            const echo = new Test.EchoPrx(communicator, `__echo:${this.getTestEndpoint()}`);
            const adapter = await communicator.createObjectAdapter("");
            // Takes over the echo server's single relay connection: the main test must be finished by now.
            await echo.setConnection();
            echo.ice_getCachedConnection()!.setAdapter(adapter);

            adapter.add(new MyObjectI(), new Ice.Identity("test"));
            adapter.use(() => {
                throw new Error("middleware factory exception");
            });

            const prx = new Test.MyObjectPrx(communicator, `test: ${this.getTestEndpoint()}`);

            try {
                await prx.getName();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownException);
                // The message does not reveal the middleware factory exception.
                test(!(ex as Ice.UnknownException).message.includes("middleware factory exception"));
            }

            // The failure is permanent for this object adapter.
            try {
                await prx.getName();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.UnknownException);
            }
        } finally {
            await communicator.destroy();
        }
        out.writeLine("ok");
    }
}
