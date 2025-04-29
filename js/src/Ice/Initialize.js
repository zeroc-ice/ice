// Copyright (c) ZeroC, Inc.

import { Communicator } from "./Communicator.js";
import { Protocol } from "./Protocol.js";
import { InitializationException } from "./LocalExceptions.js";
import { Properties } from "./Properties.js";
import { defaultSliceLoaderInstance } from "./DefaultSliceLoader.js";

export class InitializationData {
    constructor() {
        this.properties = null;
        this.logger = null;
        this.sliceLoader = null;
        this.valueFactoryManager = null;
    }

    clone() {
        const r = new InitializationData();
        r.properties = this.properties;
        r.logger = this.logger;
        r.sliceLoader = this.sliceLoader;
        r.valueFactoryManager = this.valueFactoryManager;
        return r;
    }
}

export function initialize(arg1, arg2) {
    let args = null;
    let initData = null;

    if (arg1 instanceof Array) {
        args = arg1;
    } else if (arg1 instanceof InitializationData) {
        initData = arg1;
    } else if (arg1 !== undefined && arg1 !== null) {
        throw new InitializationException("invalid argument to initialize");
    }

    if (arg2 !== undefined && arg2 !== null) {
        if (arg2 instanceof InitializationData && initData === null) {
            initData = arg2;
        } else {
            throw new InitializationException("invalid argument to initialize");
        }
    }

    if (initData === null) {
        initData = new InitializationData();
    } else {
        initData = initData.clone();
    }
    initData.properties = new Properties(args, initData.properties);

    if (initData.sliceLoader === null || initData.sliceLoader === undefined) {
        initData.sliceLoader = defaultSliceLoaderInstance;
    }

    return new Communicator(initData);
}

export function createProperties(args, defaults) {
    return new Properties(args, defaults);
}

export function currentProtocol() {
    return Protocol.currentProtocol.clone();
}

export function currentEncoding() {
    return Protocol.currentEncoding.clone();
}

export function stringVersion() {
    return "3.8.0-alpha.0"; // "A.B.C", with A=major, B=minor, C=patch
}

export function intVersion() {
    return 30850; // AABBCC, with AA=major, BB=minor, CC=patch
}
