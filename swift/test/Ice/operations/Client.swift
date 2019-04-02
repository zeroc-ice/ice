//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {

        PromiseKit.conf.Q.map = .global()
        PromiseKit.conf.Q.return = .global()
        PromiseKit.conf.logHandler = { _ in }

        let writer = getWriter()

        var initData = Ice.InitializationData()
        let (properties, _) = try createTestProperties(args: args)
        try properties.setProperty(key: "Ice.ThreadPool.Client.Size", value: "2")
        try properties.setProperty(key: "Ice.ThreadPool.Client.SizeWarn", value: "0")
        try properties.setProperty(key: "Ice.BatchAutoFlushSize", value: "100")
        initData.properties = properties

        let communicator = try self.initialize(initData)
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

        writer.write("testing oneway operations... ")
        try Oneways.oneways(self, cl)
        try Oneways.oneways(self, derivedProxy)
        writer.writeLine("ok")

        writer.write("testing twoway operations with AMI... ")
        try TwowaysAMI.twowaysAMI(self, cl)
        try TwowaysAMI.twowaysAMI(self, derivedProxy)
        try derivedProxy.opDerived()
        writer.writeLine("ok")

        writer.write("testing oneway operations with AMI... ")
        try OnewaysAMI.onewaysAMI(self, cl)
        try OnewaysAMI.onewaysAMI(self, derivedProxy)
        writer.writeLine("ok")

        writer.write("testing batch oneway operations... ")
        try BatchOneways.batchOneways(self, cl)
        try BatchOneways.batchOneways(self, derivedProxy)
        writer.writeLine("ok")

        writer.write("testing batch oneway operations with AMI... ")
        try BatchOneways.batchOneways(self, cl)
        try BatchOneways.batchOneways(self, derivedProxy)
        writer.writeLine("ok")

        try cl.shutdown()
    }
}
