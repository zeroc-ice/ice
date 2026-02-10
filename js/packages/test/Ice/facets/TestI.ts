// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";

export class DI extends Test.D {
    callA(_: Ice.Current) {
        return "A";
    }

    callB(_: Ice.Current) {
        return "B";
    }

    callC(_: Ice.Current) {
        return "C";
    }

    callD(_: Ice.Current) {
        return "D";
    }
}

export class EmptyI extends Test.Empty {}

export class FI extends Test.F {
    callE(_: Ice.Current) {
        return "E";
    }

    callF(_: Ice.Current) {
        return "F";
    }
}

export class HI extends Test.H {
    callG(_: Ice.Current) {
        return "G";
    }

    callH(_: Ice.Current) {
        return "H";
    }

    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}
