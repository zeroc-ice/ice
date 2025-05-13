// Copyright (c) ZeroC, Inc.

import Foundation

/// The output mode for xxxToString method such as identityToString and proxyToString. The actual encoding format for
/// the string is the same for all modes: you don't need to specify an encoding format or mode when reading such a
/// string.
public enum ToStringMode: UInt8 {
    /// Unicode Characters with ordinal values greater than 127 are kept as-is in the resulting string. Non-printable
    /// ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
    case Unicode = 0
    /// ASCII Characters with ordinal values greater than 127 are encoded as universal character names in the resulting
    /// string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters. Non-printable ASCII characters
    /// with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
    case ASCII = 1
    /// Compat Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal
    /// escapes.
    /// Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or an octal escape. Use this mode
    /// to generate strings compatible with Ice 3.6 and earlier.
    case Compat = 2

    public init() {
        self = .Unicode
    }
}

/// An `Ice.InputStream` extension to read `ToStringMode` enumerated values from the stream.
extension InputStream {
    /// Read an enumerated value.
    ///
    /// - returns: `ToStringMode` - The enumarated value.
    public func read() throws -> ToStringMode {
        let rawValue: UInt8 = try read(enumMaxValue: 2)
        guard let val = ToStringMode(rawValue: rawValue) else {
            throw MarshalException("invalid enum value")
        }
        return val
    }

    /// Read an optional enumerated value from the stream.
    ///
    /// - parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// - returns: `ToStringMode` - The enumerated value.
    public func read(tag: Int32) throws -> ToStringMode? {
        guard try readOptional(tag: tag, expectedFormat: .Size) else {
            return nil
        }
        return try read() as ToStringMode
    }
}

/// An `Ice.OutputStream` extension to write `ToStringMode` enumerated values to the stream.
extension OutputStream {
    /// Writes an enumerated value to the stream.
    ///
    /// - parameter value: `ToStringMode` - The enumerator to write.
    public func write(_ value: ToStringMode) {
        write(enum: value.rawValue, maxValue: 2)
    }

    /// Writes an optional enumerated value to the stream.
    ///
    /// - parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// - parameter value: `ToStringMode` - The enumerator to write.
    public func write(tag: Int32, value: ToStringMode?) {
        guard let v = value else {
            return
        }
        write(tag: tag, val: v.rawValue, maxValue: 2)
    }
}
