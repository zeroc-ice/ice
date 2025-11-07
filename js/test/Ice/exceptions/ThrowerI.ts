// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { test } from "../../Common/TestHelper.js";

export class ThrowerI extends Test.Thrower {
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
        this.throwBasB(a, b, current);
    }

    throwBasB(a: number, b: number, _: Ice.Current) {
        throw new Test.B(a, b);
    }

    throwCasA(a: number, b: number, c: number, current: Ice.Current) {
        this.throwCasC(a, b, c, current);
    }

    throwCasB(a: number, b: number, c: number, current: Ice.Current) {
        this.throwCasC(a, b, c, current);
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

    throwDispatchException(replyStatusValue: number, _: Ice.Current) {
        const replyStatus =
            Ice.ReplyStatus.valueOf(replyStatusValue) || new Ice.ReplyStatus(`${replyStatusValue}`, replyStatusValue);
        throw new Ice.DispatchException(replyStatus);
    }

    throwAfterResponse(_: Ice.Current) {
        //
        // Only relevant for AMD.
        //
    }

    throwAfterException(_: Ice.Current) {
        //
        // Only relevant for AMD.
        //
        throw new Test.A();
    }
}
