//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Foundation
import Dispatch

class PingReplyI: PingReply {

    var _replies: Int32 = 0
    var _lock = os_unfair_lock()
    var _semaphore = DispatchSemaphore(value: 0)

    func reset() {
        withLock(&_lock) {
            _replies = 0
        }
    }

    func reply(current: Current) throws {
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

public func allTests(_ helper: TestHelper) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    communicator.getProperties().setProperty(key: "ReplyAdapter.Endpoints", value: "udp")
    let adapter = try communicator.createObjectAdapter("ReplyAdapter")
    var replyI = PingReplyI()
    var reply = try uncheckedCast(prx: adapter.addWithUUID(PingReplyDisp(replyI)),
                                  type: PingReplyPrx.self).ice_datagram()
    try adapter.activate()

    let output = helper.getWriter()

    output.write("testing udp... ")
    var base = try communicator.stringToProxy("test:\(helper.getTestEndpoint(num: 0, prot: "udp"))")!.ice_datagram()
    let obj = uncheckedCast(prx: base, type: TestIntfPrx.self)

    var ret = false
    for _ in 0..<5 {
        replyI.reset()
        try obj.ping(reply)
        try obj.ping(reply)
        try obj.ping(reply)
        ret = replyI.waitReply(expectedReplies: 3, timeout: 2000)
        if ret {
            break // Success
        }

        // If the 3 datagrams were not received within the 2 seconds, we try again to
        // receive 3 new datagrams using a new object. We give up after 5 retries.
        replyI = PingReplyI()
        reply = try uncheckedCast(prx: adapter.addWithUUID(PingReplyDisp(replyI)),
                                  type: PingReplyPrx.self).ice_datagram()
    }
    try test(ret)

    if communicator.getProperties().getPropertyAsInt("Ice.Override.Compress") == 0 {
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
                try obj.sendByteSeq(seq: seq, reply: reply)
                _ = replyI.waitReply(expectedReplies: 1, timeout: 10000)
            }
        } catch is Ice.DatagramLimitException {
            //
            // The server's Ice.UDP.RcvSize property is set to 16384, which means that DatagramLimitException
            // will be throw when try to send a packet bigger than that.
            //
            try test(seq.count > 16384)
        }
        try obj.ice_getConnection()!.close(.GracefullyWithWait)
        communicator.getProperties().setProperty(key: "Ice.UDP.SndSize", value: "64000")
        seq = ByteSeq(repeating: 0, count: 50000)
        do {
            replyI.reset()
            try obj.sendByteSeq(seq: seq, reply: reply)
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
    if communicator.getProperties().getProperty("Ice.IPv6") == "1" {
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
        try objMcast.ping(reply)
        ret = replyI.waitReply(expectedReplies: 5, timeout: 5000)
        if ret {
            break
        }
        replyI = PingReplyI()
        reply = try uncheckedCast(prx: adapter.addWithUUID(PingReplyDisp(replyI)).ice_datagram(),
                                  type: PingReplyPrx.self)
    }

    if ret {
        output.writeLine("ok")
    } else {
        output.writeLine("failed(is a firewall enabled?)")
    }

    output.write("testing udp bi-dir connection... ")
    try obj.ice_getConnection()!.setAdapter(adapter)
    try objMcast.ice_getConnection()!.setAdapter(adapter)
    for _ in 0..<5 {
        replyI.reset()
        try obj.pingBiDir(reply.ice_getIdentity())
        try obj.pingBiDir(reply.ice_getIdentity())
        try obj.pingBiDir(reply.ice_getIdentity())
        ret = replyI.waitReply(expectedReplies: 3, timeout: 2000)
        if ret {
            break // Success
        }
        replyI = PingReplyI()
        reply = try uncheckedCast(prx: adapter.addWithUUID(PingReplyDisp(replyI)),
                                  type: PingReplyPrx.self).ice_datagram()
    }
    try test(ret)
    output.writeLine("ok")

    //
    // Sending the replies back on the multicast UDP connection doesn't work for most
    // platform(it works for macOS Leopard but not Snow Leopard, doesn't work on SLES,
    // Windows...). For Windows, see UdpTransceiver constructor for the details. So
    // we don't run this test.
    //
    //         Console.Out.Write("testing udp bi-dir connection... ");
    //         nRetry = 5;
    //         while(nRetry-- > 0)
    //         {
    //             replyI.reset();
    //             objMcast.pingBiDir(reply.ice_getIdentity());
    //             ret = replyI.waitReply(5, 2000);
    //             if(ret)
    //             {
    //                 break; // Success
    //             }
    //             replyI = new PingReplyI();
    //             reply =(PingReplyPrx)PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
    //         }

    //         if(!ret)
    //         {
    //             Console.Out.WriteLine("failed(is a firewall enabled?)");
    //         }
    //         else
    //         {
    //             Console.Out.WriteLine("ok");
    //         }
}
