//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class TestI: TestIntf {
    func getAdapterName(current: Ice.Current) throws -> String {
        return current.adapter!.getName()
    }
}

class RemoteCommunicatorI: RemoteCommunicator {
    var _nextPort: Int32 = 10
    var _helper: TestHelper

    init(helper: TestHelper) {
        _helper = helper
    }

    func createObjectAdapter(name: String,
                             endpoints endpts: String,
                             current: Ice.Current) throws -> RemoteObjectAdapterPrx? {
        var retry = 5
        while true {
            do {
                let communicator = current.adapter!.getCommunicator()
                var endpoints = endpts
                if !endpoints.contains("-p") {
                    _nextPort += 1
                    endpoints = _helper.getTestEndpoint(properties: communicator.getProperties(), num: _nextPort,
                                                        prot: endpoints)
                }
                communicator.getProperties().setProperty(key: "\(name).ThreadPool.Size", value: "1")
                let adapter = try communicator.createObjectAdapterWithEndpoints(name: name, endpoints: endpoints)
                return try uncheckedCast(
                    prx: current.adapter!.addWithUUID(RemoteObjectAdapterDisp(RemoteObjectAdapterI(adapter))),
                    type: RemoteObjectAdapterPrx.self)
            } catch let ex as Ice.SocketException {
                retry -= 1
                if retry == 0 {
                    throw ex
                }
            }
        }
    }

    func deactivateObjectAdapter(adapter: RemoteObjectAdapterPrx?, current: Ice.Current) throws {
        try adapter!.deactivate() // Collocated call.
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class RemoteObjectAdapterI: RemoteObjectAdapter {
    init(_ adapter: Ice.ObjectAdapter ) throws {
        _adapter = adapter
        _testIntf = try uncheckedCast(prx: _adapter.add(servant: TestIntfDisp(TestI()),
                                                        id: Ice.stringToIdentity("test")),
                                      type: TestIntfPrx.self)
        try _adapter.activate()
    }

    func getTestIntf(current: Ice.Current) throws -> TestIntfPrx? {
        return _testIntf
    }

    func deactivate(current: Ice.Current) throws {
        _adapter.destroy()
    }

    var _adapter: Ice.ObjectAdapter
    var _testIntf: TestIntfPrx
}
