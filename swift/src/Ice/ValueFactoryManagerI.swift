// Copyright (c) ZeroC, Inc.

class ValueFactoryManagerI: ValueFactoryManager {
    private var factories = [String: ValueFactory]()
    private var mutex = Mutex()

    func add(factory: @escaping ValueFactory, id: String) throws {
        try mutex.sync {
            if factories[id] != nil {
                throw AlreadyRegisteredException(kindOfObject: "value factory", id: id)
            }
            factories[id] = factory
        }
    }

    func find(_ id: String) -> ValueFactory? {
        return mutex.sync {
            factories[id]
        }
    }
}
