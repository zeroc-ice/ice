// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon


struct DispatchWrapper: Dispatcher {
    let dispatcher: Dispatcher

    func dispatch(_ request: sending IncomingRequest) async throws -> OutgoingResponse {
        let startTime = ContinuousClock.now
        let response = try await dispatcher.dispatch(request)
        let endTime = ContinuousClock.now
        let elapsed = startTime.duration(to: endTime)

        let logger = request.current.adapter.getCommunicator().getLogger()
        logger.print("Swift Dispatch: identity=\(request.current.id) operation=\(request.current.operation) duration=\(Double(elapsed.components.attoseconds) / 1_000_000_000_000_000.0) ms")

        return response
    }
}

class Server: TestHelperI, @unchecked Sendable {
    override public func run(args: [String]) async throws {
        //
        // Register the server manager. The server manager creates a new
        // 'server'(a server isn't a different process, it's just a new
        // communicator and object adapter).
        //

        let properties = try createTestProperties(args)
        properties.setProperty(key: "Ice.ThreadPool.Server.Size", value: "2")
        let communicator = try initialize(Ice.InitializationData(properties: properties))
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
            key: "ServerManager.Endpoints", value: getTestEndpoint(num: 0))
        let adapter = try communicator.createObjectAdapter("ServerManager")

        //
        // We also register a sample server locator which implements the
        // locator interface, this locator is used by the clients and the
        // 'servers' created with the server manager interface.
        //
        let registry = ServerLocatorRegistry()

        // Make sure we use a separate copy of the properties as the servant modifies them.
        let object = ServerManagerI(registry: registry, properties: properties.clone(), helper: self)

        try adapter.add(servant: DispatchWrapper(dispatcher: object), id: Ice.stringToIdentity("ServerManager"))
        try await registry.addObject(adapter.createProxy(Ice.stringToIdentity("ServerManager")))

        let registryPrx = try uncheckedCast(
            prx: adapter.add(
                servant: registry,
                id: Ice.stringToIdentity("registry")),
            type: Ice.LocatorRegistryPrx.self)

        let locator = ServerLocator(registry: registry, registryPrx: registryPrx)
        try adapter.add(servant: DispatchWrapper(dispatcher: locator), id: Ice.stringToIdentity("locator"))

        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
