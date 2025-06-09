// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

struct ServantLocatorI: Ice.ServantLocator {
    private let _dispatcher: Ice.Dispatcher = DispatcherI()

    func locate(_: Ice.Current) -> (returnValue: Ice.Dispatcher?, cookie: AnyObject?) {
        return (_dispatcher, nil)
    }

    func finished(curr _: Ice.Current, servant _: Ice.Dispatcher, cookie _: AnyObject?) {}

    func deactivate(_: String) {}
}

struct DispatcherI: Ice.Dispatcher {
    public func dispatch(_ request: sending IncomingRequest) throws -> OutgoingResponse {
        let current = request.current
        let communicator = current.adapter.getCommunicator()
        let (inEncaps, _) = try request.inputStream.readEncapsulation()
        let inS = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try inS.startEncapsulation()

        if current.operation == "opOneway" {
            return request.current.makeEmptyOutgoingResponse()
        } else if current.operation == "opString" {
            let s: String = try inS.read()
            return request.current.makeOutgoingResponse(
                s, formatType: nil
            ) { ostr, s in
                ostr.write(s)
                ostr.write(s)
            }
        } else if current.operation == "opException" {
            if current.ctx["raise"] != nil {
                throw MyException()
            }
            let ex = MyException()
            return request.current.makeOutgoingResponse(error: ex)
        } else if current.operation == "shutdown" {
            communicator.shutdown()
            return request.current.makeEmptyOutgoingResponse()
        } else if current.operation == "ice_isA" {
            let s: String = try inS.read()
            return request.current.makeOutgoingResponse(
                s, formatType: nil
            ) { ostr, s in
                if s == "::Test::MyClass" {
                    ostr.write(true)
                } else {
                    ostr.write(false)
                }
            }
        } else {
            throw Ice.OperationNotExistException()
        }
    }
}
