// Copyright (c) ZeroC, Inc.

import IceImpl

final class EndpointI: LocalObject<ICEEndpoint>, Endpoint {
    func toString() -> String {
        return handle.toString()
    }

    func getInfo() -> EndpointInfo? {
        return handle.getInfo() as? EndpointInfo
    }

    // CustomStringConvertible implementation
    var description: String {
        return toString()
    }
}

public func != (lhs: Endpoint?, rhs: Endpoint?) -> Bool {
    return !(lhs == rhs)
}

public func == (lhs: Endpoint?, rhs: Endpoint?) -> Bool {
    if lhs === rhs {
        return true
    } else if lhs === nil && rhs === nil {
        return true
    } else if lhs === nil || rhs === nil {
        return false
    } else {
        let lhsI = lhs as! EndpointI
        let rhsI = rhs as! EndpointI
        return lhsI.handle.isEqual(rhsI.handle)
    }
}

//
// Internal helpers to convert from ObjC to Swift objects
//
extension Array where Element == ICEEndpoint {
    func fromObjc() -> EndpointSeq {
        return map { objcEndpt in
            objcEndpt.getSwiftObject(EndpointI.self) {
                EndpointI(handle: objcEndpt)
            }
        }
    }
}

extension Array where Element == Endpoint {
    func toObjc() -> [ICEEndpoint] {
        return map { endpt in
            (endpt as! EndpointI).handle
        }
    }
}
