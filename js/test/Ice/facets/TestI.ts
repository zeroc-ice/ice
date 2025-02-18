// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";

export class DI extends Test.D {
    callA(current: Ice.Current) {
        return "A";
    }

    callB(current: Ice.Current) {
        return "B";
    }

    callC(current: Ice.Current) {
        return "C";
    }

    callD(current: Ice.Current) {
        return "D";
    }
}

export class EmptyI extends Test.Empty {}

export class FI extends Test.F {
    callE(current: Ice.Current) {
        return "E";
    }

    callF(current: Ice.Current) {
        return "F";
    }
}

export class HI extends Test.H {
    callG(current: Ice.Current) {
        return "G";
    }

    callH(current: Ice.Current) {
        return "H";
    }

    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}
