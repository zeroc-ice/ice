// Copyright (c) ZeroC, Inc.

final class ServantManager: Dispatcher {
    private let adapterName: String
    private let communicator: Communicator

    private var servantMapMap = [Identity: [String: Dispatcher]]()
    private var defaultServantMap = [String: Dispatcher]()
    private var locatorMap = [String: ServantLocator]()

    // This is used to distinguish between ObjectNotExistException and FacetNotExistException
    // when a servant is not found on a Swift Admin OA.
    private var adminId: Identity?

    private var mutex = Mutex()

    init(adapterName: String, communicator: Communicator) {
        self.adapterName = adapterName
        self.communicator = communicator
    }

    func addServant(servant: Dispatcher, id ident: Identity, facet: String) throws {
        try mutex.sync {
            if var m = servantMapMap[ident] {
                if m[facet] != nil {
                    var id = communicator.identityToString(ident)
                    if !facet.isEmpty {
                        id += " -f \(facet)"
                    }
                    throw AlreadyRegisteredException(kindOfObject: "servant", id: id)
                }
                m[facet] = servant
                servantMapMap[ident] = m
            } else {
                servantMapMap[ident] = [facet: servant]
            }
        }
    }

    func addDefaultServant(servant: Dispatcher, category: String) throws {
        try mutex.sync {
            guard defaultServantMap[category] == nil else {
                throw AlreadyRegisteredException(kindOfObject: "default servant", id: category)
            }

            defaultServantMap[category] = servant
        }
    }

    func removeServant(id ident: Identity, facet: String) throws -> Dispatcher {
        return try mutex.sync {
            guard var m = servantMapMap[ident], let obj = m.removeValue(forKey: facet) else {
                var id = communicator.identityToString(ident)
                if !facet.isEmpty {
                    id += " -f \(facet)"
                }
                throw NotRegisteredException(kindOfObject: "servant", id: id)
            }

            if m.isEmpty {
                servantMapMap.removeValue(forKey: ident)
            } else {
                servantMapMap[ident] = m
            }
            return obj
        }
    }

    func removeDefaultServant(category: String) throws -> Dispatcher {
        return try mutex.sync {
            guard let obj = defaultServantMap.removeValue(forKey: category) else {
                throw NotRegisteredException(kindOfObject: "default servant", id: category)
            }

            return obj
        }
    }

    func removeAllFacets(id: Identity) throws -> FacetMap {
        return try mutex.sync {
            guard let m = servantMapMap.removeValue(forKey: id) else {
                throw NotRegisteredException(kindOfObject: "servant", id: identityToString(id: id))
            }

            return m
        }
    }

    func findServant(id: Identity, facet: String) -> Dispatcher? {
        return mutex.sync {
            guard let m = servantMapMap[id] else {
                guard let obj = defaultServantMap[id.category] else {
                    return defaultServantMap[""]
                }

                return obj
            }

            return m[facet]
        }
    }

    func findDefaultServant(category: String) -> Dispatcher? {
        return mutex.sync {
            defaultServantMap[category]
        }
    }

    func findAllFacets(id: Identity) -> FacetMap {
        return mutex.sync {
            guard let m = servantMapMap[id] else {
                return FacetMap()
            }

            return m
        }
    }

    func hasServant(id: Identity) -> Bool {
        return mutex.sync {
            servantMapMap[id] != nil
        }
    }

    func addServantLocator(locator: ServantLocator, category: String) throws {
        return try mutex.sync {
            guard locatorMap[category] == nil else {
                throw AlreadyRegisteredException(kindOfObject: "servant locator", id: category)
            }

            locatorMap[category] = locator
        }
    }

    func removeServantLocator(category: String) throws -> ServantLocator {
        return try mutex.sync {
            guard let l = locatorMap.removeValue(forKey: category) else {
                throw NotRegisteredException(kindOfObject: "servant locator", id: category)
            }

            return l
        }
    }

    func findServantLocator(category: String) -> ServantLocator? {
        return mutex.sync {
            locatorMap[category]
        }
    }

    func setAdminId(_ id: Identity) {
        mutex.sync {
            adminId = id
        }
    }

    func isAdminId(_ id: Identity) -> Bool {
        return mutex.sync {
            adminId == id
        }
    }

    func destroy() {
        var m = [String: ServantLocator]()
        mutex.sync {
            servantMapMap.removeAll()
            defaultServantMap.removeAll()
            m = locatorMap
            locatorMap.removeAll()
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
