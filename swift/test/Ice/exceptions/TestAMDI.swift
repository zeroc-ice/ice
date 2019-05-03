//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

class RuntimeError: Error {}

class ThrowerI: Thrower {
    func shutdownAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            current.adapter!.getCommunicator().shutdown()
            seal.fulfill(())
        }
    }

    func supportsUndeclaredExceptionsAsync(current: Current) -> Promise<Bool> {
        return Promise.value(true)
    }

    func supportsAssertExceptionAsync(current: Current) -> Promise<Bool> {
        return Promise.value(false)
    }

    func throwAasAAsync(a: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> {seal in
            seal.reject(A(aMem: a))
        }
    }

    func throwAorDasAorDAsync(a: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            if a > 0 {
                seal.reject(A(aMem: a))
            } else {
                seal.reject(D(dMem: a))
            }
        }
    }

    func throwBasAAsync(a: Int32, b: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(B(aMem: a, bMem: b))
        }
    }

    func throwCasAAsync(a: Int32, b: Int32, c: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> {seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwBasBAsync(a: Int32, b: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(B(aMem: a, bMem: b))
        }
    }

    func throwCasBAsync(a: Int32, b: Int32, c: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwCasCAsync(a: Int32, b: Int32, c: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwModAAsync(a: Int32, a2: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(ModA(aMem: a, a2Mem: a2))
        }
    }

    func throwUndeclaredAAsync(a: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(A(aMem: a))
        }
    }

    func throwUndeclaredBAsync(a: Int32, b: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(B(aMem: a, bMem: b))
        }
    }

    func throwUndeclaredCAsync(a: Int32, b: Int32, c: Int32, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(C(aMem: a, bMem: b, cMem: c))
        }
    }

    func throwLocalExceptionAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(Ice.TimeoutException())
        }
    }

    func throwNonIceExceptionAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(RuntimeError())
        }
    }

    func throwAssertExceptionAsync(current: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func throwMemoryLimitExceptionAsync(seq: ByteSeq, current: Current) -> Promise<ByteSeq> {
        return Promise<ByteSeq> { seal in
            // 20KB is over the configured 10KB message size max.
            seal.fulfill(ByteSeq(repeating: 0, count: 1024 * 20))
        }
    }

    func throwLocalExceptionIdempotentAsync(current: Current) -> Promise<Void> {
        return Promise { seal in
            seal.reject(Ice.TimeoutException())
        }
    }

    func throwAfterResponseAsync(current: Current) -> Promise<Void> {
        return Promise { seal in
            seal.fulfill(())
            throw Ice.RuntimeError("")
        }
    }

    func throwAfterExceptionAsync(current: Current) -> Promise<Void> {
        return Promise { seal in
            seal.reject(A(aMem: 12345))
            throw Ice.RuntimeError("")
        }
    }

    func throwEAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(E(data: "E"))
        }
    }

    func throwFAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(F(data: "F"))
        }
    }

    func throwGAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(G(data: "G"))
        }
    }

    func throwHAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.reject(H(data: "H"))
        }
    }
}
