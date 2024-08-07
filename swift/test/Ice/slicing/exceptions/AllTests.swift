// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public func allTests(_ helper: TestHelper) async throws -> TestIntfPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let output = helper.getWriter()
    let communicator = helper.communicator()

    output.write("testing stringToProxy... ")
    let ref = "Test:\(helper.getTestEndpoint(num: 0)) -t 2000"
    let base = try communicator.stringToProxy(ref)!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    let testPrx = try await checkedCast(prx: base, type: TestIntfPrx.self)!
    try test(testPrx == base)
    output.writeLine("ok")

    output.write("base... ")
    do {
        try await testPrx.baseAsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "Base.b")
    }
    output.writeLine("ok")

    output.write("slicing of unknown derived... ")
    do {
        try await testPrx.unknownDerivedAsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownDerived.b")
    }
    output.writeLine("ok")

    output.write("non-slicing of known derived as base... ")
    do {
        try await testPrx.knownDerivedAsBase()
        try test(false)
    } catch let k as KnownDerived {
        try test(k.b == "KnownDerived.b")
        try test(k.kd == "KnownDerived.kd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known derived as derived... ")
    do {
        try await testPrx.knownDerivedAsKnownDerived()
        try test(false)
    } catch let k as KnownDerived {
        try test(k.b == "KnownDerived.b")
        try test(k.kd == "KnownDerived.kd")
    }
    output.writeLine("ok")

    output.write("slicing of unknown intermediate as base... ")
    do {
        try await testPrx.unknownIntermediateAsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownIntermediate.b")
    }
    output.writeLine("ok")

    output.write("slicing of known intermediate as base... ")
    do {
        try await testPrx.knownIntermediateAsBase()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "KnownIntermediate.b")
        try test(ki.ki == "KnownIntermediate.ki")
    }
    output.writeLine("ok")

    output.write("slicing of known most derived as base... ")
    do {
        try await testPrx.knownMostDerivedAsBase()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known intermediate as intermediate... ")
    do {
        try await testPrx.knownIntermediateAsKnownIntermediate()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "KnownIntermediate.b")
        try test(ki.ki == "KnownIntermediate.ki")
    }
    output.writeLine("ok")

    output.write("non-slicing of known most derived as intermediate... ")
    do {
        try await testPrx.knownMostDerivedAsKnownIntermediate()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known most derived as most derived... ")
    do {
        try await testPrx.knownMostDerivedAsKnownMostDerived()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, known intermediate as base... ")
    do {
        try await testPrx.unknownMostDerived1AsBase()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "UnknownMostDerived1.b")
        try test(ki.ki == "UnknownMostDerived1.ki")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, known intermediate as intermediate... ")
    do {
        try await testPrx.unknownMostDerived1AsKnownIntermediate()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "UnknownMostDerived1.b")
        try test(ki.ki == "UnknownMostDerived1.ki")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, unknown intermediate thrown as base... ")
    do {
        try await testPrx.unknownMostDerived2AsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownMostDerived2.b")
    }
    output.writeLine("ok")

    return testPrx
}
