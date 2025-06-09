// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

private struct ServantLocatorI: Ice.ServantLocator {
    func locate(_: Current) throws -> (returnValue: Dispatcher?, cookie: AnyObject?) {
        return (nil, nil)
    }

    func finished(curr _: Current, servant _: Dispatcher, cookie _: AnyObject?) throws {}

    func deactivate(_: String) {}
}

func allTests(_ helper: TestHelper) async throws -> ThrowerPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing object adapter registration exceptions... ")
    do {
        _ = try communicator.createObjectAdapter("TestAdapter0")
    } catch is Ice.InitializationException {}

    communicator.getProperties().setProperty(key: "TestAdapter0.Endpoints", value: "tcp -h *")
    let first = try communicator.createObjectAdapter("TestAdapter0")
    do {
        _ = try communicator.createObjectAdapter("TestAdapter0")
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}

    do {
        _ = try communicator.createObjectAdapterWithEndpoints(
            name: "TestAdapter0", endpoints: "ssl -h foo -p 12011")
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}
    first.deactivate()
    output.writeLine("ok")

    output.write("testing servant registration exceptions... ")
    communicator.getProperties().setProperty(key: "TestAdapter1.Endpoints", value: "tcp -h *")
    var adapter = try communicator.createObjectAdapter("TestAdapter1")
    let obj = EmptyI()
    _ = try adapter.add(servant: obj, id: Ice.stringToIdentity("x"))
    do {
        _ = try adapter.add(servant: obj, id: Ice.stringToIdentity("x"))
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}

    _ = try adapter.remove(Ice.stringToIdentity("x"))
    do {
        _ = try adapter.remove(Ice.stringToIdentity("x"))
        try test(false)
    } catch is Ice.NotRegisteredException {}
    adapter.deactivate()
    output.writeLine("ok")

    output.write("testing servant locator registration exceptions... ")
    communicator.getProperties().setProperty(key: "TestAdapter2.Endpoints", value: "tcp -h *")
    adapter = try communicator.createObjectAdapter("TestAdapter2")
    try adapter.addServantLocator(locator: ServantLocatorI(), category: "x")
    do {
        try adapter.addServantLocator(locator: ServantLocatorI(), category: "x")
        try test(false)
    } catch is Ice.AlreadyRegisteredException {}

    // adapter.deactivate()
    output.writeLine("ok")

    output.write("testing stringToProxy... ")
    let ref = "thrower:\(helper.getTestEndpoint(num: 0))"
    let base = try communicator.stringToProxy(ref)!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    let thrower = try await checkedCast(prx: base, type: ThrowerPrx.self)!
    try test(thrower == base)
    output.writeLine("ok")

    output.write("catching exact types... ")
    do {
        try await thrower.throwAasA(1)
        try test(false)
    } catch let ex as A {
        try test(ex.aMem == 1)
    } catch {
        output.writeLine("\(error)")
        try test(false)
    }

    do {
        try await thrower.throwAorDasAorD(1)
        try test(false)
    } catch let ex as A {
        try test(ex.aMem == 1)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwAorDasAorD(-1)
        try test(false)
    } catch let ex as D {
        try test(ex.dMem == -1)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwBasB(a: 1, b: 2)
        try test(false)
    } catch let ex as B {
        try test(ex.aMem == 1)
        try test(ex.bMem == 2)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwCasC(a: 1, b: 2, c: 3)
        try test(false)
    } catch let ex as C {
        try test(ex.aMem == 1)
        try test(ex.bMem == 2)
        try test(ex.cMem == 3)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwModA(a: 1, a2: 2)
        try test(false)
    } catch let ex as ModA {
        try test(ex.aMem == 1)
        try test(ex.a2Mem == 2)
    } catch is Ice.OperationNotExistException {
        //
        // This operation is not supported in Java.
        //
    }
    output.writeLine("ok")

    output.write("catching base types... ")
    do {
        try await thrower.throwBasB(a: 1, b: 2)
        try test(false)
    } catch let ex as A {
        try test(ex.aMem == 1)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwCasC(a: 1, b: 2, c: 3)
        try test(false)
    } catch let ex as B {
        try test(ex.aMem == 1)
        try test(ex.bMem == 2)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwModA(a: 1, a2: 2)
        try test(false)
    } catch let ex as ModA {
        try test(ex.aMem == 1)
        try test(ex.a2Mem == 2)
    } catch is Ice.OperationNotExistException {
        //
        // This operation is not supported in Java.
        //
    }
    output.writeLine("ok")

    output.write("catching derived types... ")
    do {
        try await thrower.throwBasA(a: 1, b: 2)
        try test(false)
    } catch let ex as B {
        try test(ex.aMem == 1)
        try test(ex.bMem == 2)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwCasA(a: 1, b: 2, c: 3)
        try test(false)
    } catch let ex as C {
        try test(ex.aMem == 1)
        try test(ex.bMem == 2)
        try test(ex.cMem == 3)
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwCasB(a: 1, b: 2, c: 3)
        try test(false)
    } catch let ex as C {
        try test(ex.aMem == 1)
        try test(ex.bMem == 2)
        try test(ex.cMem == 3)
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    let supportsUndeclaredExceptions = try await thrower.supportsUndeclaredExceptions()
    if supportsUndeclaredExceptions {
        output.write("catching unknown user exception... ")

        do {
            try await thrower.throwUndeclaredA(1)
            try test(false)
        } catch is Ice.UnknownUserException {} catch {
            try test(false)
        }

        do {
            try await thrower.throwUndeclaredB(a: 1, b: 2)
            try test(false)
        } catch is Ice.UnknownUserException {} catch {
            try test(false)
        }

        do {
            try await thrower.throwUndeclaredC(a: 1, b: 2, c: 3)
            try test(false)
        } catch is Ice.UnknownUserException {} catch {
            try test(false)
        }
        output.writeLine("ok")
    }

    let conn = try await thrower.ice_getConnection()
    if conn != nil {
        output.write("testing memory limit marshal exception...")
        do {
            _ = try await thrower.throwMemoryLimitException(ByteSeq())
            try test(false)
        } catch is Ice.MarshalException {} catch {
            try test(false)
        }

        do {
            _ = try await thrower.throwMemoryLimitException(ByteSeq(repeating: 0, count: 20 * 1024))  // 20KB
            try test(false)
        } catch is Ice.ConnectionLostException {} catch is Ice.UnknownLocalException {
            // Expected with JS bidir server
        } catch {
            try test(false)
        }

        do {
            var str = "thrower:\(helper.getTestEndpoint(num: 1))"
            let thrower2 = try uncheckedCast(prx: communicator.stringToProxy(str)!, type: ThrowerPrx.self)
            do {
                // 2MB(no limits)
                _ = try await thrower2.throwMemoryLimitException(
                    ByteSeq(repeating: 0, count: 2 * 1024 * 1024))
            } catch is Ice.MarshalException {}
            str = "thrower:\(helper.getTestEndpoint(num: 2))"
            let thrower3 = try uncheckedCast(prx: communicator.stringToProxy(str)!, type: ThrowerPrx.self)
            do {
                // 1KB limit
                _ = try await thrower3.throwMemoryLimitException(ByteSeq(repeating: 0, count: 1024))
                try test(false)
            } catch is Ice.ConnectionLostException {}
        } catch is Ice.ConnectionRefusedException {
            // Expected with JS bidir server
        }
        output.writeLine("ok")
    }

    output.write("catching object not exist exception... ")
    do {
        let id = try Ice.stringToIdentity("does not exist")
        do {
            let thrower2 = uncheckedCast(prx: thrower.ice_identity(id), type: ThrowerPrx.self)
            try await thrower2.ice_ping()
            try test(false)
        } catch let ex as Ice.ObjectNotExistException {
            try test(ex.id == id)
        } catch {
            try test(false)
        }
    }
    output.writeLine("ok")

    output.write("catching facet not exist exception... ")
    do {
        let thrower2 = uncheckedCast(prx: thrower, type: ThrowerPrx.self, facet: "no such facet")
        do {
            try await thrower2.ice_ping()
            try test(false)
        } catch let ex as Ice.FacetNotExistException {
            try test(ex.facet == "no such facet")
        }
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("catching operation not exist exception... ")
    do {
        let thrower2 = uncheckedCast(prx: thrower, type: WrongOperationPrx.self)
        try await thrower2.noSuchOperation()
        try test(false)
    } catch let ex as Ice.OperationNotExistException {
        try test(ex.operation == "noSuchOperation")
    } catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("catching unknown local exception... ")
    do {
        try await thrower.throwLocalException()
        try test(false)
    } catch is Ice.UnknownLocalException {} catch {
        try test(false)
    }

    do {
        try await thrower.throwLocalExceptionIdempotent()
        try test(false)
    } catch is Ice.UnknownLocalException {
    } catch is Ice.OperationNotExistException {} catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("catching unknown non-Ice exception... ")
    do {
        try await thrower.throwNonIceException()
        try test(false)
    } catch is Ice.UnknownException {} catch {
        try test(false)
    }
    output.writeLine("ok")

    output.write("catching dispatch exception... ")
    do {
        try await thrower.throwDispatchException(ReplyStatus.operationNotExist.rawValue)
        try test(false)
    } catch let ex as Ice.OperationNotExistException {
        // The message is created locally so we don't need a cross-test variant.
        try test(
            ex.message
                == "Dispatch failed with operationNotExist { id = 'thrower', facet = '', operation = 'throwDispatchException' }"
        )
    }

    do {
        try await thrower.throwDispatchException(ReplyStatus.unauthorized.rawValue)
        try test(false)
    } catch let ex as Ice.DispatchException where ReplyStatus(rawValue: ex.replyStatus) == .unauthorized {
        try test(
            ex.message == "The dispatch failed with reply status unauthorized."
                || ex.message == "The dispatch failed with reply status Unauthorized.")  // for cross tests
    }

    do {
        try await thrower.throwDispatchException(212)
        try test(false)
    } catch let ex as Ice.DispatchException where ex.replyStatus == 212 {
        try test(ex.message == "The dispatch failed with reply status 212.")
    }
    output.writeLine("ok")

    output.write("testing asynchronous exceptions... ")
    do {
        try await thrower.throwAfterResponse()
    } catch {
        try test(false)
    }

    do {
        try await thrower.throwAfterException()
        try test(false)
    } catch is A {} catch {
        try test(false)
    }
    output.writeLine("ok")

    return thrower
}
