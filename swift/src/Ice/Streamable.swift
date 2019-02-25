// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public protocol Streamable {
    init(from ins: InputStream) throws
    func ice_write(to os: OutputStream)
}

public protocol StreamableValue {
    init()
    mutating func ice_read(from ins: InputStream) throws
    func ice_write(to os: OutputStream)
}

public extension Streamable where Self: Numeric {
    func ice_write(to os: OutputStream) {
        os.write(numeric: self)
    }
}

extension UInt8: Streamable {
    public init(from ins: InputStream) throws {
        self = try ins.read(as: UInt8.self)
    }
}

extension Int16: Streamable {
    public init(from ins: InputStream) throws {
        self = try ins.read(as: Int16.self)
    }
}

extension Int32: Streamable {
    public init(from ins: InputStream) throws {
        self = try ins.read(as: Int32.self)
    }
}

extension Int64: Streamable {
    public init(from ins: InputStream) throws {
        self = try ins.read(as: Int64.self)
    }
}

extension Float: Streamable {
    public init(from ins: InputStream) throws {
        self = try ins.read(as: Float.self)
    }
}

extension Double: Streamable {
    public init(from ins: InputStream) throws {
        self = try ins.read(as: Double.self)
    }
}

extension Bool: Streamable {
    public init(from ins: InputStream) throws {
        self = try ins.read(as: UInt8.self) == 1
    }

    public mutating func ice_read(from ins: InputStream) throws {
        self = try ins.read(as: UInt8.self) == 1
    }

    public func ice_write(to os: OutputStream) {
        os.write(numeric: UInt8(self == true ? 1 : 0))
    }
}

extension String: Streamable {
    public init(from _: InputStream) throws {
        preconditionFailure("not implemented")
    }

    public mutating func ice_read(from _: InputStream) throws {
        preconditionFailure("not implemented")
    }

    public func ice_write(to os: OutputStream) {
        os.write(utf8view: utf8)
    }
}

extension Array where Element: Streamable {
    public init(from ins: InputStream, minSize: Int) throws {
        let sz = try Int(ins.readAndCheckSeqSize(minSize: minSize))
        var a = [Element]()
        a.reserveCapacity(sz)
        for i in 0 ..< sz {
            a[i] = try Element(from: ins)
        }
        self = a
    }

    public func ice_write(to os: OutputStream) {
        os.write(array: self)
    }
}

extension Array where Element == String {
    public init(from ins: InputStream) throws {
        self = try [Element].init(from: ins, minSize: 1)
    }
}

extension Array where Element: Numeric, Element: Streamable {
    public init(from ins: InputStream) throws {
        self = try [Element].init(from: ins, minSize: MemoryLayout<Element.IntegerLiteralType>.size)
    }
}

extension Dictionary where Key: Streamable, Value: Streamable {
    public init(from _: InputStream) throws {
        preconditionFailure("not implemented")
    }

    public mutating func ice_read(from _: InputStream) throws {
        preconditionFailure("not implemented")
    }

    public func ice_write(to _: OutputStream) {
        preconditionFailure("not implemented")
    }
}

extension Optional where Wrapped: Streamable {
    public init() {
        self = Optional.none
    }

    public init(from _: InputStream) throws {
        preconditionFailure("not implemented")
    }

    public mutating func ice_read(from _: InputStream) throws {
        preconditionFailure("not implemented")
    }

    public func ice_write(to _: OutputStream) {
        preconditionFailure("not implemented")
    }
}
