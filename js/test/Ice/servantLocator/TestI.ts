// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";

export class TestI extends Test.TestIntf {
    requestFailedException(current: Ice.Current) {}

    unknownUserException(current: Ice.Current) {}

    unknownLocalException(current: Ice.Current) {}

    unknownException(current: Ice.Current) {}

    localException(current: Ice.Current) {}

    userException(current: Ice.Current) {}

    jsException(current: Ice.Current) {}

    unknownExceptionWithServantException(current: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    impossibleException(shouldThrow: boolean, current: Ice.Current): string {
        if (shouldThrow) {
            throw new Test.TestImpossibleException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    intfUserException(shouldThrow: boolean, current: Ice.Current): string {
        if (shouldThrow) {
            throw new Test.TestIntfUserException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    asyncResponse(current: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    asyncException(current: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}
