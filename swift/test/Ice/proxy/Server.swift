//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit
import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Server()
    }
}


/*class MyDerivedClassI:  MyDerivedClassDisp_
{
    var _ctx: [String: String]

    public init(){
        _ctx = [String: String]()
    }

    public func echo(obj: Ice.ObjectPrx?, c: Ice.Current) throws -> Ice.ObjectPrx? {
        return obj
    }

    public func shutdown(current: Ice.Current) throws {
        guard let adapter = current.adapter else {
            precondition(false)
        }
        adapter.getCommunicator().shutdown()
    }

    public func getContext(current: Ice.Current) -> [String: String] {
        return _ctx
    }

    public func ice_isA(s: String, current: Ice.Current) {
        _ctx = current.ctx;
        return super.ice_isA(s, current)
    }
}*/

class Server: TestHelperI {
    public override func run(args: [String]) throws {
        /*let writer = getWriter()

        let (properties, _) = try Ice.createProperties(args: args)
        //
        // We don't want connection warnings because of the timeout test.
        //
        //properties.setProperty(key: "Ice.Warn.Connections", value: "0")
        //properties.setProperty(key: "Ice.Warn.Dispatch", value: "0")

        let (communicator, _) = try self.initialize(properties)
        defer {
            communicator.destroy()
        }
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: getTestEndpoint(num: 0))
        var adapter = try communicator.createObjectAdapter("TestAdapter")
        adapter.add(MyDerivedClassI(), Ice.stringToIdentity("test"))
        adapter.activate()
        serverReady()
        communicator.waitForShutdown()*/
    }
}
