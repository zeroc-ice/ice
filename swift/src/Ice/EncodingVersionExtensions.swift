// Copyright (c) ZeroC, Inc.

import Foundation

extension EncodingVersion {
    /// Returns the 1.0 encoding version.
    static public var Encoding_1_0 : EncodingVersion {
        EncodingVersion(major: 1, minor: 0)
    }

    /// Returns the 1.1 encoding version.
    static public var Encoding_1_1 : EncodingVersion {
        EncodingVersion(major: 1, minor: 1)
    }

    /// Returns the current encoding version.
    static public var currentEncoding : EncodingVersion {
        return EncodingVersion.Encoding_1_1
    }

    /// The current protocol encoding version.
    static internal var currentProtocolEncoding: EncodingVersion {
        return EncodingVersion.Encoding_1_0
    }
}
