//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class TestFacetI: TestFacet {
    func op(current _: Ice.Current) {}
}

class RemoteCommunicatorI: RemoteCommunicator {
    var _communicator: Ice.Communicator
    var _changes: [String: String] = [:]
    var _lock = os_unfair_lock()

    init(communicator: Ice.Communicator) {
        _communicator = communicator
    }

    func getAdmin(current _: Ice.Current) throws -> Ice.ObjectPrx? {
        return try _communicator.getAdmin()
    }

    func getChanges(current _: Ice.Current) throws -> [String: String] {
        return withLock(&_lock) {
            _changes
        }
    }

    func print(message: String, current _: Ice.Current) throws {
        _communicator.getLogger().print(message)
    }

    func trace(category: String, message: String, current _: Ice.Current) throws {
        _communicator.getLogger().trace(category: category, message: message)
    }

    func warning(message: String, current _: Ice.Current) throws {
        _communicator.getLogger().warning(message)
    }

    func error(message: String, current _: Ice.Current) throws {
        _communicator.getLogger().error(message)
    }

    func shutdown(current _: Ice.Current) throws {
        _communicator.shutdown()
    }

    func waitForShutdown(current _: Ice.Current) throws {
        //
        // Note that we are executing in a thread of the *main* communicator,
        // not the one that is being shut down.
        //
        _communicator.waitForShutdown()
    }

    func destroy(current _: Ice.Current) throws {
        _communicator.destroy()
    }

    func updated(changes: [String: String]) {
        withLock(&_lock) {
            _changes = changes
        }
    }
}

class NullLogger: Ice.Logger {
    func print(_: String) {}

    func trace(category _: String, message _: String) {}

    func warning(_: String) {}

    func error(_: String) {}

    func getPrefix() -> String {
        return "NullLogger"
    }

    func cloneWithPrefix(_: String) -> Logger {
        return self
    }
}

class RemoteCommunicatorFactoryI: RemoteCommunicatorFactory {
    func createCommunicator(props: [String: String], current: Ice.Current) throws -> RemoteCommunicatorPrx? {
        //
        // Prepare the property set using the given properties.
        //
        let properties = Ice.createProperties()
        for (key, value) in props {
            properties.setProperty(key: key, value: value)
        }

        var initData = Ice.InitializationData()
        initData.properties = properties
        if properties.getPropertyAsInt("NullLogger") > 0 {
            initData.logger = NullLogger()
        }

        //
        // Initialize a new communicator.
        //
        let communicator = try Ice.initialize(initData)

        //
        // Install a custom admin facet.
        //
        try communicator.addAdminFacet(servant: TestFacetDisp(TestFacetI()), facet: "TestFacet")

        //
        // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        let servant = RemoteCommunicatorI(communicator: communicator)

        if let propDisp = communicator.findAdminFacet("Properties") as? PropertiesAdminDisp,
            let propFacet = propDisp.servant as? NativePropertiesAdmin {
            _ = propFacet.addUpdateCallback { changes in
                servant.updated(changes: changes)
            }
        }
        return try uncheckedCast(prx: current.adapter!.addWithUUID(RemoteCommunicatorDisp(servant)),
                                 type: RemoteCommunicatorPrx.self)
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
