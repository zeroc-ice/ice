//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Darwin
import Ice
import TestCommon

func batchOneways(_ helper: TestHelper, _ p: MyClassPrx) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let bs1 = ByteSeq(repeating: 0, count: 10 * 1024)
    let batch = p.ice_batchOneway()
    try batch.ice_flushBatchRequests() // Empty flush

    _ = try p.opByteSOnewayCallCount() // Reset the call count

    for _ in 0 ..< 30 {
        do {
            try batch.opByteSOneway(bs1)
        } catch is Ice.MemoryLimitException {
            try test(false)
        }
    }

    var count: Int32 = 0
    while count < 27 { // 3 * 9 requests auto-flushed.
        count += try p.opByteSOnewayCallCount()
        usleep(100)
    }

    var conn = try batch.ice_getConnection()
    if conn != nil {
        let batch1 = p.ice_batchOneway()
        let batch2 = p.ice_batchOneway()

        try batch1.ice_ping()
        try batch2.ice_ping()
        try batch1.ice_flushBatchRequests()
        try batch1.ice_getConnection()!.close(Ice.ConnectionClose.GracefullyWithWait)
        try batch1.ice_ping()
        try batch2.ice_ping()

        _ = try batch1.ice_getConnection()
        _ = try batch2.ice_getConnection()

        try batch1.ice_ping()
        try batch1.ice_getConnection()!.close(Ice.ConnectionClose.GracefullyWithWait)
        try batch1.ice_ping()
        try batch2.ice_ping()
    }

    var identity = Ice.Identity()
    identity.name = "invalid"
    let batch3 = batch.ice_identity(identity)
    try batch3.ice_ping()
    try batch3.ice_flushBatchRequests()

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    try batch3.ice_ping()
    try batch.ice_ping()
    try batch.ice_flushBatchRequests()
    try batch.ice_ping()

    try p.ice_ping()

    var supportsCompress = true
    do {
        supportsCompress = try p.supportsCompress()
    } catch is Ice.OperationNotExistException {}

    conn = try p.ice_getConnection()
    if supportsCompress,
        conn != nil,
        p.ice_getCommunicator().getProperties().getProperty("Ice.Override.Compress") == "" {
        let prx = try p.ice_getConnection()!.createProxy(p.ice_getIdentity()).ice_batchOneway()

        let batchC1 = uncheckedCast(prx: prx.ice_compress(false), type: MyClassPrx.self)
        let batchC2 = uncheckedCast(prx: prx.ice_compress(true), type: MyClassPrx.self)
        let batchC3 = uncheckedCast(prx: prx.ice_identity(identity), type: MyClassPrx.self)

        try batchC1.opByteSOneway(bs1)
        try batchC1.opByteSOneway(bs1)
        try batchC1.opByteSOneway(bs1)
        try batchC1.ice_getConnection()!.flushBatchRequests(Ice.CompressBatch.Yes)

        try batchC2.opByteSOneway(bs1)
        try batchC2.opByteSOneway(bs1)
        try batchC2.opByteSOneway(bs1)
        try batchC1.ice_getConnection()!.flushBatchRequests(Ice.CompressBatch.No)

        try batchC1.opByteSOneway(bs1)
        try batchC1.opByteSOneway(bs1)
        try batchC1.opByteSOneway(bs1)
        try batchC1.ice_getConnection()!.flushBatchRequests(Ice.CompressBatch.BasedOnProxy)

        try batchC1.opByteSOneway(bs1)
        try batchC2.opByteSOneway(bs1)
        try batchC1.opByteSOneway(bs1)
        try batchC1.ice_getConnection()!.flushBatchRequests(Ice.CompressBatch.BasedOnProxy)

        try batchC1.opByteSOneway(bs1)
        try batchC3.opByteSOneway(bs1)
        try batchC1.opByteSOneway(bs1)
        try batchC1.ice_getConnection()!.flushBatchRequests(Ice.CompressBatch.BasedOnProxy)
    }
}
