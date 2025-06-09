// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

final class RuntimeError: Error {}

final class ThrowerI: Thrower {
    func shutdown(current: Ice.Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }

    func supportsUndeclaredExceptions(current _: Current) async throws -> Bool {
        return true
    }

    func supportsAssertException(current _: Current) async throws -> Bool {
        return false
    }

    func throwAasA(a: Int32, current _: Ice.Current) async throws {
        throw A(aMem: a)
    }

    func throwAorDasAorD(a: Int32, current _: Ice.Current) async throws {
        if a > 0 {
            throw A(aMem: a)
        } else {
            throw D(dMem: a)
        }
    }

    func throwBasA(a: Int32, b: Int32, current: Ice.Current) async throws {
        try await throwBasB(a: a, b: b, current: current)
    }

    func throwBasB(a: Int32, b: Int32, current _: Ice.Current) async throws {
        let ex = B()
        ex.aMem = a
        ex.bMem = b
        throw ex
    }

    func throwCasA(a: Int32, b: Int32, c: Int32, current: Ice.Current) async throws {
        try await throwCasC(a: a, b: b, c: c, current: current)
    }

    func throwCasB(a: Int32, b: Int32, c: Int32, current: Ice.Current) async throws {
        try await throwCasC(a: a, b: b, c: c, current: current)
    }

    func throwCasC(a: Int32, b: Int32, c: Int32, current _: Ice.Current) async throws {
        throw C(aMem: a, bMem: b, cMem: c)
    }

    func throwModA(a: Int32, a2: Int32, current _: Current) async throws {
        throw ModA(aMem: a, a2Mem: a2)
    }

    func throwLocalException(current _: Ice.Current) async throws {
        throw Ice.TimeoutException("thrower throwing TimeOutException")
    }

    func throwNonIceException(current _: Ice.Current) async throws {
        throw RuntimeError()
    }

    func throwAssertException(current _: Ice.Current) async throws {}

    func throwMemoryLimitException(seq _: ByteSeq, current _: Ice.Current) async throws -> ByteSeq {
        // 20KB is over the configured 10KB message size max.
        return ByteSeq(repeating: 0, count: 1024 * 20)
    }

    func throwLocalExceptionIdempotent(current _: Ice.Current) async throws {
        throw Ice.TimeoutException("thrower throwing TimeOutException")
    }

    func throwUndeclaredA(a: Int32, current _: Ice.Current) async throws {
        throw A(aMem: a)
    }

    func throwUndeclaredB(a: Int32, b: Int32, current _: Ice.Current) async throws {
        throw B(aMem: a, bMem: b)
    }

    func throwUndeclaredC(a: Int32, b: Int32, c: Int32, current _: Ice.Current) async throws {
        throw C(aMem: a, bMem: b, cMem: c)
    }

    func throwDispatchException(replyStatus: UInt8, current _: Ice.Current) async throws {
        throw Ice.DispatchException(replyStatus: replyStatus)
    }

    func throwAfterResponse(current _: Ice.Current) async throws {
        // Not relevant for Swift as there's no AMD callback but we need to implement it for cross testing.
    }

    func throwAfterException(current _: Ice.Current) async throws {
        // Not relevant for Swift as there's no AMD callback but we need to implement it for cross testing.
        throw A()
    }
}
