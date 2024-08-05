// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

class ServantLocatorI: Ice.ServantLocator {
    let _dispatcher: Ice.Dispatcher = DispatcherI()

    func locate(_: Ice.Current) -> (returnValue: Ice.Dispatcher?, cookie: AnyObject?) {
        return (_dispatcher, nil)
    }

    func finished(curr _: Ice.Current, servant _: Ice.Dispatcher, cookie _: AnyObject?) {}

    func deactivate(_: String) {}
}

class DispatcherI: Ice.Dispatcher {
    public func dispatch(_ request: IncomingRequest) async throws -> OutgoingResponse {
        let current = request.current
        let communicator = current.adapter.getCommunicator()
        let (inEncaps, _) = try request.inputStream.readEncapsulation()
        let inS = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try inS.startEncapsulation()
        let outS = Ice.OutputStream(communicator: communicator)
        outS.startEncapsulation()
        if current.operation == "opOneway" {
            return request.current.makeOutgoingResponse(ok: true, encapsulation: Data())
        } else if current.operation == "opString" {
            let s: String = try inS.read()
            outS.write(s)
            outS.write(s)
            outS.endEncapsulation()
            return request.current.makeOutgoingResponse(ok: true, encapsulation: outS.finished())
        } else if current.operation == "opException" {
            if current.ctx["raise"] != nil {
                throw MyException()
            }
            let ex = MyException()
            outS.write(ex)
            outS.endEncapsulation()
            return request.current.makeOutgoingResponse(ok: false, encapsulation: outS.finished())
        } else if current.operation == "shutdown" {
            communicator.shutdown()
            return request.current.makeOutgoingResponse(ok: true, encapsulation: Data())
        } else if current.operation == "ice_isA" {
            let s: String = try inS.read()
            if s == "::Test::MyClass" {
                outS.write(true)
            } else {
                outS.write(false)
            }
            outS.endEncapsulation()
            return request.current.makeOutgoingResponse(ok: true, encapsulation: outS.finished())
        } else {
            throw Ice.OperationNotExistException(
                id: current.id,
                facet: current.facet,
                operation: current.operation)
        }
    }
}
