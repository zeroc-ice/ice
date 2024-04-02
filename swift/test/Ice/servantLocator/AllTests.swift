//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func testExceptions(_ obj: TestIntfPrx, _ helper: TestHelper) throws {
    do {
        try obj.requestFailedException()
        try helper.test(false)
    } catch let ex as Ice.ObjectNotExistException {
        try helper.test(ex.id == obj.ice_getIdentity())
        try helper.test(ex.facet == obj.ice_getFacet())
        try helper.test(ex.operation == "requestFailedException")
    }

    do {
        try obj.unknownUserException()
        try helper.test(false)
    } catch let ex as Ice.UnknownUserException {
        try helper.test(ex.unknown == "reason")
    }

    do {
        try obj.unknownLocalException()
        try helper.test(false)
    } catch let ex as Ice.UnknownLocalException {
        try helper.test(ex.unknown == "reason")
    }

    do {
        try obj.unknownException()
        try helper.test(false)
    } catch let ex as Ice.UnknownException {
        try helper.test(ex.unknown == "reason")
    }

    do {
        try obj.userException()
        try helper.test(false)
    } catch let ex as Ice.UnknownUserException {
        try helper.test(ex.unknown.contains("Test::TestIntfUserException"))
    } catch is Ice.OperationNotExistException {}

    do {
        try obj.localException()
        try helper.test(false)
    } catch let ex as Ice.UnknownLocalException {
        try helper.test(ex.unknown.contains("Ice::SocketException") || ex.unknown.contains("Ice.SocketException"))
    }

    do {
        try obj.unknownExceptionWithServantException()
        try helper.test(false)
    } catch let ex as Ice.UnknownException {
        try helper.test(ex.unknown == "reason")
    }

    do {
        _ = try obj.impossibleException(false)
        try helper.test(false)
    } catch is Ice.UnknownUserException {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }

    do {
        _ = try obj.impossibleException(true)
        try helper.test(false)
    } catch is Ice.UnknownUserException {
        // Operation throws TestImpossibleException, but locate() and finished() throw TestIntfUserException.
    }

    do {
        _ = try obj.intfUserException(false)
        try helper.test(false)
    } catch is TestImpossibleException {
        // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
    }

    do {
        _ = try obj.intfUserException(true)
        try helper.test(false)
    } catch is TestImpossibleException {
        // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
    }
}

func allTests(_ helper: TestHelper) throws -> TestIntfPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing stringToProxy... ")
    var base = try communicator.stringToProxy("asm:\(helper.getTestEndpoint(num: 0))")!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    var obj = try checkedCast(prx: base, type: TestIntfPrx.self)!
    try test(obj == base)
    output.writeLine("ok")

    output.write("testing ice_ids... ")
    do {
        let o = try communicator.stringToProxy("category/locate:\(helper.getTestEndpoint(num: 0))")!
        _ = try o.ice_ids()
        try test(false)
    } catch let ex as Ice.UnknownUserException {
        try test(ex.unknown == "::Test::TestIntfUserException")
    }

    do {
        let o = try communicator.stringToProxy("category/finished:\(helper.getTestEndpoint(num: 0))")!
        _ = try o.ice_ids()
        try test(false)
    } catch let ex as Ice.UnknownUserException {
        try test(ex.unknown == "::Test::TestIntfUserException")
    }
    output.writeLine("ok")

    output.write("testing servant locator...")
    base = try communicator.stringToProxy("category/locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try checkedCast(prx: base, type: TestIntfPrx.self)!
    do {
        _ = try checkedCast(prx: communicator.stringToProxy("category/unknown:\(helper.getTestEndpoint(num: 0))")!,
                            type: TestIntfPrx.self)
    } catch is Ice.ObjectNotExistException {}
    output.writeLine("ok")

    output.write("testing default servant locator...")
    base = try communicator.stringToProxy("anothercat/locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try checkedCast(prx: base, type: TestIntfPrx.self)!
    base = try communicator.stringToProxy("locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try checkedCast(prx: base, type: TestIntfPrx.self)!

    do {
        _ = try checkedCast(prx: communicator.stringToProxy("anothercat/unknown:\(helper.getTestEndpoint(num: 0))")!,
                            type: TestIntfPrx.self)
    } catch is Ice.ObjectNotExistException {}

    do {
        _ = try checkedCast(prx: communicator.stringToProxy("unknown:\(helper.getTestEndpoint(num: 0))")!,
                            type: TestIntfPrx.self)
    } catch is Ice.ObjectNotExistException {}
    output.writeLine("ok")

    output.write("testing locate exceptions... ")
    base = try communicator.stringToProxy("category/locate:\(helper.getTestEndpoint(num: 0))")!
    obj = try checkedCast(prx: base, type: TestIntfPrx.self)!
    try testExceptions(obj, helper)
    output.writeLine("ok")

    output.write("testing finished exceptions... ")
    base = try communicator.stringToProxy("category/finished:\(helper.getTestEndpoint(num: 0))")!
    obj = try checkedCast(prx: base, type: TestIntfPrx.self)!
    try testExceptions(obj, helper)

    //
    // Only call these for category/finished.
    //
    do {
        try obj.asyncResponse()
    } catch is TestIntfUserException {
        try test(false)
    } catch is TestImpossibleException {
        //
        // Called by finished().
        //
    }

    //
    // Only call these for category/finished.
    //
    do {
        try obj.asyncException()
    } catch is TestIntfUserException {
        try test(false)
    } catch is TestImpossibleException {
        //
        // Called by finished().
        //
    }
    output.writeLine("ok")

    output.write("testing servant locator removal... ")
    base = try communicator.stringToProxy("test/activation:\(helper.getTestEndpoint(num: 0))")!
    let activation = try checkedCast(prx: base, type: TestActivationPrx.self)!
    try activation.activateServantLocator(false)
    do {
        try obj.ice_ping()
        try test(false)
    } catch is Ice.ObjectNotExistException {
        output.writeLine("ok")
    }

    output.write("testing servant locator addition... ")
    try activation.activateServantLocator(true)
    do {
        try obj.ice_ping()
        output.writeLine("ok")
    } catch {
        try test(false)
    }
    return obj
}
