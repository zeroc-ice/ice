// Copyright (c) ZeroC, Inc.

/// Base class for Ice user exceptions.
open class UserException: Exception {
    public required init() {}

    /// Gets the type ID of the class.
    /// - Returns: The type ID of the class.
    open class func ice_staticId() -> String { "::Ice::UserException" }

    open func _iceReadImpl(from _: InputStream) throws {}
    open func _iceWriteImpl(to _: OutputStream) {}

    open func _usesClasses() -> Bool { false }

    open func _iceRead(from istr: InputStream) throws {
        istr.startException()
        try _iceReadImpl(from: istr)
        try istr.endException()
    }

    open func _iceWrite(to ostr: OutputStream) {
        ostr.startException()
        _iceWriteImpl(to: ostr)
        ostr.endException()
    }
}

extension UserException {
    /// Gets the type ID of this Ice user exception.
    /// - Returns: The type ID of this Ice user exception.
    public func ice_id() -> String { type(of: self).ice_staticId() }
}
