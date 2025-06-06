// Copyright (c) ZeroC, Inc.

import IceImpl

final class PropertiesI: LocalObject<ICEProperties>, Properties {
    public func getProperty(_ key: String) -> String {
        handle.getProperty(key)
    }

    public func getIceProperty(_ key: String) -> String {
        handle.getIceProperty(key)
    }

    public func getPropertyWithDefault(key: String, value: String) -> String {
        handle.getPropertyWithDefault(key, value: value)
    }

    public func getPropertyAsInt(_ key: String) throws -> Int32 {
        return try autoreleasepool {
            var value: Int32 = 0
            try handle.getPropertyAsInt(key, value: &value)
            return value
        }
    }

    public func getIcePropertyAsInt(_ key: String) throws -> Int32 {
        return try autoreleasepool {
            var value: Int32 = 0
            try handle.getIcePropertyAsInt(key: key, value: &value)
            return value
        }
    }

    public func getPropertyAsIntWithDefault(key: String, value defaultValue: Int32) throws -> Int32 {
        return try autoreleasepool {
            var value: Int32 = 0
            try handle.getPropertyAsIntWithDefault(key: key, defaultValue: defaultValue, value: &value)
            return value
        }
    }

    public func getPropertyAsList(_ key: String) -> StringSeq {
        handle.getPropertyAsList(key)
    }

    public func getIcePropertyAsList(_ key: String) -> StringSeq {
        handle.getIcePropertyAsList(key)
    }

    public func getPropertyAsListWithDefault(key: String, value: StringSeq) -> StringSeq {
        handle.getPropertyAsListWithDefault(key: key, value: value)
    }

    public func getPropertiesForPrefix(_ prefix: String) -> PropertyDict {
        handle.getPropertiesForPrefix(prefix)
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
        handle.getCommandLineOptions()
    }

    public func parseCommandLineOptions(prefix: String, options: StringSeq) throws -> StringSeq {
        try autoreleasepool {
            try handle.parseCommandLineOptions(prefix, options: options)
        }
    }

    public func parseIceCommandLineOptions(_ options: StringSeq) throws -> StringSeq {
        try autoreleasepool {
            try handle.parseIceCommandLineOptions(options)
        }
    }

    public func load(_ file: String) throws {
        try autoreleasepool {
            try handle.load(file)
        }
    }

    public func clone() -> Properties {
        PropertiesI(handle: handle.clone())
    }
}
