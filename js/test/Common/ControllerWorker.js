// Copyright (c) ZeroC, Inc.

/* global self */

import { ControllerHelper } from "./ControllerHelper.js";

class Output {
    static write(message) {
        self.postMessage({ type: "write", message: message });
    }

    static writeLine(message) {
        self.postMessage({ type: "writeLine", message: message });
    }
}

self.onmessage = async e => {
    try {
        const helper = new ControllerHelper(e.data.exe, Output);

        const entryPoints = {
            "Server.js": "Server",
            "ServerAMD.js": "ServerAMD",
            "Client.js": "Client",
        };

        const module = await import(`/test/${e.data.testSuite}/index.js`);
        const cls = module[entryPoints[e.data.exe]];

        const test = new cls();
        test.setControllerHelper(helper);
        const promise = test.run(e.data.args);
        await helper.waitReady();
        self.postMessage({ type: "ready" });
        await promise;
        self.postMessage({ type: "finished" });
    } catch (ex) {
        self.postMessage({ type: "finished", exception: ex.toString() });
    }
};
