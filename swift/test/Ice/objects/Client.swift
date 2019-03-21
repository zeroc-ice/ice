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

        try initial.shutdown()
    }
}
