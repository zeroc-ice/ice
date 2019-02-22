// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Foundation

//
// Slice classes and exceptions extend Ice.ClassResolver with a static func
// which returns the Swift Value.Type or UserException.Type.
//
// We subclass NSObject so that we can use selectors.
//
// @objc dynamic Module_MyClass() -> Value.Type

public class ClassResolver: NSObject {
    private static func mangle(typeId: String) -> String {
        precondition(typeId.count > 2, "invalid type id")

        guard let r = typeId.range(of: "::") else {
            preconditionFailure("invalid type id")
        }

        precondition(r.lowerBound == typeId.startIndex, "invalid type id")

        let c = typeId.components(separatedBy: "::")[1...]
        return c.joined(separator: "_")
    }

    static func resolve(typeId: String) -> Value.Type? {
        let selector = Selector(mangle(typeId: typeId))

        guard ClassResolver.responds(to: selector) else {
            return nil
        }

        #warning("TODO: check this is correct wrt memory management")
        let val = ClassResolver.perform(Selector(mangle(typeId: typeId))).takeRetainedValue()

        guard let valueType = val as? Value.Type else {
            preconditionFailure("unexpected value type")
        }
        return valueType
    }

    static func resolve(typeId: String) -> UserException.Type? {
        let selector = Selector(mangle(typeId: typeId))

        guard ClassResolver.responds(to: selector) else {
            return nil
        }

        #warning("TODO: check this is correct wrt memory management")
        let val = ClassResolver.perform(selector).takeRetainedValue()

        guard let valueType = val as? UserException.Type else {
            preconditionFailure("unexpected value type")
        }
        return valueType
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
