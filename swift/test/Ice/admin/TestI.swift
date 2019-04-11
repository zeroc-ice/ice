//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Foundation

class TestFacetI: TestFacet {
    func op(current: Ice.Current) {
    }
}

class RemoteCommunicatorI: RemoteCommunicator {
    var _communicator: Ice.Communicator
    var _changes: [String: String] = [:]
    var _lock = os_unfair_lock()

    init(communicator: Ice.Communicator) {
        _communicator = communicator
    }

    func getAdmin(current: Ice.Current) throws -> Ice.ObjectPrx? {
        return try _communicator.getAdmin()
    }

    func getChanges(current: Ice.Current) throws -> [String: String] {
        return withLock(&_lock) {
            return _changes
        }
    }

    func print(message: String, current: Ice.Current) throws {
        _communicator.getLogger().print(message)
    }

    func trace(category: String, message: String, current: Ice.Current) throws {
        _communicator.getLogger().trace(category: category, message: message)
    }

    func warning(message: String, current: Ice.Current) throws {
        _communicator.getLogger().warning(message)
    }

    func error(message: String, current: Ice.Current) throws {
        _communicator.getLogger().error(message)
    }

    func shutdown(current: Ice.Current) throws {
        _communicator.shutdown()
    }

    func waitForShutdown(current: Ice.Current) throws {
        //
        // Note that we are executing in a thread of the *main* communicator,
        // not the one that is being shut down.
        //
        _communicator.waitForShutdown()
    }

    func destroy(current: Ice.Current) throws {
        _communicator.destroy()
    }

    func updated(changes: [String: String]) {
        withLock(&_lock) {
            _changes = changes
        }
    }
}

class NullLogger: Ice.Logger {
    func print(_ message: String) {
    }

    func trace(category: String, message: String) {
    }

    func warning(_ message: String) {
    }

    func error(_ message: String) {
    }

    func getPrefix() -> String {
        return "NullLogger"
    }

    func cloneWithPrefix(_ prefix: String) -> Logger {
        return self
    }
}

class RemoteCommunicatorFactoryI: RemoteCommunicatorFactory {
    func createCommunicator(props: [String: String], current: Ice.Current) throws -> RemoteCommunicatorPrx? {
        //
        // Prepare the property set using the given properties.
        //
        let (properties, _) = try Ice.createProperties()
        for (key, value) in props {
            try properties.setProperty(key: key, value: value)
        }

        var initData = Ice.InitializationData()
        initData.properties = properties
        if properties.getPropertyAsInt("NullLogger") > 0 {
            initData.logger = NullLogger()
        }

        //
        // Initialize a new communicator.
        //
        let communicator = try Ice.initialize(initData: initData)

        //
        // Install a custom admin facet.
        //
        try communicator.addAdminFacet(servant: TestFacetI(), facet: "TestFacet")

        //
        // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
        // Set the callback on the admin facet.
        //
        let servant = RemoteCommunicatorI(communicator: communicator)
        let propFacet = try communicator.findAdminFacet("Properties") as? NativePropertiesAdmin

        if propFacet != nil {
            _ = propFacet!.addUpdateCallback({ changes in
                servant.updated(changes: changes)
            })
        }
        return try uncheckedCast(prx: current.adapter!.addWithUUID(servant), type: RemoteCommunicatorPrx.self)
    }

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
