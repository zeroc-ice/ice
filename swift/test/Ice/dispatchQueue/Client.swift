// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

public class Client: TestHelperI {
    override public func run(args: [String]) async throws {

        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }

        let output = getWriter()

        // Make the OA use its own dispatch queue
        communicator.getProperties().setProperty(key: "TestAdapter.ThreadPool.Size", value: "1")

        output.write("testing dispatch queues... ")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        try adapter.activate()

        try await withCheckedThrowingContinuation { continuation in
            do {
                try communicator.getClientDispatchQueue().async {
                    continuation.resume()
                }
            } catch {
                continuation.resume(throwing: error)
            }

        }

        try await withCheckedThrowingContinuation { continuation in
            do {
                try communicator.getServerDispatchQueue().async {
                    continuation.resume()
                }
            } catch {
                continuation.resume(throwing: error)
            }

        }

        try await withCheckedThrowingContinuation { continuation in
            do {
                try adapter.getDispatchQueue().async {
                    continuation.resume()
                }
            } catch {
                continuation.resume(throwing: error)
            }
        }

        output.writeLine("ok")
    }
}
