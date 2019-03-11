// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ValueFactoryManagerI: ValueFactoryManager {

    var factories = [String: ValueFactory]()
    var mutex = Mutex()

    func add(factory: @escaping ValueFactory, id: String) throws {
        try mutex.sync {
            if factories[id] != nil {
                throw AlreadyRegisteredException(kindOfObject: "value factory", id: id, file: #file, line: #line)
            }
            factories[id] = factory
        }
    }

    func find(id: String) -> ValueFactory? {
        return mutex.sync {
            factories[id]
        }
    }
}
