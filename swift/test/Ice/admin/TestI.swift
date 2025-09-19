// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

final class TestFacetI: TestFacet, Sendable {
    func op(current _: Ice.Current) {}
}

actor Changes {
    private var changes: [String: String] = [:]

    func getChanges() -> [String: String] {
        return changes
    }
    func setChanges(_ newChanges: [String: String]) {
        changes = newChanges
    }
}

final class RemoteCommunicatorI: RemoteCommunicator {
    let _communicator: Ice.Communicator
    let _changes: Changes = Changes()

    init(communicator: Ice.Communicator) {
        _communicator = communicator
    }

    func getAdmin(current _: Ice.Current) throws -> Ice.ObjectPrx? {
        return try _communicator.getAdmin()
    }

    func getChanges(current _: Ice.Current) async -> [String: String] {
        return await _changes.getChanges()
    }

    func print(message: String, current _: Ice.Current) {
        _communicator.getLogger().print(message)
    }

    func trace(category: String, message: String, current _: Ice.Current) {
        _communicator.getLogger().trace(category: category, message: message)
    }

    func warning(message: String, current _: Ice.Current) {
        _communicator.getLogger().warning(message)
    }

    func error(message: String, current _: Ice.Current) {
        _communicator.getLogger().error(message)
    }

    func shutdown(current _: Ice.Current) {
        _communicator.shutdown()
    }

    func waitForShutdown(current _: Ice.Current) {
        //
        // Note that we are executing in a thread of the *main* communicator,
        // not the one that is being shut down.
        //
        _communicator.waitForShutdown()
    }

    func destroy(current _: Ice.Current) {
        _communicator.destroy()
    }

    func updated(changes: [String: String]) {
        Task { [_changes] in await _changes.setChanges(changes) }
    }
}

final class NullLogger: Ice.Logger {
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

final class RemoteCommunicatorFactoryI: RemoteCommunicatorFactory {
    func createCommunicator(props: [String: String], current: Ice.Current) throws -> RemoteCommunicatorPrx? {
        //
        // Prepare the property set using the given properties.
        //
        let properties = Ice.createProperties()
        for (key, value) in props {
            properties.setProperty(key: key, value: value)
        }

        var initData = Ice.InitializationData(properties: properties)
        if try properties.getPropertyAsInt("NullLogger") > 0 {
            initData.logger = NullLogger()
        }

        //
        // Initialize a new communicator.
        //
        let communicator = try Ice.initialize(initData)

        //
        // Install a custom admin facet.
        //
        try communicator.addAdminFacet(servant: TestFacetI(), facet: "TestFacet")

        //
        // Set the callback on the admin facet.
        //
        let servant = RemoteCommunicatorI(communicator: communicator)

        if let propFacet = communicator.findAdminFacet("Properties") as? NativePropertiesAdmin {
            _ = propFacet.addUpdateCallback { changes in
                servant.updated(changes: changes)
            }
        }
        return try uncheckedCast(
            prx: current.adapter.addWithUUID(servant),
            type: RemoteCommunicatorPrx.self)
    }

    func shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown()
    }
}
