//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

let testString = "This is a test string"

class Cookie {
    func getString() -> String {
        return testString
    }
}

func allTests(_ helper: TestHelper) throws -> MyClassPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    let baseProxy = try communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 0))")!
    let cl = try checkedCast(prx: baseProxy, type: MyClassPrx.self)!
    let oneway = cl.ice_oneway()
    let batchOneway = cl.ice_batchOneway()

    output.write("testing ice_invoke... ")
    do {
        try test(oneway.ice_invoke(operation: "opOneway", mode: .Normal).ok)
        try test(batchOneway.ice_invoke(operation: "opOneway", mode: .Normal).ok)
        try test(batchOneway.ice_invoke(operation: "opOneway", mode: .Normal).ok)
        try test(batchOneway.ice_invoke(operation: "opOneway", mode: .Normal).ok)
        try test(batchOneway.ice_invoke(operation: "opOneway", mode: .Normal).ok)

        try batchOneway.ice_flushBatchRequests()
        let outS = Ice.OutputStream(communicator: communicator)
        outS.startEncapsulation()
        outS.write(testString)
        outS.endEncapsulation()
        let inEncaps = outS.finished()
        let result = try cl.ice_invoke(operation: "opString", mode: .Normal, inEncaps: inEncaps)
        try test(result.ok)
        let inS = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try inS.startEncapsulation()
        var s: String = try inS.read()
        try test(s == testString)
        s = try inS.read()
        try inS.endEncapsulation()
        try test(s == testString)
    }

    for i in 0..<2 {

        var ctx: Ice.Context! = nil
        if i == 1 {
            ctx = Ice.Context()
            ctx["raise"] = ""
        }
        let result = try cl.ice_invoke(operation: "opException", mode: .Normal, context: ctx)
        try test(!result.ok)
        let inS = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try inS.startEncapsulation()

        do {
            try inS.throwException()
        } catch is MyException {
            try inS.endEncapsulation()
        } catch {
            try test(false)
        }
    }

    output.writeLine("ok")

    output.write("testing asynchronous ice_invoke... ")
    do {
        var result = try oneway.ice_invokeAsync(operation: "opOneway", mode: .Normal).wait()
        try test(result.ok)

        let outS = Ice.OutputStream(communicator: communicator)
        outS.startEncapsulation()
        outS.write(testString)
        outS.endEncapsulation()
        let inEncaps = outS.finished()

        result = try cl.ice_invokeAsync(operation: "opString", mode: .Normal, inEncaps: inEncaps).wait()
        try test(result.ok)
        let inS = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try inS.startEncapsulation()
        var s: String = try inS.read()
        try test(s == testString)
        s = try inS.read()
        try inS.endEncapsulation()
        try test(s == testString)

    }

    do {
        let result = try cl.ice_invokeAsync(operation: "opException", mode: .Normal).wait()
        try test(!result.ok)
        let inS = Ice.InputStream(communicator: communicator, bytes: result.outEncaps)
        _ = try inS.startEncapsulation()
        do {
            try inS.throwException()
        }
        catch is MyException {
            try inS.endEncapsulation()
        } catch {
            try test(false)
        }
    }

    output.writeLine("ok")
    return cl
}
