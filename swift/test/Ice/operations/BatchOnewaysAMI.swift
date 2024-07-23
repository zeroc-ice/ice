// Copyright (c) ZeroC, Inc.

import Darwin
import Ice
import PromiseKit
import TestCommon

func batchOnewaysAMI(_ helper: TestHelper, _ p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let bs1 = ByteSeq(repeating: 0, count: 10 * 1024)
    let batch = p.ice_batchOneway()

    try await batch.ice_flushBatchRequestsAsync()

    for _ in 0..<30 {
        async let _ = try batch.opByteSOnewayAsync(bs1)
    }

    var count: Int32 = 0
    while count < 27 {  // 3 * 9 requests auto-flushed.
        count += try p.opByteSOnewayCallCount()
        usleep(100)
    }

    let conn = try batch.ice_getConnection()
    if conn != nil {
        let batch1 = uncheckedCast(prx: p.ice_batchOneway(), type: MyClassPrx.self)
        let batch2 = uncheckedCast(prx: p.ice_batchOneway(), type: MyClassPrx.self)

        async let _ = try batch1.ice_pingAsync()
        async let _ = try batch2.ice_pingAsync()
        try await batch1.ice_flushBatchRequestsAsync()
        try batch1.ice_getConnection()!.close(Ice.ConnectionClose.GracefullyWithWait)
        async let _ = try batch1.ice_pingAsync()
        async let _ = try batch2.ice_pingAsync()

        _ = try batch1.ice_getConnection()
        _ = try batch2.ice_getConnection()

        async let _ = try batch1.ice_pingAsync()
        try batch1.ice_getConnection()!.close(Ice.ConnectionClose.GracefullyWithWait)

        async let _ = try batch1.ice_pingAsync()
        async let _ = try batch2.ice_pingAsync()
    }

    let batch3 = batch.ice_identity(Ice.Identity(name: "invalid", category: ""))
    async let _ = try batch3.ice_pingAsync()
    try await batch3.ice_flushBatchRequestsAsync()

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    async let _ = try batch3.ice_pingAsync()
    async let _ = try batch.ice_pingAsync()
    try await batch.ice_flushBatchRequestsAsync()
    async let _ = try batch.ice_pingAsync()
}
