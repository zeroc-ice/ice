//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class TestI: TestIntf {
    func requestFailedException(current _: Current) throws {}

    func unknownUserException(current _: Current) throws {}

    func unknownLocalException(current _: Current) throws {}

    func unknownException(current _: Current) throws {}

    func localException(current _: Current) throws {}

    func userException(current _: Current) throws {}

    func unknownExceptionWithServantException(current: Current) throws {
        throw ObjectNotExistException(id: current.id,
                                      facet: current.facet,
                                      operation: current.operation)
    }

    func impossibleException(throw t: Bool, current _: Current) throws -> String {
        if t {
            throw TestImpossibleException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func intfUserException(throw t: Bool, current _: Current) throws -> String {
        if t {
            throw TestIntfUserException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func asyncResponse(current _: Current) throws {
        //
        // Only relevant for AMD.
        //
    }

    func asyncException(current _: Current) throws {
        //
        // Only relevant for AMD.
        //
    }

    func shutdown(current: Current) throws {
        current.adapter!.deactivate()
    }
}

class TestActivationI: TestActivation {
    var _helper: TestHelper

    init(_ helper: TestHelper) {
        _helper = helper
    }

    func activateServantLocator(activate: Bool, current: Current) throws {
        if activate {
            try current.adapter!.addServantLocator(locator: ServantLocatorI("", _helper), category: "")
            try current.adapter!.addServantLocator(locator: ServantLocatorI("category", _helper), category: "category")
        } else {
            var locator = try current.adapter!.removeServantLocator("")
            locator.deactivate("")
            locator = try current.adapter!.removeServantLocator("category")
            locator.deactivate("category")
        }
    }
}
