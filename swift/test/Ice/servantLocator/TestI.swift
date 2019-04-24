//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Foundation

class CookieI: Cookie {
    func message() -> String{
        return "blahblah"
    }
}

class TestI: TestIntf {
    func requestFailedException(current: Current) throws {}

    func unknownUserException(current: Current) throws {}

    func unknownLocalException(current: Current) throws {}

    func unknownException(current: Current) throws {}

    func localException(current: Current) throws {}

    func userException(current: Current) throws {}

    func stdException(current: Current) throws {}

    func cppException(current: Current) throws {}

    func unknownExceptionWithServantException(current: Current) throws {
        throw ObjectNotExistException(id: current.id,
                                      facet: current.facet,
                                      operation: current.operation)
    }

    func impossibleException(throw t: Bool, current: Current) throws -> String {
        if t {
            throw TestImpossibleException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func intfUserException(throw t: Bool, current: Current) throws -> String {
        if t {
            throw TestIntfUserException()
        }
        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello"
    }

    func asyncResponse(current: Current) throws {
        //
        // Only relevant for AMD.
        //
    }

    func asyncException(current: Current) throws {
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

class ServantLocatorI: Ice.ServantLocator {
    var _deactivated: Bool
    var _category: String
    var _requestId: Int32
    var _helper: TestHelper
    var _lock = os_unfair_lock()

    init(_ category: String, _ helper: TestHelper) {
        _category = category
        _deactivated = false
        _requestId = -1
        _helper = helper
    }

    deinit {
        withLock(&_lock) {
            precondition(_deactivated)
        }
    }

    func locate(_ curr: Ice.Current) throws -> (returnValue: Ice.Object?, cookie: AnyObject?) {
        try withLock(&_lock) {
            try _helper.test(!_deactivated)
        }

        try _helper.test(curr.id.category == _category || _category == "")

        if curr.id.name == "unknown" {
            return (nil, nil)
        }

        try _helper.test(curr.id.name == "locate" || curr.id.name == "finished")

        if curr.id.name == "locate" {
            try exception(curr)
        }

        //
        // Ensure locate() is only called once per request.
        //
        try _helper.test(_requestId == -1)
        _requestId = curr.requestId

        return (TestI(), CookieI())
    }

    func finished(curr: Ice.Current, servant: Ice.Object, cookie: AnyObject?) throws {
        try withLock(&_lock) {
            try _helper.test(!_deactivated)
        }

        //
        // Ensure finished() is only called once per request.
        //
        try _helper.test(_requestId == curr.requestId)
        _requestId = -1

        try _helper.test(curr.id.category == _category || _category == "")
        try _helper.test(curr.id.name == "locate" || curr.id.name == "finished")

        if curr.id.name == "finished" {
            try exception(curr)
        }

        try _helper.test((cookie as! Cookie).message() == "blahblah")
    }

    func deactivate(_ category: String) {
        withLock(&_lock) {
            precondition(!_deactivated)
            self._deactivated = true
        }
    }

    func exception(_ current: Ice.Current) throws {
        if current.operation == "ice_ids" {
            throw TestIntfUserException()
        } else if current.operation == "requestFailedException" {
            throw Ice.ObjectNotExistException(id: current.id, facet: current.facet, operation: current.operation)
        } else if current.operation == "unknownUserException" {
            throw Ice.UnknownUserException(unknown: "reason")
        } else if current.operation == "unknownLocalException" {
            throw Ice.UnknownLocalException(unknown: "reason")
        } else if current.operation == "unknownException" {
            throw Ice.UnknownException(unknown: "reason")
        } else if current.operation == "userException" {
            throw TestIntfUserException()
        } else if current.operation == "localException" {
            throw Ice.SocketException(error: 0)
        } else if current.operation == "csException" {
            throw Ice.RuntimeError("message")
        } else if current.operation == "unknownExceptionWithServantException" {
            throw Ice.UnknownException(unknown: "reason")
        } else if current.operation == "impossibleException" {
            throw TestIntfUserException() // Yes, it really is meant to be TestIntfException.
        } else if current.operation == "intfUserException" {
            throw TestImpossibleException() // Yes, it really is meant to be TestImpossibleException.
        } else if current.operation == "asyncResponse" {
            throw TestImpossibleException()
        } else if current.operation == "asyncException" {
            throw TestImpossibleException()
        }
    }
}
