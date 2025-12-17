// Copyright (c) ZeroC, Inc.

/// Abstract base class for all exceptions defined in Slice.
///
/// User exceptions are marked as `@unchecked Sendable` as the error protocol requires them to be `Sendable`.
/// However, generated exception classes are not thread-safe; do not share instances between threads.
open class UserException: Exception, @unchecked Sendable {
    public required init() {}

    /// Returns the Slice type ID of this user exception.
    ///
    /// - Returns: The return value is always `"::Ice::UserException"`.
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
    /// Returns the Slice type ID of this user exception.
    ///
    /// - Returns: The Slice type ID.
    public func ice_id() -> String { type(of: self).ice_staticId() }
}
