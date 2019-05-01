//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

class BlobjectI: Ice.Blobject {

    func ice_invoke(inEncaps: Data, current: Ice.Current) throws -> (ok: Bool, outParams: Data) {
        let communicator = current.adapter!.getCommunicator()
        let inS = Ice.InputStream(communicator: communicator, bytes: inEncaps)
        _ = try inS.startEncapsulation()
        let outS = Ice.OutputStream(communicator: communicator)
        outS.startEncapsulation()
        if current.operation == "opOneway" {
            return (true, Data())
        } else if current.operation == "opString" {
            let s: String = try inS.read()
            outS.write(s)
            outS.write(s)
            outS.endEncapsulation()
            return (true, outS.finished())
        } else if current.operation == "opException" {
            if current.ctx["raise"] != nil {
                throw MyException()
            }
            let ex = MyException()
            outS.write(ex)
            outS.endEncapsulation()
            return (false, outS.finished())
        } else if current.operation == "shutdown" {
            communicator.shutdown()
            return (true, Data())
        } else if current.operation == "ice_isA" {
            let s: String = try inS.read()
            if s == "::Test::MyClass" {
                outS.write(true)
            } else {
                outS.write(false)
            }
            outS.endEncapsulation()
            return (true, outS.finished())
        } else {
            throw Ice.OperationNotExistException(id: current.id, facet: current.facet, operation: current.operation)
        }
    }
}

class BlobjectAsyncI: Ice.BlobjectAsync {
    func ice_invokeAsync(inEncaps: Data, current: Current) -> Promise<(ok: Bool, outParams: Data)> {
        do {
            let communicator = current.adapter!.getCommunicator()
            let inS = Ice.InputStream(communicator: communicator, bytes: inEncaps)
            _ = try inS.startEncapsulation()
            let outS = Ice.OutputStream(communicator: communicator)
            outS.startEncapsulation()
            if current.operation == "opOneway" {
                return Promise.value((true, Data()))
            } else if current.operation == "opString" {
                let s: String = try inS.read()
                outS.write(s)
                outS.write(s)
                outS.endEncapsulation()
                return Promise.value((true, outS.finished()))
            } else if current.operation == "opException" {
                let ex = MyException()
                outS.write(ex)
                outS.endEncapsulation()
                return Promise.value((false, outS.finished()))
            } else if current.operation == "shutdown" {
                communicator.shutdown()
                return Promise.value((false, Data()))
            } else if current.operation == "ice_isA" {
                let s: String = try inS.read()
                if s == "::Test::MyClass" {
                    outS.write(true)
                } else {
                    outS.write(false)
                }
                outS.endEncapsulation()
                return Promise.value((true, outS.finished()))
            } else {
                throw Ice.OperationNotExistException(id: current.id,
                                                     facet: current.facet,
                                                     operation: current.operation)
            }
        } catch {
            return Promise<(ok: Bool, outParams: Data)> { seal in
                seal.reject(error)
            }
        }
    }
}
