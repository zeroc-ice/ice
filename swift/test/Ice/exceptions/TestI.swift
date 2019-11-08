//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

open class G: Ice.LocalException {
    public var data: Swift.String = ""

    public required init() {
        super.init()
    }

    public init(data: Swift.String, file: Swift.String = #file, line: Swift.Int = #line) {
        self.data = data
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `Swift.String` - the Slice type ID of this exception.
    open override class func ice_staticId() -> Swift.String {
        return "::Test::G"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `Swift.String` - The string representaton of this exception.
    open override func ice_print() -> Swift.String {
        return _GDescription
    }
}

open class H: Ice.LocalException {
 public var data: Swift.String = ""

 public required init() {
     super.init()
 }

 public init(data: Swift.String, file: Swift.String = #file, line: Swift.Int = #line) {
     self.data = data
     super.init(file: file, line: line)
 }

 /// Returns the Slice type ID of this exception.
 ///
 /// - returns: `Swift.String` - the Slice type ID of this exception.
 open override class func ice_staticId() -> Swift.String {
     return "::Test::H"
 }

 /// Returns a string representation of this exception
 ///
 /// - returns: `Swift.String` - The string representaton of this exception.
 open override func ice_print() -> Swift.String {
     return _HDescription
    }
}

class RuntimeError: Error {}

class ThrowerI: Thrower {
    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }

    func supportsUndeclaredExceptions(current _: Current) throws -> Bool {
        return true
    }

    func supportsAssertException(current _: Current) throws -> Bool {
        return false
    }

    func throwAasA(a: Int32, current _: Ice.Current) throws {
        throw A(aMem: a)
    }

    func throwAorDasAorD(a: Int32, current _: Ice.Current) throws {
        if a > 0 {
            throw A(aMem: a)
        } else {
            throw D(dMem: a)
        }
    }

    func throwBasA(a: Int32, b: Int32, current: Ice.Current) throws {
        try throwBasB(a: a, b: b, current: current)
    }

    func throwBasB(a: Int32, b: Int32, current _: Ice.Current) throws {
        let ex = B()
        ex.aMem = a
        ex.bMem = b
        throw ex
    }

    func throwCasA(a: Int32, b: Int32, c: Int32, current: Ice.Current) throws {
        try throwCasC(a: a, b: b, c: c, current: current)
    }

    func throwCasB(a: Int32, b: Int32, c: Int32, current: Ice.Current) throws {
        try throwCasC(a: a, b: b, c: c, current: current)
    }

    func throwCasC(a: Int32, b: Int32, c: Int32, current _: Ice.Current) throws {
        throw C(aMem: a, bMem: b, cMem: c)
    }

    func throwModA(a: Int32, a2: Int32, current _: Current) throws {
        throw ModA(aMem: a, a2Mem: a2)
    }

    func throwLocalException(current _: Ice.Current) throws {
        throw Ice.TimeoutException()
    }

    func throwNonIceException(current _: Ice.Current) throws {
        throw RuntimeError()
    }

    func throwAssertException(current _: Ice.Current) throws {}

    func throwMemoryLimitException(seq _: ByteSeq, current _: Ice.Current) throws -> ByteSeq {
        return ByteSeq(repeating: 0, count: 1024 * 20) // 20KB is over the configured 10KB message size max.
    }

    func throwLocalExceptionIdempotent(current _: Ice.Current) throws {
        throw Ice.TimeoutException()
    }

    func throwUndeclaredA(a: Int32, current _: Ice.Current) throws {
        throw A(aMem: a)
    }

    func throwUndeclaredB(a: Int32, b: Int32, current _: Ice.Current) throws {
        throw B(aMem: a, bMem: b)
    }

    func throwUndeclaredC(a: Int32, b: Int32, c: Int32, current _: Ice.Current) throws {
        throw C(aMem: a, bMem: b, cMem: c)
    }

    func throwAfterResponse(current _: Ice.Current) throws {
        //
        // Only relevant for AMD.
        //
    }

    func throwAfterException(current _: Ice.Current) throws {
        //
        // Only relevant for AMD.
        //
        throw A()
    }
}
