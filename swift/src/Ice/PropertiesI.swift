// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class PropertiesI: LocalObject<ICEProperties>, Properties {
    public func getProperty(_ key: String) -> String {
        return _handle.getProperty(key)
    }

    public func getPropertyWithDefault(key: String, value: String) -> String {
        return _handle.getPropertyWithDefault(key, value: value)
    }

    public func getPropertyAsInt(_ key: String) -> Int32 {
        return _handle.getPropertyAsInt(key)
    }

    public func getPropertyAsIntWithDefault(key: String, value: Int32) -> Int32 {
        return _handle.getPropertyAsInt(withDefault: key, value: value)
    }

    public func getPropertyAsList(_ key: String) -> StringSeq {
        return _handle.getPropertyAsList(key)
    }

    public func getPropertyAsListWithDefault(key: String, value: StringSeq) -> StringSeq {
        return _handle.getPropertyAsList(withDefault: key, value: value)
    }

    public func getPropertiesForPrefix(_ prefix: String) -> PropertyDict {
        return _handle.getPropertiesForPrefix(prefix)
    }

    public func setProperty(key: String, value: String) throws {
        try autoreleasepool {
            try _handle.setProperty(key, value: value)
        }
    }

    public func getCommandLineOptions() -> StringSeq {
        return _handle.getCommandLineOptions()
    }

    public func parseCommandLineOptions(prefix: String, options: StringSeq) throws -> StringSeq {
        return try autoreleasepool {
            try _handle.parseCommandLineOptions(prefix, options: options)
        }
    }

    public func parseIceCommandLineOptions(_ options: StringSeq) throws -> StringSeq {
        return try autoreleasepool {
            try _handle.parseIceCommandLineOptions(options)
        }
    }

    public func load(_ file: String) throws {
        return try autoreleasepool {
            try _handle.load(file)
        }
    }

    public func clone() -> Properties {
        return PropertiesI(handle: _handle.clone())
    }
}
