// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public protocol Exception: Error {}

public class LocalException: Exception {}

public protocol UserException: Exception, Streamable, CustomStringConvertible {
    var description: String { get }

    static func ice_staticId() -> String
}

public extension UserException {
    var description: String {
        return Self.ice_staticId()
    }

    internal func isBase(of ex: UserException) -> Bool {
        return ex is Self
    }
}

#warning("TODO: Add proper LocalException  CustomStringConvertible impl")
// TODO: All LocalExceptions should be CustomStringConvertible and print detailed messages
// like C++ (by calling IceUtilInternal::errorToString from ObjC)
