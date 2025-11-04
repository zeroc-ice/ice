// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

class PingReplyI: PingReply, @unchecked Sendable {
    private let _stream: AsyncStream<Void>
    private let _streamContinuation: AsyncStream<Void>.Continuation

    init() {
        let (stream, continuation) = AsyncStream<Void>.makeStream()
        _stream = stream
        _streamContinuation = continuation
    }

    func reply(current _: Current) {
        _streamContinuation.yield()
    }

    func getProxy(_ adapter: ObjectAdapter) throws -> PingReplyPrx {
        try uncheckedCast(prx: adapter.addWithUUID(self), type: PingReplyPrx.self).ice_datagram()
    }

    func waitReply(expectedReplies: Int, timeout: Int) async -> Bool {
        precondition(timeout > 0, "Timeout must be greater than 0")

        return await withTaskGroup(of: Bool.self) { group in
            group.addTask {
                var count = 0
                for await _ in self._stream {
                    count += 1
                    if count == expectedReplies {
                        break
                    }
                }
                return true
            }

            group.addTask {
                try? await Task.sleep(for: .milliseconds(timeout))
                return false
            }

            let result = await group.next()!
            group.cancelAll()
            return result
        }
    }
}

public func allTests(_ helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    communicator.getProperties().setProperty(key: "ReplyAdapter.Endpoints", value: "udp")
    let adapter = try communicator.createObjectAdapter("ReplyAdapter")
    try adapter.activate()

    let output = helper.getWriter()

    output.write("testing udp... ")
    var base = try communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 0, prot: "udp"))")!
        .ice_datagram()
    let obj = uncheckedCast(prx: base, type: TestIntfPrx.self)

    var ret = false
    for _ in 0..<5 {
        let replyI = PingReplyI()
        let reply = try replyI.getProxy(adapter)

        try await obj.ping(reply)
        try await obj.ping(reply)
        try await obj.ping(reply)
        ret = await replyI.waitReply(expectedReplies: 3, timeout: 2000)
        if ret {
            break  // Success
        }

        // If the 3 datagrams were not received within the 2 seconds, we try again to
        // receive 3 new datagrams using a new object. We give up after 5 retries.
    }
    try test(ret)

    if try communicator.getProperties().getIcePropertyAsInt("Ice.Override.Compress") == 0 {
        //
        // Only run this test if compression is disabled, the test expect fixed message size
        // to be sent over the wire.
        //
        var seq: ByteSeq
        do {
            seq = ByteSeq(repeating: 0, count: 1024)
            let replyI = PingReplyI()
            let reply = try replyI.getProxy(adapter)

            while true {
                seq = ByteSeq(repeating: 0, count: seq.count * 2 + 10)
                try await obj.sendByteSeq(seq: seq, reply: reply)
            }
        } catch is Ice.DatagramLimitException {
            //
            // The server's Ice.UDP.RcvSize property is set to 16384, which means that DatagramLimitException
            // will be throw when try to send a packet bigger than that.
            //
            try test(seq.count > 16384)
        }
        try await obj.ice_getConnection()!.close()
        communicator.getProperties().setProperty(key: "Ice.UDP.SndSize", value: "64000")
        seq = ByteSeq(repeating: 0, count: 50000)
        do {
            let replyI = PingReplyI()
            let reply = try replyI.getProxy(adapter)

            try await obj.sendByteSeq(seq: seq, reply: reply)
            let b = await replyI.waitReply(expectedReplies: 1, timeout: 500)
            //
            // The server's Ice.UDP.RcvSize property is set to 16384, which means this packet
            // should not be delivered.
            //
            try test(!b)
        } catch {
            print("Failed to send large request over udp: \(error)")
            try test(false)
        }
    }
    output.writeLine("ok")

    output.write("testing udp multicast... ")
    var endpoint = "udp -h "
    if communicator.getProperties().getIceProperty("Ice.IPv6") == "1" {
        endpoint += "\"ff15::1:1\" --interface \"::1\""
    } else {
        endpoint += "239.255.1.1 --interface 127.0.0.1"
    }
    endpoint += " -p "
    endpoint += "\(helper.getTestPort(num: 10))"
    base = try communicator.stringToProxy("test -d:\(endpoint)")!
    let objMcast = uncheckedCast(prx: base, type: TestIntfPrx.self)

    for _ in 0..<5 {
        let replyI = PingReplyI()
        let reply = try replyI.getProxy(adapter)
        do {
            try await objMcast.ping(reply)
            ret = await replyI.waitReply(expectedReplies: 5, timeout: 5000)
        } catch is Ice.SocketException
            where communicator.getProperties().getIceProperty("Ice.IPv6") == "1"
        {
            output.write("(not supported) ")
            ret = true
        }

        if ret {
            break
        }
    }

    try test(ret)
    output.writeLine("ok")

    output.write("testing udp bi-dir connection... ")
    try await obj.ice_getConnection()!.setAdapter(adapter)
    for _ in 0..<5 {
        let replyI = PingReplyI()
        let reply = try replyI.getProxy(adapter)
        try await obj.pingBiDir(reply.ice_getIdentity())
        try await obj.pingBiDir(reply.ice_getIdentity())
        try await obj.pingBiDir(reply.ice_getIdentity())
        ret = await replyI.waitReply(expectedReplies: 3, timeout: 2000)
        if ret {
            break  // Success
        }
    }
    try test(ret)
    output.writeLine("ok")
}
