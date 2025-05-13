// Copyright (c) ZeroC, Inc.

import Dispatch
import IceImpl

/// Converts a string to an encoding version.
///
/// - parameter s: `String` - The string to convert.
///
/// - returns: `Ice.EncodingVersion` - The converted encoding version.</returns>
func stringToEncodingVersion(_ s: String) throws -> EncodingVersion {
    let (major, minor) = try stringToMajorMinor(s)
    return EncodingVersion(major: major, minor: minor)
}

func stringToMajorMinor(_ s: String) throws -> (UInt8, UInt8) {
    let components = s.components(separatedBy: ".")
    guard components.count == 2 else {
        throw ParseException("malformed Ice version string '\(s)'")
    }

    guard let major = UInt8(components[0] as String), let minor = UInt8(components[1]) else {
        throw ParseException("invalid Ice version value '\(s)'")
    }

    return (major, minor)
}

func checkSupportedEncoding(_ v: EncodingVersion) throws {
    let c = currentEncoding
    if v.major != c.major || v.minor > c.minor {
        throw MarshalException(
            "this Ice runtime does not support encoding version \(encodingVersionToString(v))")
    }
}
