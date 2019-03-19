// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Dispatch

func stringToEncodingVersion(_ s: String) throws -> EncodingVersion {
    let (major, minor) = try stringToMajorMinor(s)
    return EncodingVersion(major: major, minor: minor)
}

func stringToMajorMinor(_ s: String) throws -> (UInt8, UInt8) {
    let components = s.components(separatedBy: ".")
    guard components.count == 2 else {
        throw VersionParseException(str: "malformed value `\(s)'")
    }

    guard let major = UInt8(components[0] as String), let minor = UInt8(components[1]) else {
        throw VersionParseException(str: "invalid version value `\(s)'")
    }

    return (major, minor)
}

func createSentCallback(sent: ((Bool) -> Void)? = nil, sentOn: DispatchQueue?) -> ((Bool) -> Void)? {
    var sentCallback: ((Bool) -> Void)?
    if let s = sent {
        sentCallback = { (sentSynchronously: Bool) -> Void in
            if let queue = sentOn {
                queue.async {
                    s(sentSynchronously)
                }
            }
            s(sentSynchronously)
        }
    }
    return sentCallback
}
