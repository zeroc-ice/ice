//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import IceImpl
import PromiseKit

public final class Incoming {
    private let current: Current
    private var format: FormatType
    private let istr: InputStream

    // sendResponse must be called exactly once. It's likely the current implementation does not guarantee it.
    private let sendResponse: ICESendResponse

    private var servant: Disp?
    private var locator: ServantLocator?
    private var cookie: AnyObject?

    private var ostr: OutputStream!  // must be set before calling sendResponse
    private var ok: Bool  // false if response contains a UserException

    init(
        istr: InputStream, sendResponse: @escaping ICESendResponse,
        current: Current
    ) {
        self.istr = istr
        format = .DefaultFormat
        ok = true
        self.sendResponse = sendResponse
        self.current = current
    }

    public func readEmptyParams() throws {
        let encoding = try istr.startEncapsulation()
        assert(encoding == current.encoding)
    }

    public func readParamEncaps() throws -> Data {
        let params = try istr.readEncapsulation()
        assert(params.encoding == current.encoding)
        return params.bytes
    }

    public func read<T>(_ cb: (InputStream) throws -> T) throws -> T {
        let encoding = try istr.startEncapsulation()
        assert(encoding == current.encoding)
        let l = try cb(istr)
        try istr.endEncapsulation()
        return l
    }

    public func startOver() {
        istr.startOver()
        ostr = nil
    }

    public func writeParamEncaps(ok: Bool, outParams: Data) -> OutputStream {
        let ostr = OutputStream(communicator: istr.communicator, encoding: current.encoding)
        if outParams.isEmpty {
            ostr.writeEmptyEncapsulation(current.encoding)
        } else {
            ostr.writeEncapsulation(outParams)
        }
        self.ok = ok

        return ostr
    }

    public func response() {
        guard locator == nil || servantLocatorFinished() else {
            return
        }
        precondition(ostr != nil, "OutputStream was not set before calling response()")
        ostr.finished().withUnsafeBytes {
            sendResponse(ok, $0.baseAddress!, $0.count, nil)
        }
    }

    public func exception(_ ex: Error) {
        guard locator == nil || servantLocatorFinished() else {
            return
        }
        handleException(ex)
    }

    public func setFormat(_ format: FormatType) {
        self.format = format
    }

    @discardableResult
    public func setResult(_ os: OutputStream) -> Promise<OutputStream>? {
        ostr = os
        return nil  // Response is cached in the Incoming to not have to create unnecessary promise
    }

    public func setResult() -> Promise<OutputStream>? {
        let ostr = OutputStream(communicator: istr.communicator)
        ostr.writeEmptyEncapsulation(current.encoding)
        self.ostr = ostr
        return nil  // Response is cached in the Incoming to not have to create unnecessary future
    }

    public func setResult(_ cb: (OutputStream) -> Void) -> Promise<OutputStream>? {
        let ostr = OutputStream(communicator: istr.communicator)
        ostr.startEncapsulation(encoding: current.encoding, format: format)
        cb(ostr)
        ostr.endEncapsulation()
        self.ostr = ostr
        return nil  // Response is cached in the Incoming to not have to create unnecessary future
    }

    public func setResultPromise(_ p: Promise<Void>) -> Promise<OutputStream> {
        // Use the thread which fulfilled the promise (on: nil)
        return p.map(on: nil) {
            let ostr = OutputStream(communicator: self.istr.communicator)
            ostr.writeEmptyEncapsulation(self.current.encoding)
            return ostr
        }
    }

    public func setResultPromise<T>(
        _ p: Promise<T>,
        _ cb: @escaping (OutputStream, T) -> Void
    ) -> Promise<OutputStream> {
        // Use the thread which fulfilled the promise (on: nil)
        return p.map(on: nil) { t in
            let ostr = OutputStream(communicator: self.istr.communicator)
            ostr.startEncapsulation(encoding: self.current.encoding, format: self.format)
            cb(ostr, t)
            ostr.endEncapsulation()
            return ostr
        }
    }

    func servantLocatorFinished() -> Bool {
        guard let locator = locator, let servant = servant else {
            preconditionFailure()
        }

        do {
            try locator.finished(curr: current, servant: servant, cookie: cookie)
            return true
        } catch {
            handleException(error)
        }

        return false
    }

    func invoke(_ servantManager: ServantManager) {
        servant = servantManager.findServant(id: current.id, facet: current.facet)

        if servant == nil {
            locator = servantManager.findServantLocator(category: current.id.category)

            if locator == nil, !current.id.category.isEmpty {
                locator = servantManager.findServantLocator(category: "")
            }

            if let locator = locator {
                do {
                    let locatorReturn = try locator.locate(current)
                    (servant, cookie) = (locatorReturn.returnValue, locatorReturn.cookie)
                } catch {
                    handleException(error)
                    return
                }
            }
        }

        guard let s = servant else {
            do {
                if servantManager.hasServant(id: current.id) || servantManager.isAdminId(current.id) {
                    throw FacetNotExistException(
                        id: current.id, facet: current.facet, operation: current.operation)
                } else {
                    throw ObjectNotExistException(
                        id: current.id, facet: current.facet, operation: current.operation)
                }
            } catch {
                sendResponse(false, nil, 0, convertIntoDispatchException(error))
                return
            }
        }

        //
        // Dispatch in the incoming call
        //
        do {
            // Request was dispatched asynchronously if promise is non-nil
            if let promise = try s.dispatch(request: self, current: current) {
                // Use the thread which fulfilled the promise (on: nil)
                promise.done(on: nil) { ostr in
                    self.ostr = ostr
                    self.response()
                }.catch(on: nil) { error in
                    self.exception(error)
                }
            } else {
                response()
            }
        } catch {
            exception(error)
        }
    }

    func handleException(_ exception: Error) {
        guard let e = exception as? UserException else {
            sendResponse(false, nil, 0, convertIntoDispatchException(exception))
            return
        }
        ok = false  // response will contain a UserException
        let ostr = OutputStream(communicator: istr.communicator)
        ostr.startEncapsulation(encoding: current.encoding, format: format)
        ostr.write(e)
        ostr.endEncapsulation()
        ostr.finished().withUnsafeBytes {
            sendResponse(ok, $0.baseAddress!, $0.count, nil)
        }
    }

    // This code is temporary: we should give a fully marshaled response back to the Objective-C++ code.
    func convertIntoDispatchException(_ exception: Error) -> ICEDispatchException {
        switch exception {
        // OperationNotExistException and friends
        case let e as ObjectNotExistException:
            ICEDispatchException.objectNotExistException(
                e.id.name, category: e.id.category, facet: e.facet, operation: e.operation, file: e.file,
                line: e.line)
        case let e as FacetNotExistException:
            ICEDispatchException.facetNotExistException(
                e.id.name, category: e.id.category, facet: e.facet, operation: e.operation, file: e.file,
                line: e.line)
        case let e as OperationNotExistException:
            ICEDispatchException.operationNotExistException(
                e.id.name, category: e.id.category, facet: e.facet, operation: e.operation, file: e.file,
                line: e.line)
        // Unknown exceptions
        case let e as UnknownUserException:
            ICEDispatchException.unknownUserException(e.unknown, file: e.file, line: e.line)
        case let e as UnknownLocalException:
            ICEDispatchException.unknownLocalException(e.unknown, file: e.file, line: e.line)
        case let e as UnknownException:
            ICEDispatchException.unknownException(e.unknown, file: e.file, line: e.line)
        // Other exceptions mapped to Unknown exceptions
        case let e as LocalException:
            ICEDispatchException.unknownLocalException("\(e)", file: e.file, line: e.line)
        case let e as UserException:
            ICEDispatchException.unknownUserException("\(e.ice_id())", file: #file, line: #line)
        default:
            ICEDispatchException.unknownException("\(exception)", file: #file, line: #line)
        }
    }
}
