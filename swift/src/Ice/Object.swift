// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

public protocol Object {
    func ice_id(current: Current) throws -> String
    func ice_ids(current: Current) throws -> [String]
    func ice_isA(s: String, current: Current) throws -> Bool
    func ice_ping(current: Current) throws

    func iceDispatch(incoming: Incoming, current: Current) throws
}

public extension Object {
    func ice_id(current: Current) throws -> String {
        return "::Ice::Object"
    }

    func ice_ids(current: Current) throws -> [String] {
        return ["::Ice::Object"]
    }

    func ice_isA(s: String, current: Current) throws -> Bool {
        return s == "::Ice::Object"
    }

    func ice_ping(current: Current) throws {
        // Do nothing
    }

    func iceD_ice_id(incoming inS: Incoming, current: Current) throws {
        try inS.readEmptyParams()

        let returnValue = try self.ice_id(current: current)

        inS.write { ostr in
            ostr.write(returnValue)
        }
    }

    func iceD_ice_ids(incoming inS: Incoming, current: Current) throws {
        try inS.readEmptyParams()

        let returnValue = try self.ice_ids(current: current)

        inS.write { ostr in
            ostr.write(returnValue)
        }
    }

    func iceD_ice_isA(incoming inS: Incoming, current: Current) throws {
        let ident: String = try inS.read { istr in
            return try istr.read()
        }

        let returnValue = try self.ice_isA(s: ident, current: current)

        inS.write { ostr in
            ostr.write(returnValue)
        }
    }

    func iceD_ice_ping(incoming inS: Incoming, current: Current) throws {
        try inS.readEmptyParams()
        try self.ice_ping(current: current)
        inS.writeEmptyParams()
    }

    func iceDispatch(incoming: Incoming, current: Current) throws {
        switch current.operation {
        case "ice_id":
            try iceD_ice_id(incoming: incoming, current: current)
        case "ice_ids":
            try iceD_ice_ids(incoming: incoming, current: current)
        case "ice_isA":
            try iceD_ice_isA(incoming: incoming, current: current)
        case "ice_ping":
            try iceD_ice_ping(incoming: incoming, current: current)
        default:
            throw OperationNotExistException(id: current.id, facet: current.facet, operation: current.operation)
        }
    }
}
