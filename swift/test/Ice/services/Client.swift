// Copyright (c) ZeroC, Inc.

import Glacier2
import Ice
import IceGrid
import IceStorm
import TestCommon

public class Client: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let out = getWriter()

        do {
            out.write("Testing Glacier2 stub... ")
            let router = try uncheckedCast(
                prx: communicator.stringToProxy("test:\(getTestEndpoint(num: 0))")!,
                type: Glacier2.RouterPrx.self)
            do {
                _ = try await router.createSession(userId: "foo", password: "bar")
                try test(false)
            } catch is LocalException {
                // expected
            }
            out.writeLine("ok")
        }

        do {
            out.write("Testing IceStorm stub... ")
            let manager = try uncheckedCast(
                prx: communicator.stringToProxy("test:\(getTestEndpoint(num: 0))")!,
                type: TopicManagerPrx.self)

            let topicName = "time"
            var topic: TopicPrx?
            do {
                topic = try await manager.retrieve(topicName)
                try test(false)
            } catch is NoSuchTopic {
                try test(false)
            } catch is LocalException {
                // expected
            }

            // topic is always nil, we're just checking the API compiling/linking here
            let publisher = try await topic?.subscribeAndGetPublisher(
                theQoS: QoS(), subscriber: nil)
            try test(publisher == nil)
            out.writeLine("ok")
        }

        do {
            out.write("Testing IceGrid stub... ")
            let registry = try uncheckedCast(
                prx: communicator.stringToProxy("test:\(getTestEndpoint(num: 0))")!,
                type: RegistryPrx.self)

            var session: AdminSessionPrx?
            do {
                session = try await registry.createAdminSession(userId: "foo", password: "bar")
                try test(false)
            } catch is LocalException {
                // expected
            }

            // session is always nil, we're just checking the API
            let admin = try await session?.getAdmin()
            try test(admin == nil)
            out.writeLine("ok")
        }
    }
}
