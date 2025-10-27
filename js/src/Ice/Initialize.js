// Copyright (c) ZeroC, Inc.

import { Communicator } from "./Communicator.js";
import { Protocol } from "./Protocol.js";
import { Properties } from "./Properties.js";

export function initialize(arg1, arg2) {
    return new Communicator(arg1, arg2);
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
