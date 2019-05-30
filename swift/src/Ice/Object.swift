//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

public protocol Object {
    /// Returns the Slice type ID of the most-derived interface supported by this object.
    ///
    /// - parameter current: `Ice.Current` - The Current object for the invocation.
    ///
    /// - returns: `String` - The Slice type ID of the most-derived interface.
    func ice_id(current: Current) throws -> String

    /// Returns the Slice type IDs of the interfaces supported by this object.
    ///
    /// - parameter current: `Ice.Current` - The Current object for the invocation.
    ///
    /// - returns: `[String]` The Slice type IDs of the interfaces supported by this object, in base-to-derived
    ///   order. The first element of the returned array is always `::Ice::Object`.
    func ice_ids(current: Current) throws -> [String]

    /// Tests whether this object supports a specific Slice interface.
    ///
    /// - parameter s: `String` - The type ID of the Slice interface to test against.
    ///
    /// - parameter current: `Ice.Current` - The Current object for the invocation.
    ///
    /// - returns: `Bool` - True if this object has the interface specified by s or
    ///   derives from the interface specified by s.
    func ice_isA(s: String, current: Current) throws -> Bool

    /// Tests whether this object can be reached.
    ///
    /// - parameter current: The Current object for the invocation.
    func ice_ping(current: Current) throws

    func _iceDispatch(incoming: Incoming, current: Current) throws
}

public extension Object {
    func ice_id(current _: Current) -> String {
        return "::Ice::Object"
    }

    func ice_ids(current _: Current) -> [String] {
        return ["::Ice::Object"]
    }

    func ice_isA(s: String, current _: Current) -> Bool {
        return s == "::Ice::Object"
    }

    func ice_ping(current _: Current) {
        // Do nothing
    }

    func _iceD_ice_id(incoming inS: Incoming, current: Current) throws {
        try inS.readEmptyParams()

        let returnValue = try ice_id(current: current)

        inS.write { ostr in
            ostr.write(returnValue)
        }
    }

    func _iceD_ice_ids(incoming inS: Incoming, current: Current) throws {
        try inS.readEmptyParams()

        let returnValue = try ice_ids(current: current)

        inS.write { ostr in
            ostr.write(returnValue)
        }
    }

    func _iceD_ice_isA(incoming inS: Incoming, current: Current) throws {
        let ident: String = try inS.read { istr in
            try istr.read()
        }

        let returnValue = try ice_isA(s: ident, current: current)

        inS.write { ostr in
            ostr.write(returnValue)
        }
    }

    func _iceD_ice_ping(incoming inS: Incoming, current: Current) throws {
        try inS.readEmptyParams()
        try ice_ping(current: current)
        inS.writeEmptyParams()
    }

    func _iceDispatch(incoming: Incoming, current: Current) throws {
        switch current.operation {
        case "ice_id":
            try _iceD_ice_id(incoming: incoming, current: current)
        case "ice_ids":
            try _iceD_ice_ids(incoming: incoming, current: current)
        case "ice_isA":
            try _iceD_ice_isA(incoming: incoming, current: current)
        case "ice_ping":
            try _iceD_ice_ping(incoming: incoming, current: current)
        default:
            throw OperationNotExistException(id: current.id, facet: current.facet, operation: current.operation)
        }
    }
}
