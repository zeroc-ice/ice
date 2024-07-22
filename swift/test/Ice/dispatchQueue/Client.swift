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

        let semaphore = DispatchSemaphore(value: 0)
        try communicator.getClientDispatchQueue().async {
            semaphore.signal()
        }

        semaphore.wait()
        try communicator.getServerDispatchQueue().async {
            semaphore.signal()
        }

        semaphore.wait()
        try adapter.getDispatchQueue().async {
            semaphore.signal()
        }

        semaphore.wait()
        output.writeLine("ok")
    }
}
