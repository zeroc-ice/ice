//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestCommon
import IceProperties
import IceProxy
import IceDefaultValue
import IceEnums
import IceStream

let tests: [String: () -> TestHelper] = [
    "Ice.properties.Client": { IceProperties.Client() },
    "Ice.proxy.Client": { IceProxy.Client() },
    "Ice.defaultValue.Client": { IceDefaultValue.Client() },
    "Ice.stream.Client": { IceStream.Client() },
    "Ice.enums.Client": { IceEnums.Client() },
    "Ice.enums.Server": { IceEnums.Server() }
]

class TestFactory {
    public static func create(name: String) throws -> TestCommon.TestHelper {
        if let test = tests[name] {
            return test()
        } else {
            throw NSError(domain: "Test not found \(name)", code: 1024)
        }
    }
}
