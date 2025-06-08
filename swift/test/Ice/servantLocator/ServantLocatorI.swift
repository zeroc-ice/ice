// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import Synchronization
import TestCommon

class Cookie {
    func message() -> String {
        return "blahblah"
    }
}

final class ServantLocatorI: Ice.ServantLocator {
    private let _deactivated = Mutex<Bool>(false)
    private let _category: String
    private var _requestId: Int32
    private let _helper: TestHelper

    init(_ category: String, _ helper: TestHelper) {
        _category = category
        _requestId = -1
        _helper = helper
    }

    deinit {
        _deactivated.withLock {
            precondition($0, "ServantLocatorI deactivated must be true before deinit")
        }
    }

    func locate(_ curr: Ice.Current) throws -> (returnValue: Ice.Dispatcher?, cookie: AnyObject?) {
        try _deactivated.withLock {
            try _helper.test(!$0)
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

        //
        // Ensure locate() is only called once per request.
        //
        try _helper.test(_requestId == -1)
        _requestId = curr.requestId

        return (TestI(), Cookie())
    }

    func finished(curr: Ice.Current, servant _: Ice.Dispatcher, cookie: AnyObject?) throws {
        try _deactivated.withLock {
            try _helper.test(!$0)
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

    func deactivate(_: String) {
        _deactivated.withLock {
            precondition(!$0)
            $0 = true
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
