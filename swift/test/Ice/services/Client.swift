//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Glacier2
import Ice
import IceGrid
import IceStorm
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        var initData = Ice.InitializationData()
        initData.properties = try createTestProperties(args)
        let communicator = try initialize(initData)
        defer {
            communicator.destroy()
        }
        let out = getWriter()

        do {
            out.write("Testing Glacier2 stub... ")
            let router = uncheckedCast(prx: try communicator.stringToProxy("test:\(getTestEndpoint(num: 0))")!,
                                       type: Glacier2.RouterPrx.self)
            do {
                _ = try router.createSession(userId: "foo", password: "bar")
                try test(false)
            } catch is LocalException {
                // expected
            }
            out.writeLine("ok")
        }

        do {
            out.write("Testing IceStorm stub... ")
            let manager = uncheckedCast(prx: try communicator.stringToProxy("test:\(getTestEndpoint(num: 0))")!,
                                                 type: TopicManagerPrx.self)

            let topicName = "time"
            var topic: TopicPrx?
            do {
                topic = try manager.retrieve(topicName)
                try test(false)
            } catch is NoSuchTopic {
                try test(false)
            } catch is LocalException {
                // expected
            }

            // topic is always nil, we're just checking the API compiling/linking here
            let publisher = try topic?.subscribeAndGetPublisher(theQoS: QoS(), subscriber: nil)
            try test(publisher == nil)
            out.writeLine("ok")
        }

        do {
            out.write("Testing IceGrid stub... ")
            let registry = uncheckedCast(prx: try communicator.stringToProxy("test:\(getTestEndpoint(num: 0))")!,
                                        type: RegistryPrx.self)

            var session: AdminSessionPrx?
            do {
                session = try registry.createAdminSession(userId: "foo", password: "bar")
                try test(false)
            } catch is LocalException {
                // expected
            }

            // session is always nil, we're just checking the API
            let admin = try session?.getAdmin()
            try test(admin == nil)
            out.writeLine("ok")
        }
    }
}
