// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

typealias IceObjcResponse = (Bool, UnsafeRawPointer?, Int) -> Void
typealias IceObjcException = (ICERuntimeException) -> Void

public final class Incoming {
    var current: Current
    var format: FormatType
    var istr: InputStream
    var ostr: OutputStream
    var responseCallback: IceObjcResponse
    var exceptionCallback: IceObjcException

    var servant: Object?
    var locator: ServantLocator?
    var cookie: AnyObject!

    init(istr: InputStream, response: @escaping IceObjcResponse, exception: @escaping IceObjcException,
         current: Current) {
        self.istr = istr
        format = .DefaultFormat
        responseCallback = response
        exceptionCallback = exception
        self.current = current
        ostr = OutputStream(communicator: istr.getCommunicator(), encoding: current.encoding)
    }

    public func readEmptyParams() throws {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        current.encoding = try istr.skipEmptyEncapsulation()
    }

    public func read<T>(_ cb: (InputStream) throws -> T) throws -> T {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        current.encoding = try istr.startEncapsulation()
        let l = try cb(istr)
        try istr.endEncapsulation()
        return l
    }

    public func write(_ cb: (OutputStream) -> Void) {
        ostr.startEncapsulation(encoding: current.encoding, format: format)
        cb(ostr)
        ostr.endEncapsulation()
        response()
    }

    public func writeEmptyParams() {
        ostr.writeEmptyEncapsulation(current.encoding)
        response()
    }

    public func response() {
        guard locator == nil || servantLocatorFinished() else {
            return
        }
        responseCallback(true, ostr.getConstBytes(), ostr.getCount())
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

    func servantLocatorFinished() -> Bool {
        guard let locator = locator, let servant = servant else {
            preconditionFailure()
        }

        do {
            try locator.finished(curr: current, servant: servant, cookie: cookie)
            return true
        } catch let e {
            exceptionCallback(convertException(e))
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
                } catch let err {
                    exceptionCallback(convertException(err))
                    return
                }
            }
        }

        guard let s = servant else {
            do {
                if servantManager.hasServant(id: current.id) {
                    throw FacetNotExistException(id: current.id, facet: current.facet, operation: current.operation)
                } else {
                    throw ObjectNotExistException(id: current.id, facet: current.facet, operation: current.operation)
                }
            } catch let err {
                exceptionCallback(convertException(err))
                return
            }
        }

        //
        // Dispatch in the incoming call
        //
        do {
            try s.iceDispatch(incoming: self, current: current)
        } catch {
            exception(error)
        }
    }

    func handleException(_ exception: Error) {
        guard let e = exception as? UserException else {
            exceptionCallback(convertException(exception))
            return
        }
        ostr = OutputStream(communicator: istr.getCommunicator(), encoding: current.encoding)
        ostr.startEncapsulation(encoding: current.encoding, format: .DefaultFormat)
        ostr.write(e)
        ostr.endEncapsulation()
        responseCallback(false, ostr.getBytes()!, ostr.getCount())
    }

    func convertException(_ exception: Error) -> ICERuntimeException {
        switch exception {
        case let exception as ObjectNotExistException:
            let e = ICEObjectNotExistException()
            e.file = exception.file
            e.line = Int32(exception.line)
            e.name = exception.id.name
            e.category = exception.id.category
            e.facet = exception.facet
            e.operation = exception.operation
            return e
        case let exception as FacetNotExistException:
            let e = ICEFacetNotExistException()
            e.file = exception.file
            e.line = Int32(exception.line)
            e.name = exception.id.name
            e.category = exception.id.category
            e.facet = exception.facet
            e.operation = exception.operation
            return e
        case let exception as OperationNotExistException:
            let e = ICEOperationNotExistException()
            e.file = exception.file
            e.line = Int32(exception.line)
            e.name = exception.id.name
            e.category = exception.id.category
            e.facet = exception.facet
            e.operation = exception.operation
            return e
        case let exception as UnknownUserException:
            let e = ICEUnknownUserException()
            e.file = exception.file
            e.line = Int32(exception.line)
            e.unknown = exception.unknown
            return e
        case let exception as UnknownLocalException:
            let e = ICEUnknownLocalException()
            e.file = exception.file
            e.line = Int32(exception.line)
            e.unknown = exception.unknown
            return e
        case let exception as UnknownException:
            let e = ICEUnknownException()
            e.file = exception.file
            e.line = Int32(exception.line)
            e.unknown = exception.unknown
            return e
        default:
            let e = ICEUnknownException()
            e.file = #file
            e.line = Int32(#line)
            e.unknown = "\(exception)"
            return e
        }
    }
}
