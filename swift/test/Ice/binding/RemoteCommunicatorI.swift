//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

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
                try communicator.getProperties().setProperty(key: "\(name).ThreadPool.Size", value: "1")
                let adapter = try communicator.createObjectAdapterWithEndpoints(name: name, endpoints: endpoints)
                return try uncheckedCast(prx: current.adapter!.addWithUUID(RemoteObjectAdapterI(adapter)),
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
