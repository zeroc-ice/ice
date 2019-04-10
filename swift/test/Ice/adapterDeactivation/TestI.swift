//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import Foundation

class TestI: TestIntf {
    init() {}

    func transient(current: Ice.Current) throws {
        let communicator = current.adapter!.getCommunicator()

        let adapter = try communicator.createObjectAdapterWithEndpoints(name: "TransientTestAdapter", endpoints: "default")
        try adapter.activate()
        adapter.destroy()
    }
    
    func deactivate(current: Ice.Current) throws {
        current.adapter!.deactivate()
        Thread.sleep(forTimeInterval: 0.1)
    }
}
