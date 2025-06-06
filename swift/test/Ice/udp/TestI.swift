// Copyright (c) ZeroC, Inc.

import Ice

final class TestIntfI: TestIntf {
    func ping(reply: PingReplyPrx?, current _: Current) async throws {
        do {
            try await reply!.reply()
        } catch {
            preconditionFailure()
        }
    }

    func sendByteSeq(seq _: ByteSeq, reply: PingReplyPrx?, current _: Current) async throws {
        do {
            try await reply!.reply()
        } catch {
            preconditionFailure()
        }
    }

    func pingBiDir(reply: Identity, current: Current) async throws {
        do {
            //
            // Ensure sending too much data doesn't cause the UDP connection
            // to be closed.
            //
            do {
                let seq = Ice.ByteSeq(repeating: 0, count: 32 * 1024)
                let prx = try uncheckedCast(prx: current.con!.createProxy(reply), type: TestIntfPrx.self)
                try await prx.sendByteSeq(seq: seq, reply: nil)
            } catch is Ice.DatagramLimitException {
                // Expected.
            }
            try await uncheckedCast(
                prx: current.con!.createProxy(reply),
                type: PingReplyPrx.self
            ).reply()
        } catch {
            preconditionFailure()
        }
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}
