// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";

export class TestI extends Test.TestIntf {
    override requestFailedException(_: Ice.Current) {}

    override unknownUserException(_: Ice.Current) {}

    override unknownLocalException(_: Ice.Current) {}

    override unknownException(_: Ice.Current) {}

    override localException(_: Ice.Current) {}

    override userException(_: Ice.Current) {}

    override jsException(_: Ice.Current) {}

    override unknownExceptionWithServantException(_: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    override impossibleException(shouldThrow: boolean, _: Ice.Current): string {
        if (shouldThrow) {
            throw new Test.TestImpossibleException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    override intfUserException(shouldThrow: boolean, _: Ice.Current): string {
        if (shouldThrow) {
            throw new Test.TestIntfUserException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    override asyncResponse(_: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    asyncException(_: Ice.Current) {
        throw new Ice.ObjectNotExistException();
    }

    override shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}
