// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class ThrowerI extends Test.Thrower {
    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }

    supportsUndeclaredExceptions(current: Ice.Current) {
        return true;
    }

    supportsAssertException(current: Ice.Current) {
        return false;
    }

    throwAasA(a: number, current: Ice.Current) {
        throw new Test.A(a);
    }

    throwAorDasAorD(a: number, current: Ice.Current) {
        if (a > 0) {
            throw new Test.A(a);
        } else {
            throw new Test.D(a);
        }
    }

    throwBasA(a: number, b: number, current: Ice.Current) {
        this.throwBasB(a, b, current);
    }

    throwBasB(a: number, b: number, current: Ice.Current) {
        throw new Test.B(a, b);
    }

    throwCasA(a: number, b: number, c: number, current: Ice.Current) {
        this.throwCasC(a, b, c, current);
    }

    throwCasB(a: number, b: number, c: number, current: Ice.Current) {
        this.throwCasC(a, b, c, current);
    }

    throwCasC(a: number, b: number, c: number, current: Ice.Current) {
        throw new Test.C(a, b, c);
    }

    throwUndeclaredA(a: number, current: Ice.Current) {
        throw new Test.A(a);
    }

    throwUndeclaredB(a: number, b: number, current: Ice.Current) {
        throw new Test.B(a, b);
    }

    throwUndeclaredC(a: number, b: number, c: number, current: Ice.Current) {
        throw new Test.C(a, b, c);
    }

    throwLocalException(current: Ice.Current) {
        throw new Ice.TimeoutException();
    }

    throwLocalExceptionIdempotent(current: Ice.Current) {
        throw new Ice.TimeoutException();
    }

    throwNonIceException(current: Ice.Current) {
        throw new Error();
    }

    throwAssertException(current: Ice.Current) {
        test(false);
    }

    throwMemoryLimitException(seq: Uint8Array, current: Ice.Current) {
        return new Uint8Array(1024 * 20); // 20KB is over the configured 10KB message size max.
    }

    throwDispatchException(replyStatus: number, current: Ice.Current) {
        // valueOf create a new ReplyStatus enumerator if one doesn't exist already.
        throw new Ice.DispatchException(Ice.ReplyStatus.valueOf(replyStatus));
    }

    throwAfterResponse(current: Ice.Current) {
        //
        // Only relevant for AMD.
        //
    }

    throwAfterException(current: Ice.Current) {
        //
        // Only relevant for AMD.
        //
        throw new Test.A();
    }
}
