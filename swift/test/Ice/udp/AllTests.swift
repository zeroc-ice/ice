// Copyright (c) ZeroC, Inc.

import Dispatch
import Foundation
import Ice
import TestCommon

class PingReplyI: PingReply {
    var _replies: Int32 = 0
    var _lock = os_unfair_lock()
    var _semaphore = DispatchSemaphore(value: 0)

    func reset() {
        withLock(&_lock) {
            _replies = 0
        }
    }

    func reply(current _: Current) async throws {
        withLock(&_lock) {
            _replies += 1
            _semaphore.signal()
        }
    }

    func waitReply(expectedReplies: Int, timeout: Int) -> Bool {
        let end = DispatchTime.now() + .milliseconds(timeout)
        while _replies < expectedReplies {
            let begin = DispatchTime.now()
            let delay = end.uptimeNanoseconds - begin.uptimeNanoseconds
            if delay > 0 {
                if _semaphore.wait(timeout: end) == .timedOut {
                    break
                }
            } else {
                break
            }
        }
        return _replies == expectedReplies
    }
}

public func allTests(_ helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    communicator.getProperties().setProperty(key: "ReplyAdapter.Endpoints", value: "udp")
    let adapter = try communicator.createObjectAdapter("ReplyAdapter")
    var replyI = PingReplyI()
    var reply = try uncheckedCast(
        prx: adapter.addWithUUID(replyI),
        type: PingReplyPrx.self
    ).ice_datagram()
    try adapter.activate()

    let output = helper.getWriter()

    output.write("testing udp... ")
    var base = try communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 0, prot: "udp"))")!
        .ice_datagram()
    let obj = uncheckedCast(prx: base, type: TestIntfPrx.self)

    var ret = false
    for _ in 0..<5 {
        replyI.reset()
        try await obj.ping(reply)
        try await obj.ping(reply)
        try await obj.ping(reply)
        ret = replyI.waitReply(expectedReplies: 3, timeout: 2000)
        if ret {
            break  // Success
        }

        // If the 3 datagrams were not received within the 2 seconds, we try again to
        // receive 3 new datagrams using a new object. We give up after 5 retries.
        replyI = PingReplyI()
        reply = try uncheckedCast(
            prx: adapter.addWithUUID(replyI),
            type: PingReplyPrx.self
        ).ice_datagram()
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
            while true {
                seq = ByteSeq(repeating: 0, count: seq.count * 2 + 10)
                replyI.reset()
                try await obj.sendByteSeq(seq: seq, reply: reply)
                _ = replyI.waitReply(expectedReplies: 1, timeout: 10000)
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
            replyI.reset()
            try await obj.sendByteSeq(seq: seq, reply: reply)
            let b = replyI.waitReply(expectedReplies: 1, timeout: 500)
            //
            // The server's Ice.UDP.RcvSize property is set to 16384, which means this packet
            // should not be delivered.
            //
            try test(!b)
        } catch is Ice.DatagramLimitException {}
    }
    output.writeLine("ok")

    output.write("testing udp multicast... ")
    var endpoint = ""
    //
    // Use loopback to prevent other machines to answer.
    //
    if communicator.getProperties().getIceProperty("Ice.IPv6") == "1" {
        endpoint += "udp -h \"ff15::1:1\" --interface \"::1\""
    } else {
        endpoint += "udp -h 239.255.1.1 --interface 127.0.0.1"
    }
    endpoint += " -p "
    endpoint += "\(helper.getTestPort(num: 10))"
    base = try communicator.stringToProxy("test -d:\(endpoint)")!
    let objMcast = uncheckedCast(prx: base, type: TestIntfPrx.self)

    for _ in 0..<5 {
        replyI.reset()
        do {
            try await objMcast.ping(reply)
            ret = replyI.waitReply(expectedReplies: 5, timeout: 5000)
        } catch is Ice.SocketException
            where communicator.getProperties().getIceProperty("Ice.IPv6") == "1"
        {
            output.write("(not supported) ")
            ret = true
        }

        if ret {
            break
        }

        replyI = PingReplyI()
        reply = try uncheckedCast(
            prx: adapter.addWithUUID(replyI).ice_datagram(),
            type: PingReplyPrx.self)
    }

    try test(ret)
    output.writeLine("ok")

    output.write("testing udp bi-dir connection... ")
    try await obj.ice_getConnection()!.setAdapter(adapter)
    for _ in 0..<5 {
        replyI.reset()
        try await obj.pingBiDir(reply.ice_getIdentity())
        try await obj.pingBiDir(reply.ice_getIdentity())
        try await obj.pingBiDir(reply.ice_getIdentity())
        ret = replyI.waitReply(expectedReplies: 3, timeout: 2000)
        if ret {
            break  // Success
        }
        replyI = PingReplyI()
        reply = try uncheckedCast(
            prx: adapter.addWithUUID(replyI),
            type: PingReplyPrx.self
        ).ice_datagram()
    }
    try test(ret)
    output.writeLine("ok")
}
