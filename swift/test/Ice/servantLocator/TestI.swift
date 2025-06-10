// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

final class TestI: TestIntf {
    func requestFailedException(current _: Current) {}

    func unknownUserException(current _: Current) {}

    func unknownLocalException(current _: Current) {}

    func unknownException(current _: Current) {}

    func localException(current _: Current) {}

    func userException(current _: Current) {}

    func unknownExceptionWithServantException(current: Current) throws {
        throw ObjectNotExistException()
    }

    func impossibleException(shouldThrow: Bool, current _: Current) throws -> String {
        if shouldThrow {
            throw TestImpossibleException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func intfUserException(shouldThrow: Bool, current _: Current) throws -> String {
        if shouldThrow {
            throw TestIntfUserException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func shutdown(current: Current) {
        current.adapter.deactivate()
    }
}

final class TestActivationI: TestActivation {
    let _helper: TestHelper

    init(_ helper: TestHelper) {
        _helper = helper
    }

    func activateServantLocator(activate: Bool, current: Current) throws {
        if activate {
            try current.adapter.addServantLocator(locator: ServantLocatorI("", _helper), category: "")
            try current.adapter.addServantLocator(
                locator: ServantLocatorI("category", _helper), category: "category")
        } else {
            var locator = try current.adapter.removeServantLocator("")
            locator.deactivate("")
            locator = try current.adapter.removeServantLocator("category")
            locator.deactivate("category")
        }
    }
}
