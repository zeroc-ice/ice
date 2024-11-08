//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";
import { Test } from "./Controller.js";
import { ControllerHelper } from "./ControllerHelper.js";

export class Logger extends Ice.Logger {
    constructor(out) {
        super();
        this._out = out;
    }

    write(message, indent) {
        if (indent) {
            message = message.replace(/\n/g, "\n   ");
        }
        this._out.writeLine(message);
    }
}

class ProcessI extends Test.Common.Process {
    constructor(promise, helper, output) {
        super();
        this._promise = promise;
        this._helper = helper;
        this._output = output;
    }

    async waitReady() {
        await this._helper.waitReady();
    }

    async waitSuccess() {
        try {
            await this._promise;
            this._helper.serverReady();
            return 0;
        } catch (ex) {
            this._output.writeLine(`unexpected exception while running test: ${ex.toString()}`);
            this._output.writeLine(ex.stack);
            this._helper.serverReady(ex);
            return 1;
        }
    }

    terminate(current) {
        current.adapter.remove(current.id);
        return this._output.get();
    }
}

class ProcessControllerI extends Test.Common.BrowserProcessController {
    constructor(clientOutput, serverOutput, useWorker) {
        super();
        this._clientOutput = clientOutput;
        this._serverOutput = serverOutput;
        this._useWorker = useWorker;
    }

    async start(testSuite, exe, args, current) {
        let promise;
        let out;
        if (exe === "Server.js" || exe === "ServerAMD.js") {
            out = this._serverOutput;
        } else {
            out = this._clientOutput;
        }
        out.clear();
        const helper = new ControllerHelper(exe, out);

        if (this._useWorker) {
            promise = new Promise((resolve, reject) => {
                const worker = new Worker("/test/Common/ControllerWorker.js", { type: "module" });
                this._worker = worker;
                worker.onmessage = function (e) {
                    if (e.data.type == "write") {
                        helper.write(e.data.message);
                    } else if (e.data.type == "writeLine") {
                        helper.writeLine(e.data.message);
                    } else if (e.data.type == "ready") {
                        helper.serverReady();
                    } else if (e.data.type == "finished") {
                        if (e.data.exception) {
                            reject(e.data.exception);
                        } else {
                            resolve();
                        }
                        worker.terminate();
                    }
                };
                worker.postMessage({ exe: exe, testSuite: testSuite, args: args });
            });
        } else {
            const entryPoints = {
                "Server.js": "Server",
                "ServerAMD.js": "ServerAMD",
                "Client.js": "Client",
            };

            const module = await import(`/test/${testSuite}/index.js`);
            const cls = module[entryPoints[exe]];
            const test = new cls();
            test.setControllerHelper(helper);
            promise = test.run(args);
        }
        return Test.Common.ProcessPrx.uncheckedCast(current.adapter.addWithUUID(new ProcessI(promise, helper, out)));
    }

    getHost() {
        return "127.0.0.1";
    }

    redirect(url, current) {
        current.con.close();
        window.location.href = url;
    }
}

export async function runController(clientOutput, serverOutput) {
    class Output {
        constructor(output) {
            this.output = output;
        }

        write(msg) {
            this.output.value += msg;
        }

        writeLine(msg) {
            this.write(msg + "\n");
            this.output.scrollTop = this.output.scrollHeight;
        }

        get() {
            return this.output.value;
        }

        clear() {
            this.output.value = "";
        }
    }

    const out = new Output(clientOutput);
    const serverOut = new Output(serverOutput);

    const url = new URL(document.location);
    const protocol = url.protocol === "http:" ? "ws" : "wss";
    const port = url.searchParams.get("port") || 15002;
    const worker = url.searchParams.get("worker") === "True";

    const initData = new Ice.InitializationData();
    initData.logger = new Logger(out);
    initData.properties = Ice.createProperties();

    async function registerProcessController(adapter, registry, processController) {
        try {
            await registry.ice_ping();
            const connection = registry.ice_getCachedConnection();
            connection.setAdapter(adapter);
            connection.setCloseCallback(() => out.writeLine("connection with process controller registry closed"));
            await registry.setProcessController(Test.Common.ProcessControllerPrx.uncheckedCast(processController));
        } catch (ex) {
            if (ex instanceof Ice.ConnectFailedException) {
                setTimeout(() => registerProcessController(adapter, registry, processController), 2000);
            } else {
                out.writeLine(
                    "unexpected exception while connecting to process controller registry:\n" + ex.toString(),
                );
                out.writeLine(ex.stack);
            }
        }
    }

    let comm;
    try {
        comm = Ice.initialize(initData);
        const str = `Util/ProcessControllerRegistry:${protocol} -h ${document.location.hostname} -p ${port}`;
        const registry = Test.Common.ProcessControllerRegistryPrx.uncheckedCast(comm.stringToProxy(str));
        const adapter = await comm.createObjectAdapter("");
        const ident = new Ice.Identity("ProcessController", "Browser");
        const processController = adapter.add(new ProcessControllerI(out, serverOut, worker), ident);
        registerProcessController(adapter, registry, processController);
    } catch (ex) {
        out.writeLine("unexpected exception while creating controller:\n" + ex.toString());
        comm.destroy();
    }
}
