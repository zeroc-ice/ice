// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

final class TestI: TestIntf {
    func requestFailedException(current _: Current) async throws {}

    func unknownUserException(current _: Current) async throws {}

    func unknownLocalException(current _: Current) async throws {}

    func unknownException(current _: Current) async throws {}

    func localException(current _: Current) async throws {}

    func userException(current _: Current) async throws {}

    func unknownExceptionWithServantException(current: Current) async throws {
        throw ObjectNotExistException()
    }

    func impossibleException(shouldThrow: Bool, current _: Current) async throws -> String {
        if shouldThrow {
            throw TestImpossibleException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func intfUserException(shouldThrow: Bool, current _: Current) async throws -> String {
        if shouldThrow {
            throw TestIntfUserException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func shutdown(current: Current) async throws {
        current.adapter.deactivate()
    }
}

final class TestActivationI: TestActivation {
    let _helper: TestHelper

    init(_ helper: TestHelper) {
        _helper = helper
    }

    func activateServantLocator(activate: Bool, current: Current) async throws {
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
