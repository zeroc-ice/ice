// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

class ServantManager {
    let adapterName: String
    let communicator: Communicator

    var servantMapMap = [Identity: [String: Object]]()
    var defaultServantMap = [String: Object]()
    var locatorMap = [String: ServantLocator]()

    // This is used to distingish between ObjectNotExistException and FacetNotExistException
    // when a servant is not found on a Swift Admin OA.
    var adminId: Identity?

    var mutex = Mutex()

    init(adapterName: String, communicator: Communicator) {
        self.adapterName = adapterName
        self.communicator = communicator
    }

    func addServant(servant: Object, id ident: Identity, facet: String) throws {
        try mutex.sync {
            if var m = servantMapMap[ident] {
                if m[facet] != nil {
                    var id = try communicator.identityToString(ident)
                    if !facet.isEmpty {
                        id += try " - f " + escapeString(string: facet, special: "", communicator: communicator)
                    }
                    throw AlreadyRegisteredException(kindOfObject: "servant", id: id)
                }
                m[facet] = servant
            } else {
                servantMapMap[ident] = [facet: servant]
            }
        }
    }

    func addDefaultServant(servant: Object, category: String) throws {
        try mutex.sync {
            guard defaultServantMap[category] == nil else {
                throw AlreadyRegisteredException(kindOfObject: "default servant", id: category)
            }

            defaultServantMap[category] = servant
        }
    }

    func removeServant(id ident: Identity, facet: String) throws -> Object {
        return try mutex.sync {
            guard var m = servantMapMap[ident], let obj = m.removeValue(forKey: facet) else {
                var id = try communicator.identityToString(ident)
                if !facet.isEmpty {
                    id += try " - f " + escapeString(string: facet, special: "", communicator: communicator)
                }
                throw NotRegisteredException(kindOfObject: "servant", id: id)
            }

            if m.isEmpty {
                servantMapMap.removeValue(forKey: ident)
            }

            return obj
        }
    }

    func removeDefaultServant(category: String) throws -> Object {
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
                throw NotRegisteredException(kindOfObject: "servant", id: try identityToString(id: id))
            }

            return m
        }
    }

    func findServant(id: Identity, facet: String) -> Object? {
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

    func findDefaultServant(category: String) -> Object? {
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
                let id = try escapeString(string: category, special: "", communicator: communicator)
                throw AlreadyRegisteredException(kindOfObject: "servant locator", id: id)
            }

            locatorMap[category] = locator
        }
    }

    func removeServantLocator(category: String) throws -> ServantLocator {
        return try mutex.sync {
            guard let l = locatorMap[category] else {
                let id = try escapeString(string: category, special: "", communicator: communicator)
                throw NotRegisteredException(kindOfObject: "servant locator", id: id)
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

        m.forEach { category, locator in
            do {
                try locator.deactivate(category)
            } catch {
                let logger = communicator.getLogger()
                logger.error("""
                exception during locator deactivation:
                object adapter: `\(adapterName)'
                locator category: `\(category)'
                \(error)
                """)
            }
        }
    }
}
