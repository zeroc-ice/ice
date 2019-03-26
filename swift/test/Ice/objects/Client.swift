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

        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        try test(b2.theA === b2)
        try test(d.theC === nil)
        output.writeLine("ok")

        output.write("getting B1, B2, C, and D all at once... ")
        let (b1out, b2out, cout, dout) = try initial.getAll()
        try test(b1out !== nil)
        try test(b2out !== nil)
        try test(cout !== nil)
        try test(dout !== nil)
        output.writeLine("ok")

        output.write("checking consistency... ")
        try test(b1out !== b2out)
        try test(b1out!.theA === b2out)
        try test(b1out!.theB === b1out)
        try test(b1out!.theC === nil)
        try test(b2out!.theA === b2out)
        try test(b2out!.theB === b1out)
        try test(b2out!.theC === cout)
        try test(cout!.theB === b2out)
        try test(dout!.theA === b1out)
        try test(dout!.theB === b2out)
        try test(dout!.theC === nil)
        try test(dout!.preMarshalInvoked)
        try test(dout!.postUnmarshalInvoked)
        try test(dout!.theA!.preMarshalInvoked)
        try test(dout!.theA!.postUnmarshalInvoked)
        try test(dout!.theB!.preMarshalInvoked)
        try test(dout!.theB!.postUnmarshalInvoked)
        try test(dout!.theB!.theC!.preMarshalInvoked)
        try test(dout!.theB!.theC!.postUnmarshalInvoked)
        output.writeLine("ok")

        output.write("getting I, J and H... ")
        let i = try initial.getI()
        try test(i !== nil)
        let j = try initial.getJ()
        try test(j != nil)
        let h = try initial.getH()
        try test(h != nil)
        output.writeLine("ok")

        output.write("getting K... ")
        let k = try initial.getK()!
        let l = k.value as! L
        try test(l.data == "l")
        output.writeLine("ok")

        output.write("testing Value as parameter... ")
        do {
            let (v3, v2) = try initial.opValue(L(data: "l"))
            try test((v2 as! L).data == "l")
            try test((v3 as! L).data == "l")
        }
        do {
            let (v3, v2) = try initial.opValueSeq([L(data: "l")])
            try test((v2[0] as! L).data == "l")
            try test((v3[0] as! L).data == "l")
        }
        do {
            let (v3, v2) = try initial.opValueMap(["l": L(data: "l")])
            try test((v2["l"]! as! L).data == "l")
            try test((v3["l"]! as! L).data == "l")
        }
        output.writeLine("ok")
        try initial.shutdown()
    }
}
