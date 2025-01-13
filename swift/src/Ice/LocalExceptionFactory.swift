// Copyright (c) ZeroC, Inc.

import IceImpl

class LocalExceptionFactory: ICELocalExceptionFactory {
    static func requestFailedException(
        _ typeId: String, name: String, category: String, facet: String, operation: String,
        message: String, cxxDescription: String, file: String, line: Int32
    ) -> Error {
        let className = typeId.dropFirst(2).replacingOccurrences(of: "::", with: ".")
        if let requestFailedExceptionType = NSClassFromString(className) as? RequestFailedException.Type {
            return requestFailedExceptionType.init(
                id: Identity(name: name, category: category), facet: facet, operation: operation,
                message: message,
                cxxDescription: cxxDescription, file: file, line: line)
        } else {
            fatalError("unexpected RequestFailedException type: \(typeId)")
        }
    }

    static func registeredException(
        _ typeId: String, kindOfObject: String, objectId: String, message: String,
        cxxDescription: String, file: String,
        line: Int32
    ) -> Error {
        switch typeId {
        case "::Ice::AlreadyRegisteredException":
            AlreadyRegisteredException(
                kindOfObject: kindOfObject, id: objectId, message: message, cxxDescription: cxxDescription,
                file: file,
                line: line)
        case "::Ice::NotRegisteredException":
            NotRegisteredException(
                kindOfObject: kindOfObject, id: objectId, message: message, cxxDescription: cxxDescription,
                file: file,
                line: line)
        default:
            fatalError("unexpected XxxRegisteredException type: \(typeId)")
        }
    }

    static func connectionClosedException(
        _ typeId: String, closedByApplication: Bool, message: String, cxxDescription: String,
        file: String, line: Int32
    ) -> Error {
        switch typeId {
        case "::Ice::ConnectionAbortedException":
            ConnectionAbortedException(
                closedByApplication: closedByApplication, message: message, cxxDescription: cxxDescription,
                file: file,
                line: line)
        case "::Ice::ConnectionClosedException":
            ConnectionClosedException(
                closedByApplication: closedByApplication, message: message, cxxDescription: cxxDescription,
                file: file,
                line: line)
        default:
            fatalError("unexpected ConnectionClosedException type: \(typeId)")
        }
    }

    static func localException(
        _ typeId: String, message: String, cxxDescription: String, file: String, line: Int32
    )
        -> Error
    {
        let className = typeId.dropFirst(2).replacingOccurrences(of: "::", with: ".")
        return if let localExceptionType = NSClassFromString(className) as? LocalException.Type {
            localExceptionType.init(
                message: message, cxxDescription: cxxDescription, file: file, line: line)
        } else {
            CxxLocalException(
                typeId: typeId, message: message, cxxDescription: cxxDescription, file: file, line: line)
        }
    }

    static func cxxException(_ typeName: String, message: String) -> Error {
        CxxLocalException(
            typeId: typeName, message: message, cxxDescription: "\(typeName) \(message)", file: "???",
            line: 0)
    }
}
