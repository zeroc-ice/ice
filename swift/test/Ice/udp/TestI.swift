//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice

class TestIntfI: TestIntf {
    func ping(reply: PingReplyPrx?, current: Current) throws {
        do {
            try reply!.reply()
        } catch {
            precondition(false)
        }
    }

    func sendByteSeq(seq: ByteSeq, reply: PingReplyPrx?, current: Current) throws {
        do {
            try reply!.reply()
        } catch {
            precondition(false)
        }
    }

    func pingBiDir(reply: Identity, current: Current) throws {
        do {
            //
            // Ensure sending too much data doesn't cause the UDP connection
            // to be closed.
            //
            do {
                let seq = [UInt8](repeating: 0, count: 32 * 1024)
                let prx = try uncheckedCast(prx: current.con!.createProxy(reply)!, type: TestIntfPrx.self)
                try prx.sendByteSeq(seq: seq, reply: nil)
            } catch is Ice.DatagramLimitException {
                // Expected.
            }
            try uncheckedCast(prx: current.con!.createProxy(reply)!,
                              type: PingReplyPrx.self).reply()
        } catch {
            precondition(false)
        }
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
