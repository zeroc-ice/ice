// Copyright (c) ZeroC, Inc.

import Darwin
import Ice
import TestCommon

func batchOneways(_ helper: TestHelper, _ p: MyClassPrx) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let bs1 = ByteSeq(repeating: 0, count: 10 * 1024)
    let batch = p.ice_batchOneway()
    try await batch.ice_flushBatchRequests()  // Empty flush

    _ = try await p.opByteSOnewayCallCount()  // Reset the call count

    for _ in 0..<30 {
        do {
            try await batch.opByteSOneway(bs1)
        } catch is Ice.MarshalException {
            try test(false)
        }
    }

    var count: Int32 = 0
    while count < 27 {  // 3 * 9 requests auto-flushed.
        count += try await p.opByteSOnewayCallCount()
        usleep(100)
    }

    var conn = try await batch.ice_getConnection()
    if conn != nil {
        let batch1 = p.ice_batchOneway()
        let batch2 = p.ice_batchOneway()

        try await batch1.ice_ping()
        try await batch2.ice_ping()
        try await batch1.ice_flushBatchRequests()
        try await batch1.ice_getConnection()!.close()
        try await batch1.ice_ping()
        try await batch2.ice_ping()

        _ = try await batch1.ice_getConnection()
        _ = try await batch2.ice_getConnection()

        try await batch1.ice_ping()
        try await batch1.ice_getConnection()!.close()
        try await batch1.ice_ping()
        try await batch2.ice_ping()
    }

    var identity = Ice.Identity()
    identity.name = "invalid"
    let batch3 = batch.ice_identity(identity)
    try await batch3.ice_ping()
    try await batch3.ice_flushBatchRequests()

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    try await batch3.ice_ping()
    try await batch.ice_ping()
    try await batch.ice_flushBatchRequests()
    try await batch.ice_ping()

    try await p.ice_ping()

    var supportsCompress = true
    do {
        supportsCompress = try await p.supportsCompress()
    } catch is Ice.OperationNotExistException {}

    conn = try await p.ice_getConnection()
    if supportsCompress,
        conn != nil,
        p.ice_getCommunicator().getProperties().getIceProperty("Ice.Override.Compress") == ""
    {
        let prx = try await p.ice_getConnection()!.createProxy(p.ice_getIdentity()).ice_batchOneway()

        let batchC1 = uncheckedCast(prx: prx.ice_compress(false), type: MyClassPrx.self)
        let batchC2 = uncheckedCast(prx: prx.ice_compress(true), type: MyClassPrx.self)
        let batchC3 = uncheckedCast(prx: prx.ice_identity(identity), type: MyClassPrx.self)

        try await batchC1.opByteSOneway(bs1)
        try await batchC1.opByteSOneway(bs1)
        try await batchC1.opByteSOneway(bs1)
        try await batchC1.ice_getConnection()!.flushBatchRequests(.Yes)

        try await batchC2.opByteSOneway(bs1)
        try await batchC2.opByteSOneway(bs1)
        try await batchC2.opByteSOneway(bs1)
        try await batchC1.ice_getConnection()!.flushBatchRequests(.No)

        try await batchC1.opByteSOneway(bs1)
        try await batchC1.opByteSOneway(bs1)
        try await batchC1.opByteSOneway(bs1)
        try await batchC1.ice_getConnection()!.flushBatchRequests(.BasedOnProxy)

        try await batchC1.opByteSOneway(bs1)
        try await batchC2.opByteSOneway(bs1)
        try await batchC1.opByteSOneway(bs1)
        try await batchC1.ice_getConnection()!.flushBatchRequests(.BasedOnProxy)

        try await batchC1.opByteSOneway(bs1)
        try await batchC3.opByteSOneway(bs1)
        try await batchC1.opByteSOneway(bs1)
        try await batchC1.ice_getConnection()!.flushBatchRequests(.BasedOnProxy)
    }
}
