//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestCommon
import IceProperties
import IceDefaultValue

class TestFactory {
    public static func create(name: String) throws -> TestCommon.TestHelper {
        if name == "Ice.properties.Client" {
            return IceProperties.Client()
        } else if name == "Ice.defaultValue.Client" {
            return IceDefaultValue.Client()
        }
        throw NSError(domain: "Test not found \(name)", code: 1024)
    }
}
