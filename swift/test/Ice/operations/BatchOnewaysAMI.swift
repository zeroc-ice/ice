//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Darwin
import Ice
import TestCommon
import PromiseKit

func batchOnewaysAMI(_ helper: TestHelper, _ p: MyClassPrx) throws {

    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let bs1 = [UInt8](repeating: 0, count: 10 * 1024)
    let batch = p.ice_batchOneway()

    try firstly {
        batch.ice_flushBatchRequestsAsync()
    }.wait()

    for _ in 0..<30 {
        _ = batch.opByteSOnewayAsync(bs1)
    }

    var count: Int32 = 0
    while count < 27 { // 3 * 9 requests auto-flushed.
        count += try p.opByteSOnewayCallCount()
        usleep(100)
    }

    let conn = try batch.ice_getConnection()
    if conn != nil {
        let batch1 = uncheckedCast(prx: p.ice_batchOneway(), type: MyClassPrx.self)
        let batch2 = uncheckedCast(prx: p.ice_batchOneway(), type: MyClassPrx.self)

        _ = batch1.ice_pingAsync()
        _ = batch2.ice_pingAsync()
        try batch1.ice_flushBatchRequestsAsync().wait()
        try batch1.ice_getConnection()!.close(Ice.ConnectionClose.GracefullyWithWait)
        _ = batch1.ice_pingAsync()
        _ = batch2.ice_pingAsync()

        _ = try batch1.ice_getConnection()
        _ = try batch2.ice_getConnection()

        _ = batch1.ice_pingAsync()
        try batch1.ice_getConnection()!.close(Ice.ConnectionClose.GracefullyWithWait)

        _ = batch1.ice_pingAsync()
        _ = batch2.ice_pingAsync()
    }

    let batch3 = try batch.ice_identity(Ice.Identity(name: "invalid", category: ""))
    _ = batch3.ice_pingAsync()
    try batch3.ice_flushBatchRequestsAsync().wait()

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    _ = batch3.ice_pingAsync()
    _ = batch.ice_pingAsync()
    try batch.ice_flushBatchRequestsAsync().wait()
    _ = batch.ice_pingAsync()
}
