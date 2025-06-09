// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import Synchronization
import TestCommon

final class ServantLocatorI: Ice.ServantLocator {
    private class Cookie {
        func message() -> String {
            return "blahblah"
        }
    }

    private struct State {
        var deactivated = false
        var requestId: Int32 = -1
    }

    private let _category: String
    private let _helper: TestHelper
    private let _state = Mutex<State>(State())

    init(_ category: String, _ helper: TestHelper) {
        _category = category
        _helper = helper
    }

    deinit {
        _state.withLock {
            precondition($0.deactivated, "ServantLocatorI deactivated must be true before deinit")
        }
    }

    func locate(_ curr: Ice.Current) throws -> (returnValue: Ice.Dispatcher?, cookie: AnyObject?) {
        try _state.withLock {
            try _helper.test(!$0.deactivated)
        }

        try _helper.test(curr.id.category == _category || _category == "")

        if curr.id.name == "unknown" {
            return (nil, nil)
        }

        if curr.id.name == "invalidReturnValue" || curr.id.name == "invalidReturnType" {
            return (nil, nil)
        }

        try _helper.test(curr.id.name == "locate" || curr.id.name == "finished")

        if curr.id.name == "locate" {
            try exception(curr)
        }

        // Ensure locate() is only called once per request.
        try _state.withLock {
            try _helper.test($0.requestId == -1)
            $0.requestId = curr.requestId
        }

        return (TestI(), Cookie())
    }

    func finished(curr: Ice.Current, servant _: Ice.Dispatcher, cookie: AnyObject?) throws {
        try _state.withLock {
            try _helper.test(!$0.deactivated)

            // Ensure finished() is only called once per request.
            try _helper.test($0.requestId == curr.requestId)
            $0.requestId = -1
        }

        try _helper.test(curr.id.category == _category || _category == "")
        try _helper.test(curr.id.name == "locate" || curr.id.name == "finished")

        if curr.id.name == "finished" {
            try exception(curr)
        }

        try _helper.test((cookie as! Cookie).message() == "blahblah")
    }

    func deactivate(_: String) {
        _state.withLock {
            precondition(!$0.deactivated)
            $0.deactivated = true
        }
    }

    func exception(_ current: Ice.Current) throws {
        if current.operation == "ice_ids" {
            throw TestIntfUserException()
        } else if current.operation == "requestFailedException" {
            throw Ice.ObjectNotExistException()
        } else if current.operation == "unknownUserException" {
            throw Ice.UnknownUserException(badTypeId: "::Foo::BarException")
        } else if current.operation == "unknownLocalException" {
            throw Ice.UnknownLocalException("reason")
        } else if current.operation == "unknownException" {
            throw Ice.UnknownException("reason")
        } else if current.operation == "userException" {
            throw TestIntfUserException()
        } else if current.operation == "localException" {
            throw Ice.SocketException("socket error")
        } else if current.operation == "unknownExceptionWithServantException" {
            throw Ice.UnknownException("reason")
        } else if current.operation == "impossibleException" {
            throw TestIntfUserException()  // Yes, it really is meant to be TestIntfException.
        } else if current.operation == "intfUserException" {
            throw TestImpossibleException()  // Yes, it really is meant to be TestImpossibleException.
        }
    }
}
