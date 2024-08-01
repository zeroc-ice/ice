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
    let testPrx = try checkedCast(prx: base, type: TestIntfPrx.self)!
    try test(testPrx == base)
    output.writeLine("ok")

    output.write("base... ")
    do {
        try testPrx.baseAsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "Base.b")
    }
    output.writeLine("ok")

    output.write("base (AMI)... ")
    do {
        try await testPrx.baseAsBaseAsync()
        try test(false)
    } catch let b as Base {
        try test(b.b == "Base.b")
    }
    output.writeLine("ok")

    output.write("slicing of unknown derived... ")
    do {
        try testPrx.unknownDerivedAsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownDerived.b")
    }
    output.writeLine("ok")

    output.write("slicing of unknown derived (AMI)... ")
    do {
        try await testPrx.unknownDerivedAsBaseAsync()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownDerived.b")
    }
    output.writeLine("ok")

    output.write("non-slicing of known derived as base... ")
    do {
        try testPrx.knownDerivedAsBase()
        try test(false)
    } catch let k as KnownDerived {
        try test(k.b == "KnownDerived.b")
        try test(k.kd == "KnownDerived.kd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known derived as base (AMI)... ")
    do {
        try await testPrx.knownDerivedAsBaseAsync()
        try test(false)
    } catch let k as KnownDerived {
        try test(k.b == "KnownDerived.b")
        try test(k.kd == "KnownDerived.kd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known derived as derived... ")
    do {
        try testPrx.knownDerivedAsKnownDerived()
        try test(false)
    } catch let k as KnownDerived {
        try test(k.b == "KnownDerived.b")
        try test(k.kd == "KnownDerived.kd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known derived as derived (AMI)... ")
    do {
        try await testPrx.knownDerivedAsKnownDerivedAsync()
        try test(false)
    } catch let k as KnownDerived {
        try test(k.b == "KnownDerived.b")
        try test(k.kd == "KnownDerived.kd")
    }
    output.writeLine("ok")

    output.write("slicing of unknown intermediate as base... ")
    do {
        try testPrx.unknownIntermediateAsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownIntermediate.b")
    }
    output.writeLine("ok")

    output.write("slicing of unknown intermediate as base (AMI)... ")
    do {
        try await testPrx.unknownIntermediateAsBaseAsync()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownIntermediate.b")
    }
    output.writeLine("ok")

    output.write("slicing of known intermediate as base... ")
    do {
        try testPrx.knownIntermediateAsBase()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "KnownIntermediate.b")
        try test(ki.ki == "KnownIntermediate.ki")
    }
    output.writeLine("ok")

    output.write("slicing of known intermediate as base (AMI)... ")
    do {
        try await testPrx.knownIntermediateAsBaseAsync()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "KnownIntermediate.b")
        try test(ki.ki == "KnownIntermediate.ki")
    }
    output.writeLine("ok")

    output.write("slicing of known most derived as base... ")
    do {
        try testPrx.knownMostDerivedAsBase()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("slicing of known most derived as base (AMI)... ")
    do {
        try await testPrx.knownMostDerivedAsBaseAsync()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known intermediate as intermediate... ")
    do {
        try testPrx.knownIntermediateAsKnownIntermediate()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "KnownIntermediate.b")
        try test(ki.ki == "KnownIntermediate.ki")
    }
    output.writeLine("ok")

    output.write("non-slicing of known intermediate as intermediate (AMI)... ")
    do {
        try await testPrx.knownIntermediateAsKnownIntermediateAsync()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "KnownIntermediate.b")
        try test(ki.ki == "KnownIntermediate.ki")
    }
    output.writeLine("ok")

    output.write("non-slicing of known most derived as intermediate... ")
    do {
        try testPrx.knownMostDerivedAsKnownIntermediate()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known most derived as intermediate (AMI)... ")
    do {
        try await testPrx.knownMostDerivedAsKnownIntermediateAsync()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known most derived as most derived... ")
    do {
        try testPrx.knownMostDerivedAsKnownMostDerived()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("non-slicing of known most derived as most derived (AMI)... ")
    do {
        try await testPrx.knownMostDerivedAsKnownMostDerivedAsync()
        try test(false)
    } catch let kmd as KnownMostDerived {
        try test(kmd.b == "KnownMostDerived.b")
        try test(kmd.ki == "KnownMostDerived.ki")
        try test(kmd.kmd == "KnownMostDerived.kmd")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, known intermediate as base... ")
    do {
        try testPrx.unknownMostDerived1AsBase()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "UnknownMostDerived1.b")
        try test(ki.ki == "UnknownMostDerived1.ki")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, known intermediate as base (AMI)... ")
    do {
        try await testPrx.unknownMostDerived1AsBaseAsync()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "UnknownMostDerived1.b")
        try test(ki.ki == "UnknownMostDerived1.ki")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, known intermediate as intermediate... ")
    do {
        try testPrx.unknownMostDerived1AsKnownIntermediate()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "UnknownMostDerived1.b")
        try test(ki.ki == "UnknownMostDerived1.ki")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ")
    do {
        try await testPrx.unknownMostDerived1AsKnownIntermediateAsync()
        try test(false)
    } catch let ki as KnownIntermediate {
        try test(ki.b == "UnknownMostDerived1.b")
        try test(ki.ki == "UnknownMostDerived1.ki")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, unknown intermediate thrown as base... ")
    do {
        try testPrx.unknownMostDerived2AsBase()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownMostDerived2.b")
    }
    output.writeLine("ok")

    output.write("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ")
    do {
        try await testPrx.unknownMostDerived2AsBaseAsync()
        try test(false)
    } catch let b as Base {
        try test(b.b == "UnknownMostDerived2.b")
    }
    output.writeLine("ok")

    return testPrx
}
