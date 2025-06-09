// Copyright (c) ZeroC, Inc.

import Synchronization

final class ServantManager: Dispatcher {

    private struct State {
        var servantMapMap = [Identity: [String: Dispatcher]]()
        var defaultServantMap = [String: Dispatcher]()
        var locatorMap = [String: ServantLocator]()
        var adminId: Identity?
    }

    private let adapterName: String
    private let communicator: Communicator
    private let state = Mutex<State>(State())

    init(adapterName: String, communicator: Communicator) {
        self.adapterName = adapterName
        self.communicator = communicator
    }

    func addServant(servant: Dispatcher, id ident: Identity, facet: String) throws {
        try state.withLock {
            if var m = $0.servantMapMap[ident] {
                if m[facet] != nil {
                    var id = communicator.identityToString(ident)
                    if !facet.isEmpty {
                        id += " -f \(facet)"
                    }
                    throw AlreadyRegisteredException(kindOfObject: "servant", id: id)
                }
                m[facet] = servant
                $0.servantMapMap[ident] = m
            } else {
                $0.servantMapMap[ident] = [facet: servant]
            }
        }
    }

    func addDefaultServant(servant: Dispatcher, category: String) throws {
        try state.withLock {
            guard $0.defaultServantMap[category] == nil else {
                throw AlreadyRegisteredException(kindOfObject: "default servant", id: category)
            }

            $0.defaultServantMap[category] = servant
        }
    }

    func removeServant(id ident: Identity, facet: String) throws -> Dispatcher {
        return try state.withLock {
            guard var m = $0.servantMapMap[ident], let obj = m.removeValue(forKey: facet) else {
                var id = communicator.identityToString(ident)
                if !facet.isEmpty {
                    id += " -f \(facet)"
                }
                throw NotRegisteredException(kindOfObject: "servant", id: id)
            }

            if m.isEmpty {
                $0.servantMapMap.removeValue(forKey: ident)
            } else {
                $0.servantMapMap[ident] = m
            }
            return obj
        }
    }

    func removeDefaultServant(category: String) throws -> Dispatcher {
        return try state.withLock {
            guard let obj = $0.defaultServantMap.removeValue(forKey: category) else {
                throw NotRegisteredException(kindOfObject: "default servant", id: category)
            }

            return obj
        }
    }

    func removeAllFacets(id: Identity) throws -> FacetMap {
        return try state.withLock {
            guard let m = $0.servantMapMap.removeValue(forKey: id) else {
                throw NotRegisteredException(kindOfObject: "servant", id: identityToString(id: id))
            }

            return m
        }
    }

    func findServant(id: Identity, facet: String) -> Dispatcher? {
        return state.withLock {
            guard let m = $0.servantMapMap[id] else {
                guard let obj = $0.defaultServantMap[id.category] else {
                    return $0.defaultServantMap[""]
                }

                return obj
            }

            return m[facet]
        }
    }

    func findDefaultServant(category: String) -> Dispatcher? {
        return state.withLock {
            $0.defaultServantMap[category]
        }
    }

    func findAllFacets(id: Identity) -> FacetMap {
        return state.withLock {
            guard let m = $0.servantMapMap[id] else {
                return FacetMap()
            }

            return m
        }
    }

    func hasServant(id: Identity) -> Bool {
        return state.withLock {
            $0.servantMapMap[id] != nil
        }
    }

    func addServantLocator(locator: ServantLocator, category: String) throws {
        return try state.withLock {
            guard $0.locatorMap[category] == nil else {
                throw AlreadyRegisteredException(kindOfObject: "servant locator", id: category)
            }

            $0.locatorMap[category] = locator
        }
    }

    func removeServantLocator(category: String) throws -> ServantLocator {
        return try state.withLock {
            guard let l = $0.locatorMap.removeValue(forKey: category) else {
                throw NotRegisteredException(kindOfObject: "servant locator", id: category)
            }

            return l
        }
    }

    func findServantLocator(category: String) -> ServantLocator? {
        return state.withLock {
            $0.locatorMap[category]
        }
    }

    func setAdminId(_ id: Identity) {
        state.withLock {
            $0.adminId = id
        }
    }

    func isAdminId(_ id: Identity) -> Bool {
        return state.withLock {
            $0.adminId == id
        }
    }

    func destroy() {
        var m = [String: ServantLocator]()
        state.withLock {
            $0.servantMapMap.removeAll()
            $0.defaultServantMap.removeAll()
            m = $0.locatorMap
            $0.locatorMap.removeAll()
        }

        for (category, locator) in m {
            locator.deactivate(category)
        }
    }

    func dispatch(_ request: sending IncomingRequest) async throws -> OutgoingResponse {
        let current = request.current
        var servant = findServant(id: current.id, facet: current.facet)

        if let servant = servant {
            // the simple, common path
            return try await servant.dispatch(request)
        }

        // Else, check servant locators

        var locator = findServantLocator(category: current.id.category)
        if locator == nil, !current.id.category.isEmpty {
            locator = findServantLocator(category: "")
        }

        if let locator = locator {
            var cookie: AnyObject?
            (servant, cookie) = try locator.locate(current)

            if let servant = servant {
                let response: OutgoingResponse
                do {
                    response = try await servant.dispatch(request)
                } catch {
                    response = current.makeOutgoingResponse(error: error)
                }

                // If the locator returned a servant, we must execute finished once no matter what.
                try locator.finished(curr: current, servant: servant, cookie: cookie)

                return response
            }
        }

        if hasServant(id: current.id) || isAdminId(current.id) {
            throw FacetNotExistException()
        } else {
            throw ObjectNotExistException()
        }
    }
}
