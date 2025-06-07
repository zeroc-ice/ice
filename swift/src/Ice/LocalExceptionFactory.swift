// Copyright (c) ZeroC, Inc.

import IceImpl

final class LocalExceptionFactory: ICELocalExceptionFactory {
    static func dispatchException(_ replyStatus: UInt8, message: String, file: String, line: Int32) -> Error {
        if let replyStatusEnum = ReplyStatus(rawValue: replyStatus) {
            switch replyStatusEnum {
            case .unknownException:
                UnknownException(message, file: file, line: line)
            case .unknownLocalException:
                UnknownLocalException(message, file: file, line: line)
            case .unknownUserException:
                UnknownUserException(message, file: file, line: line)
            default:
                DispatchException(replyStatus: replyStatus, message: message, file: file, line: line)
            }
        } else {
            DispatchException(replyStatus: replyStatus, message: message, file: file, line: line)
        }
    }

    static func requestFailedException(
        _ replyStatus: UInt8, name: String, category: String, facet: String, operation: String, file: String,
        line: Int32
    ) -> Error {
        switch ReplyStatus(rawValue: replyStatus)! {
        case .objectNotExist:
            ObjectNotExistException(
                id: Identity(name: name, category: category), facet: facet, operation: operation, file: file, line: line
            )
        case .facetNotExist:
            FacetNotExistException(
                id: Identity(name: name, category: category), facet: facet, operation: operation, file: file, line: line
            )
        case .operationNotExist:
            OperationNotExistException(
                id: Identity(name: name, category: category), facet: facet, operation: operation, file: file, line: line
            )
        default:
            fatalError("unexpected RequestFailedException with reply status: \(replyStatus)")
        }
    }

    static func registeredException(
        _ typeId: String, kindOfObject: String, objectId: String, message: String, file: String, line: Int32
    ) -> Error {
        switch typeId {
        case "::Ice::AlreadyRegisteredException":
            AlreadyRegisteredException(
                kindOfObject: kindOfObject, id: objectId, message: message,
                file: file,
                line: line)
        case "::Ice::NotRegisteredException":
            NotRegisteredException(
                kindOfObject: kindOfObject, id: objectId, message: message,
                file: file,
                line: line)
        default:
            fatalError("unexpected XxxRegisteredException type: \(typeId)")
        }
    }

    static func connectionClosedException(
        _ typeId: String, closedByApplication: Bool, message: String, file: String, line: Int32
    ) -> Error {
        switch typeId {
        case "::Ice::ConnectionAbortedException":
            ConnectionAbortedException(
                closedByApplication: closedByApplication, message: message,
                file: file,
                line: line)
        case "::Ice::ConnectionClosedException":
            ConnectionClosedException(
                closedByApplication: closedByApplication, message: message,
                file: file,
                line: line)
        default:
            fatalError("unexpected ConnectionClosedException type: \(typeId)")
        }
    }

    static func localException(_ typeId: String, message: String, file: String, line: Int32) -> Error {
        let className = typeId.dropFirst(2).replacingOccurrences(of: "::", with: ".")
        return if let localExceptionType = NSClassFromString(className) as? LocalException.Type {
            localExceptionType.init(message, file: file, line: line)
        } else {
            CxxLocalException(typeId: typeId, message: message, file: file, line: line)
        }
    }

    static func cxxException(_ typeName: String, message: String) -> Error {
        CxxLocalException(typeId: typeName, message: message, file: "???", line: 0)
    }
}
