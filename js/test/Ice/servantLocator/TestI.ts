// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";

export class TestI extends Test.TestIntf {
    override requestFailedException(current: Ice.Current) {}

    override unknownUserException(current: Ice.Current) {}

    override unknownLocalException(current: Ice.Current) {}

    override unknownException(current: Ice.Current) {}

    override localException(current: Ice.Current) {}

    override userException(current: Ice.Current) {}

    override jsException(current: Ice.Current) {}

    override unknownExceptionWithServantException(current: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    override impossibleException(shouldThrow: boolean, current: Ice.Current): string {
        if (shouldThrow) {
            throw new Test.TestImpossibleException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    override intfUserException(shouldThrow: boolean, current: Ice.Current): string {
        if (shouldThrow) {
            throw new Test.TestIntfUserException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    override asyncResponse(current: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    asyncException(current: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    override shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}
