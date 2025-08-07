// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { test } from "../../Common/TestHelper.js";

export class AMDThrowerI extends Test.Thrower {
    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }

    supportsUndeclaredExceptions(_: Ice.Current) {
        return true;
    }

    supportsAssertException(_: Ice.Current) {
        return false;
    }

    throwAasA(a: number, _: Ice.Current) {
        throw new Test.A(a);
    }

    throwAorDasAorD(a: number, _: Ice.Current) {
        if (a > 0) {
            throw new Test.A(a);
        } else {
            throw new Test.D(a);
        }
    }

    throwBasA(a: number, b: number, current: Ice.Current) {
        return this.throwBasB(a, b, current);
    }

    throwBasB(a: number, b: number, _: Ice.Current) {
        throw new Test.B(a, b);
    }

    throwCasA(a: number, b: number, c: number, current: Ice.Current) {
        return this.throwCasC(a, b, c, current);
    }

    throwCasB(a: number, b: number, c: number, current: Ice.Current) {
        return this.throwCasC(a, b, c, current);
    }

    throwCasC(a: number, b: number, c: number, _: Ice.Current) {
        throw new Test.C(a, b, c);
    }

    throwUndeclaredA(a: number, _: Ice.Current) {
        throw new Test.A(a);
    }

    throwUndeclaredB(a: number, b: number, _: Ice.Current) {
        throw new Test.B(a, b);
    }

    throwUndeclaredC(a: number, b: number, c: number, _: Ice.Current) {
        throw new Test.C(a, b, c);
    }

    throwLocalException(_: Ice.Current) {
        throw new Ice.TimeoutException();
    }

    throwLocalExceptionIdempotent(_: Ice.Current) {
        throw new Ice.TimeoutException();
    }

    throwNonIceException(_: Ice.Current) {
        throw new Error();
    }

    throwAssertException(_: Ice.Current) {
        test(false);
    }

    throwMemoryLimitException(_seq: Uint8Array, _: Ice.Current) {
        return new Uint8Array(1024 * 20); // 20KB is over the configured 10KB message size max.
    }

    throwDispatchException(replyStatus: number, _: Ice.Current) {
        // valueOf creates a new ReplyStatus enumerator if one doesn't exist already.
        throw new Ice.DispatchException(Ice.ReplyStatus.valueOf(replyStatus));
    }

    throwAfterResponse(_: Ice.Current) {}

    throwAfterException(_: Ice.Current) {
        throw new Test.A();
    }
}
