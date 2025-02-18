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

    createTestProperties(args = []) {
        const properties = Ice.createProperties(args);
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

    static test(value, ex) {
        if (!value) {
            let message = "test failed";
            if (ex) {
                message += "\n" + ex.toString();
            }
            throw new Error(message);
        }
    }

    static isSafari() {
        return typeof navigator !== "undefined" && /^((?!chrome).)*safari/i.test(navigator.userAgent);
    }

    static isBrowser() {
        return typeof window !== "undefined" || TestHelper.isWorker();
    }

    static isWorker() {
        return typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope;
    }
}
