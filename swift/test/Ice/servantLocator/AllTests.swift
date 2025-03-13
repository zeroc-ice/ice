// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

func testExceptions(_ obj: TestIntfPrx, _ helper: TestHelper) async throws {
    do {
        try await obj.requestFailedException()
        try helper.test(false)
    } catch let ex as Ice.ObjectNotExistException {
        try helper.test(ex.id == obj.ice_getIdentity())
        try helper.test(ex.facet == obj.ice_getFacet())
        try helper.test(ex.operation == "requestFailedException")
    }

    do {
        try await obj.unknownUserException()
        try helper.test(false)
    } catch let ex as Ice.UnknownUserException {
        try helper.test(ex.message.contains("::Foo::BarException"))
    }

    do {
        try await obj.unknownLocalException()
        try helper.test(false)
    } catch let ex as Ice.UnknownLocalException {
        try helper.test(ex.message == "reason")
    }

    do {
        try await obj.unknownException()
        try helper.test(false)
    } catch let ex as Ice.UnknownException {
        try helper.test(ex.message == "reason")
    }

    do {
        try await obj.userException()
        try helper.test(false)
    } catch let ex as Ice.UnknownUserException {
        try helper.test(ex.message.contains("Test::TestIntfUserException"))
    } catch is Ice.OperationNotExistException {}

    do {
        try await obj.localException()
        try helper.test(false)
    } catch let ex as Ice.UnknownLocalException {
        try helper.test(
            ex.message.contains("Ice::SocketException") || ex.message.contains("Ice.SocketException"))
    }

    do {
        try await obj.unknownExceptionWithServantException()
        try helper.test(false)
    } catch let ex as Ice.UnknownException {
        try helper.test(ex.message == "reason")
    }

    do {
        _ = try await obj.impossibleException(false)
        try helper.test(false)
    } catch is Ice.UnknownUserException {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }

    do {
        _ = try await obj.impossibleException(true)
        try helper.test(false)
    } catch is Ice.UnknownUserException {
        // Operation throws TestImpossibleException, but locate() and finished() throw TestIntfUserException.
    }

    do {
        _ = try await obj.intfUserException(false)
        try helper.test(false)
    } catch is TestImpossibleException {
        // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
    }

    do {
        _ = try await obj.intfUserException(true)
        try helper.test(false)
    } catch is TestImpossibleException {
        // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
    }
}

func allTests(_ helper: TestHelper) async throws -> TestIntfPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing stringToProxy... ")
    var base = try communicator.stringToProxy("asm:\(helper.getTestEndpoint(num: 0))")!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    var obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    try test(obj == base)
    output.writeLine("ok")

    output.write("testing ice_ids... ")
    do {
        let o = try communicator.stringToProxy("category/locate:\(helper.getTestEndpoint(num: 0))")!
        _ = try await o.ice_ids()
        try test(false)
    } catch let ex as Ice.UnknownUserException {
        try test(ex.message.contains("::Test::TestIntfUserException"))
    }

    do {
        let o = try communicator.stringToProxy("category/finished:\(helper.getTestEndpoint(num: 0))")!
        _ = try await o.ice_ids()
        try test(false)
    } catch let ex as Ice.UnknownUserException {
        try test(ex.message.contains("::Test::TestIntfUserException"))
    }
    output.writeLine("ok")

    output.write("testing servant locator...")
    base = try communicator.stringToProxy("category/locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    do {
        _ = try await checkedCast(
            prx: communicator.stringToProxy("category/unknown:\(helper.getTestEndpoint(num: 0))")!,
            type: TestIntfPrx.self)
    } catch is Ice.ObjectNotExistException {}
    output.writeLine("ok")

    output.write("testing default servant locator...")
    base = try communicator.stringToProxy("anothercat/locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    base = try communicator.stringToProxy("locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!

    do {
        _ = try await checkedCast(
            prx: communicator.stringToProxy("anothercat/unknown:\(helper.getTestEndpoint(num: 0))")!,
            type: TestIntfPrx.self)
    } catch is Ice.ObjectNotExistException {}

    do {
        _ = try await checkedCast(
            prx: communicator.stringToProxy("unknown:\(helper.getTestEndpoint(num: 0))")!,
            type: TestIntfPrx.self)
    } catch is Ice.ObjectNotExistException {}
    output.writeLine("ok")

    output.write("testing locate exceptions... ")
    base = try communicator.stringToProxy("category/locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    try await testExceptions(obj, helper)
    output.writeLine("ok")

    output.write("testing finished exceptions... ")
    base = try communicator.stringToProxy("category/finished:\(helper.getTestEndpoint(num: 0))")!
    obj = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    try await testExceptions(obj, helper)
    output.writeLine("ok")

    output.write("testing servant locator removal... ")
    base = try communicator.stringToProxy("test/activation:\(helper.getTestEndpoint(num: 0))")!
    let activation = try await checkedCast(prx: base, type: TestActivationPrx.self)!
    try await activation.activateServantLocator(false)
    do {
        try await obj.ice_ping()
        try test(false)
    } catch is Ice.ObjectNotExistException {
        output.writeLine("ok")
    }

    output.write("testing servant locator addition... ")
    try await activation.activateServantLocator(true)
    do {
        try await obj.ice_ping()
        output.writeLine("ok")
    } catch {
        try test(false)
    }
    return obj
}
