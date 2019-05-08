//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

class PropertiesI: LocalObject<ICEProperties>, Properties {
    public func getProperty(_ key: String) -> String {
        return handle.getProperty(key)
    }

    public func getPropertyWithDefault(key: String, value: String) -> String {
        return handle.getPropertyWithDefault(key, value: value)
    }

    public func getPropertyAsInt(_ key: String) -> Int32 {
        return handle.getPropertyAsInt(key)
    }

    public func getPropertyAsIntWithDefault(key: String, value: Int32) -> Int32 {
        return handle.getPropertyAsIntWithDefault(key: key, value: value)
    }

    public func getPropertyAsList(_ key: String) -> StringSeq {
        return handle.getPropertyAsList(key)
    }

    public func getPropertyAsListWithDefault(key: String, value: StringSeq) -> StringSeq {
        return handle.getPropertyAsListWithDefault(key: key, value: value)
    }

    public func getPropertiesForPrefix(_ prefix: String) -> PropertyDict {
        return handle.getPropertiesForPrefix(prefix)
    }

    public func setProperty(key: String, value: String) {
        precondition(!key.isEmpty, "Key cannot be empty")
        do {
            try autoreleasepool {
                try handle.setProperty(key, value: value)
            }
        } catch {
            fatalError("\(error)")
        }
    }

    public func getCommandLineOptions() -> StringSeq {
        return handle.getCommandLineOptions()
    }

    public func parseCommandLineOptions(prefix: String, options: StringSeq) throws -> StringSeq {
        return try autoreleasepool {
            try handle.parseCommandLineOptions(prefix, options: options)
        }
    }

    public func parseIceCommandLineOptions(_ options: StringSeq) throws -> StringSeq {
        return try autoreleasepool {
            try handle.parseIceCommandLineOptions(options)
        }
    }

    public func load(_ file: String) throws {
        return try autoreleasepool {
            try handle.load(file)
        }
    }

    public func clone() -> Properties {
        return PropertiesI(handle: handle.clone())
    }
}
