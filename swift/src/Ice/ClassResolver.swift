// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Foundation

open class ValueTypeResolver: NSObject {
    open func type() -> Value.Type {
        precondition(false, "Abstract method")
    }
}

open class UserExceptionTypeResolver: NSObject {
    open func type() -> UserException.Type {
        precondition(false, "Abstract method")
    }
}

//
// Slice classes and exceptions generate an extesion for
// Ice.ClassResolver with a static func which returns a
// ValueTypeRessolver or UserExceptionTypeRessolver.
//

public class ClassResolver: NSObject {

    static private func resolveImpl(typeId: String) -> AnyObject? {
        let start = typeId.index(typeId.startIndex, offsetBy: 2)
        let selector = Selector(typeId[start...].replacingOccurrences(of: "::", with: "_"))
        guard ClassResolver.responds(to: selector) else {
            return nil
        }
        return ClassResolver.perform(selector).takeUnretainedValue()
    }

    static func resolve(typeId: String) -> Value.Type? {
        return (resolveImpl(typeId: typeId) as! ValueTypeResolver).type()
    }

    static func resolve(typeId: String) -> UserException.Type? {
        return (resolveImpl(typeId: typeId) as! UserExceptionTypeResolver).type()
    }
}

public class TypeIdResolver: NSObject {
    static func resolve(compactId: Int32) -> String? {
        let selector = Selector("TypeId_\(compactId)")

        guard TypeIdResolver.responds(to: selector) else {
            return nil
        }

        #warning("TODO: check this is correct wrt memory management")
        let val = TypeIdResolver.perform(selector).takeRetainedValue()

        guard let typeId = val as? String else {
            preconditionFailure("unexpected value type")
        }
        return typeId
    }
}
