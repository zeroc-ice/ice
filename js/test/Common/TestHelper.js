// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";

export class TestHelper {
    getTestEndpoint(...args) {
        let properties;
        let num;
        let protocol = "";

        if (args.length > 0) {
            if (typeof args[0] == "object") {
                properties = args[0];
                if (args.length > 1) {
                    num = args[1];
                    if (args.length > 2) {
                        protocol = args[2];
                    }
                }
            } else if (typeof args[0] == "number") {
                num = args[0];
                if (args.length > 1) {
                    protocol = args[1];
                }
            } else {
                protocol = args[0];
            }
        }

        if (properties === undefined) {
            properties = this._communicator.getProperties();
        }

        if (num === undefined) {
            num = 0;
        }

        if (protocol == "") {
            protocol = properties.getIceProperty("Ice.Default.Protocol");
        }

        const port = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;

        return `${protocol} -p ${port}`;
    }

    getTestHost(properties) {
        if (properties === undefined) {
            properties = this._communicator.getProperties();
        }
        return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
    }

    getTestProtocol(properties) {
        if (properties === undefined) {
            properties = this._communicator.getProperties();
        }
        return properties.getIceProperty("Ice.Default.Protocol");
    }

    getTestPort(...args) {
        let properties;
        let num;
        if (args.length > 1) {
            properties = args[0];
            num = args[1];
        } else {
            num = args[0];
        }

        if (properties === undefined) {
            properties = this._communicator.getProperties();
        }
        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
    }

    updateLogFileProperty(properties, suffix) {
        const logFile = properties.getIceProperty("Ice.LogFile");
        if (logFile) {
            const newLogFile =
                logFile.substring(0, logFile.lastIndexOf(".")) + suffix + logFile.substring(logFile.lastIndexOf("."));
            properties.setProperty("Ice.LogFile", newLogFile);
        }
    }

    createTestProperties(args = []) {
        const properties = new Ice.Properties(args);
        args = properties.parseCommandLineOptions("Test", args);
        return [properties, args];
    }

    initialize(...args) {
        let initData;
        if (args[0] instanceof Ice.InitializationData) {
            initData = args[0];
        } else {
            initData = new Ice.InitializationData();
            if (args[0] instanceof Ice.Properties) {
                initData.properties = args[0];
            } else {
                [initData.properties, args[0]] = this.createTestProperties(args[0]);
            }
        }

        const communicator = Ice.initialize(initData);
        if (this._communicator === undefined) {
            this._communicator = communicator;
        }
        return [communicator, args[0]];
    }

    communicator() {
        return this._communicator;
    }

    shutdown() {
        if (this._communicator !== undefined) {
            this._communicator.shutdown();
        }
    }

    getWriter() {
        return this.controllerHelper;
    }

    setControllerHelper(controllerHelper) {
        this.controllerHelper = controllerHelper;
    }

    serverReady() {
        this.controllerHelper.serverReady();
    }

    static isBrowser() {
        return typeof window !== "undefined" || TestHelper.isWorker();
    }

    static isWorker() {
        return typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope;
    }
}

export function test(value, error) {
    if (!value) {
        let message = "test failed";
        if (error) {
            message += `\n${error}`;
        }
        throw new Error(message);
    }
}
