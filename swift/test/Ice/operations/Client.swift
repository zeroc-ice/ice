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
        properties.setProperty(key: "Ice.ThreadPool.Client.Size", value: "2")
        properties.setProperty(key: "Ice.ThreadPool.Client.SizeWarn", value: "0")
        properties.setProperty(key: "Ice.BatchAutoFlushSize", value: "100")
        initData.properties = properties

        let communicator = try self.initialize(initData)
        defer {
            communicator.destroy()
        }

        let baseProxy = try communicator.stringToProxy("test:\(self.getTestEndpoint(num: 0))")!
        let cl = try checkedCast(prx: baseProxy, type: MyClassPrx.self)!
        let derivedProxy = try checkedCast(prx: cl, type: MyDerivedClassPrx.self)!

        writer.write("testing twoway operations... ")
        try twoways(self, cl)
        try twoways(self, derivedProxy)
        try derivedProxy.opDerived()
        writer.writeLine("ok")

        writer.write("testing oneway operations... ")
        try oneways(self, cl)
        try oneways(self, derivedProxy)
        writer.writeLine("ok")

        writer.write("testing twoway operations with AMI... ")
        try twowaysAMI(self, cl)
        try twowaysAMI(self, derivedProxy)
        try derivedProxy.opDerived()
        writer.writeLine("ok")

        writer.write("testing oneway operations with AMI... ")
        try onewaysAMI(self, cl)
        try onewaysAMI(self, derivedProxy)
        writer.writeLine("ok")

        writer.write("testing batch oneway operations... ")
        try batchOneways(self, cl)
        try batchOneways(self, derivedProxy)
        writer.writeLine("ok")

        writer.write("testing batch oneway operations with AMI... ")
        try batchOneways(self, cl)
        try batchOneways(self, derivedProxy)
        writer.writeLine("ok")

        try cl.shutdown()
    }
}
