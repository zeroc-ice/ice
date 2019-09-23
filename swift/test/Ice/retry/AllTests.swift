//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

public func allTests(helper: TestHelper, communicator2: Ice.Communicator, ref: String) throws -> RetryPrx {
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
    let retry1 = try checkedCast(prx: base1, type: RetryPrx.self)!
    try test(retry1 == base1)
    var retry2 = try checkedCast(prx: base2, type: RetryPrx.self)!
    try test(retry2 == base2)
    output.writeLine("ok")

    output.write("calling regular operation with first proxy... ")
    try retry1.op(false)
    output.writeLine("ok")

    output.write("calling operation to kill connection with second proxy... ")
    do {
        try retry2.op(true)
        try test(false)
    } catch is Ice.UnknownLocalException {
        // Expected with collocation
    } catch is Ice.ConnectionLostException {}
    output.writeLine("ok")

    output.write("calling regular operation with first proxy again... ")
    try retry1.op(false)
    output.writeLine("ok")

    output.write("calling regular AMI operation with first proxy... ")
    try retry1.opAsync(false).wait()
    output.writeLine("ok")

    output.write("calling AMI operation to kill connection with second proxy... ")
    do {
        try retry2.opAsync(true).wait()
    } catch is Ice.ConnectionLostException {} catch is Ice.UnknownLocalException {}
    output.writeLine("ok")

    output.write("calling regular AMI operation with first proxy again... ")
    try retry1.opAsync(false).wait()
    output.writeLine("ok")

    output.write("testing idempotent operation... ")
    try test(retry1.opIdempotent(4) == 4)
    try test(retry1.opIdempotentAsync(4).wait() == 4)
    output.writeLine("ok")

    output.write("testing non-idempotent operation... ")
    do {
        try retry1.opNotIdempotent()
        try test(false)
    } catch is Ice.LocalException {}

    do {
        try retry1.opNotIdempotentAsync().wait()
        try test(false)
    } catch is Ice.LocalException {}
    output.writeLine("ok")

    if try retry1.ice_getConnection() != nil {
        output.write("testing system exception... ")
        do {
            try retry1.opSystemException()
            try test(false)
        } catch {}

        do {
            try retry1.opSystemExceptionAsync().wait()
            try test(false)
        } catch {}
        output.writeLine("ok")
    }

    output.write("testing invocation timeout and retries... ")

    retry2 = try checkedCast(prx: communicator2.stringToProxy(retry1.ice_toString())!,
                             type: RetryPrx.self)!
    do {
        // No more than 2 retries before timeout kicks-in
        _ = try retry2.ice_invocationTimeout(500).opIdempotent(4)
        try test(false)
    } catch is Ice.InvocationTimeoutException {
        _ = try retry2.opIdempotent(-1) // Reset the counter
    }

    do {
        // No more than 2 retries before timeout kicks-in
        _ = try retry2.ice_invocationTimeout(500).opIdempotentAsync(4).wait()
        try test(false)
    } catch is Ice.InvocationTimeoutException {
        _ = try retry2.opIdempotent(-1) // Reset the counter
    }

    if try retry1.ice_getConnection() != nil {
        // The timeout might occur on connection establishment or because of the sleep. What's
        // important here is to make sure there are 4 retries and that no calls succeed to
        // ensure retries with the old connection timeout semantics work.
        let retryWithTimeout = retry1.ice_invocationTimeout(-2).ice_timeout(100)
        do {
            try retryWithTimeout.sleep(500)
            try test(false)
        } catch is Ice.ConnectionTimeoutException {
        }
    }
    output.writeLine("ok")
    return retry1
}
