// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public func allTests(helper: TestHelper, communicator2: Ice.Communicator, ref: String) async throws
    -> RetryPrx
{
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let output = helper.getWriter()
    let communicator = helper.communicator()

    output.write("testing stringToProxy... ")
    let base1 = try communicator.stringToProxy(ref)!
    let base2 = try communicator.stringToProxy(ref)!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    let retry1 = try await checkedCast(prx: base1, type: RetryPrx.self)!
    try test(retry1 == base1)
    var retry2 = try await checkedCast(prx: base2, type: RetryPrx.self)!
    try test(retry2 == base2)
    output.writeLine("ok")

    output.write("calling regular operation with first proxy... ")
    try await retry1.op(false)
    output.writeLine("ok")

    output.write("calling operation to kill connection with second proxy... ")
    do {
        try await retry2.op(true)
        try test(false)
    } catch is Ice.UnknownLocalException {
        // Expected with collocation
    } catch is Ice.ConnectionLostException {}
    output.writeLine("ok")

    output.write("calling regular operation with first proxy again... ")
    try await retry1.op(false)
    output.writeLine("ok")

    output.write("testing idempotent operation... ")
    try await test(retry1.opIdempotent(4) == 4)
    output.writeLine("ok")

    output.write("testing non-idempotent operation... ")
    do {
        try await retry1.opNotIdempotent()
        try test(false)
    } catch is Ice.LocalException {}
    output.writeLine("ok")

    output.write("testing invocation timeout and retries... ")

    retry2 = try await checkedCast(
        prx: communicator2.stringToProxy(retry1.ice_toString())!,
        type: RetryPrx.self)!
    do {
        // No more than 2 retries before timeout kicks-in
        _ = try await retry2.ice_invocationTimeout(500).opIdempotent(4)
        try test(false)
    } catch is Ice.InvocationTimeoutException {
        _ = try await retry2.opIdempotent(-1)  // Reset the counter
    }

    output.writeLine("ok")
    return retry1
}
