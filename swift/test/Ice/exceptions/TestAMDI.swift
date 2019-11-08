//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
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
    func shutdownAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            current.adapter!.getCommunicator().shutdown()
            seal.fulfill(())
        }
    }

    func supportsUndeclaredExceptionsAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(true)
    }

    func supportsAssertExceptionAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(false)
    }

    func throwAasAAsync(a: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(A(aMem: a))
        }
    }

    func throwAorDasAorDAsync(a: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            if a > 0 {
                seal.reject(A(aMem: a))
            } else {
                seal.reject(D(dMem: a))
            }
        }
    }

    func throwBasAAsync(a: Int32, b: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(B(aMem: a, bMem: b))
        }
    }

    func throwCasAAsync(a: Int32, b: Int32, c: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwBasBAsync(a: Int32, b: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(B(aMem: a, bMem: b))
        }
    }

    func throwCasBAsync(a: Int32, b: Int32, c: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwCasCAsync(a: Int32, b: Int32, c: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwModAAsync(a: Int32, a2: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(ModA(aMem: a, a2Mem: a2))
        }
    }

    func throwUndeclaredAAsync(a: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(A(aMem: a))
        }
    }

    func throwUndeclaredBAsync(a: Int32, b: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(B(aMem: a, bMem: b))
        }
    }

    func throwUndeclaredCAsync(a: Int32, b: Int32, c: Int32, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwLocalExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(Ice.TimeoutException())
        }
    }

    func throwNonIceExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(RuntimeError())
        }
    }

    func throwAssertExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func throwMemoryLimitExceptionAsync(seq _: ByteSeq, current _: Current) -> Promise<ByteSeq> {
        return Promise<ByteSeq> { seal in
            // 20KB is over the configured 10KB message size max.
            seal.fulfill(ByteSeq(repeating: 0, count: 1024 * 20))
        }
    }

    func throwLocalExceptionIdempotentAsync(current _: Current) -> Promise<Void> {
        return Promise { seal in
            seal.reject(Ice.TimeoutException())
        }
    }

    func throwAfterResponseAsync(current _: Current) -> Promise<Void> {
        return Promise { seal in
            seal.fulfill(())
            throw Ice.RuntimeError("")
        }
    }

    func throwAfterExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise { seal in
            seal.reject(A(aMem: 12345))
            throw Ice.RuntimeError("")
        }
    }

    func throwEAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(E(data: "E"))
        }
    }

    func throwFAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(F(data: "F"))
        }
    }

    func throwGAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(G(data: "G"))
        }
    }

    func throwHAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(H(data: "H"))
        }
    }
}
