//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()
        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }

        let baseProxy = try communicator.stringToProxy("test:\(self.getTestEndpoint(num: 0))")!
        let cl = try checkedCast(prx: baseProxy, type: MyClassPrx.self)!
        let derivedProxy = try checkedCast(prx: cl, type: MyDerivedClassPrx.self)!

        writer.write("testing twoway operations... ")
        try Twoways.twoways(self, cl)
        try Twoways.twoways(self, derivedProxy)
        try derivedProxy.opDerived()
        writer.writeLine("ok")

        try cl.shutdown()
    }
}
