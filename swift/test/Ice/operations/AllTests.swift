//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func allTests(helper: TestHelper) throws -> MyClassPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    let baseProxy = try communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 0))")!
    let cl = try checkedCast(prx: baseProxy, type: MyClassPrx.self)!
    let derivedProxy = try checkedCast(prx: cl, type: MyDerivedClassPrx.self)!

    output.write("testing twoway operations... ")
    try twoways(helper, cl)
    try twoways(helper, derivedProxy)
    try derivedProxy.opDerived()
    output.writeLine("ok")

    output.write("testing oneway operations... ")
    try oneways(helper, cl)
    try oneways(helper, derivedProxy)
    output.writeLine("ok")

    output.write("testing twoway operations with AMI... ")
    try twowaysAMI(helper, cl)
    try twowaysAMI(helper, derivedProxy)
    try derivedProxy.opDerived()
    output.writeLine("ok")

    output.write("testing oneway operations with AMI... ")
    try onewaysAMI(helper, cl)
    try onewaysAMI(helper, derivedProxy)
    output.writeLine("ok")

    output.write("testing batch oneway operations... ")
    try batchOneways(helper, cl)
    try batchOneways(helper, derivedProxy)
    output.writeLine("ok")

    output.write("testing batch oneway operations with AMI... ")
    try batchOneways(helper, cl)
    try batchOneways(helper, derivedProxy)
    output.writeLine("ok")

    return cl
}
