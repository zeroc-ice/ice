//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }
        try communicator.getValueFactoryManager().add(factory: { _ in BI() }, id: "::Test::B")
        try communicator.getValueFactoryManager().add(factory: { _ in CI() }, id: "::Test::C")
        try communicator.getValueFactoryManager().add(factory: { _ in DI() }, id: "::Test::D")
        try communicator.getValueFactoryManager().add(factory: { _ in EI() }, id: "::Test::E")
        try communicator.getValueFactoryManager().add(factory: { _ in FI() }, id: "::Test::F")
        try communicator.getValueFactoryManager().add(factory: { _ in II() }, id: "::Test::I")
        try communicator.getValueFactoryManager().add(factory: { _ in JI() }, id: "::Test::J")
        try communicator.getValueFactoryManager().add(factory: { _ in HI() }, id: "::Test::H")

        let output = self.getWriter()

        output.write("testing stringToProxy... ")
        let ref = "initial:\(self.getTestEndpoint(num: 0))"
        let base = try communicator.stringToProxy(ref)!
        output.writeLine("ok")

        output.write("testing checked cast... ")
        let initial = try checkedCast(prx: base, type: InitialPrx.self)!
        try test(initial == base)
        output.writeLine("ok")

        output.write("getting B1... ")
        let b1 = try initial.getB1()!
        output.writeLine("ok")

        output.write("getting B2... ")
        let b2 = try initial.getB2()!
        output.writeLine("ok")

        output.write("getting C... ")
        let c = try initial.getC()!
        output.writeLine("ok")

        output.write("getting D... ")
        let d = try initial.getD()!
        output.writeLine("ok")

        output.write("checking consistency... ")
        try test(b1 !== b2)
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
        try test(b1.theB === b1)
        try test(b1.theC == nil)
        try test(b1.theA is B)
        try test((b1.theA as! B).theA === b1.theA)
        try test((b1.theA as! B).theB === b1)
        //test(((B)b1.theA).theC is C); // Redundant -- theC is always of type C
        try test((b1.theA as! B).theC!.theB === b1.theA)
        try test(b1.preMarshalInvoked)
        try test(b1.postUnmarshalInvoked)
        try test(b1.theA!.preMarshalInvoked)
        try test(b1.theA!.postUnmarshalInvoked)
        try test((b1.theA as! B).theC!.preMarshalInvoked)
        try test((b1.theA as! B).theC!.postUnmarshalInvoked)

        try initial.shutdown()
    }
}
