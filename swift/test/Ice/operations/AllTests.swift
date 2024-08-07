// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

func allTests(helper: TestHelper) async throws -> MyClassPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    let baseProxy = try communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 0))")!
    let cl = try await checkedCast(prx: baseProxy, type: MyClassPrx.self)!
    let derivedProxy = try await checkedCast(prx: cl, type: MyDerivedClassPrx.self)!

    output.write("testing twoway operations... ")
    try await twoways(helper, cl)
    try await twoways(helper, derivedProxy)
    try await derivedProxy.opDerived()
    output.writeLine("ok")

    output.write("testing oneway operations... ")
    try await oneways(helper, cl)
    try await oneways(helper, derivedProxy)
    output.writeLine("ok")

    output.write("testing batch oneway operations... ")
    try await batchOneways(helper, cl)
    try await batchOneways(helper, derivedProxy)
    output.writeLine("ok")

    return cl
}
