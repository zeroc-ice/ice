//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestCommon
import IceProperties


class TestFactory {
    public static func create(name: String) throws -> TestCommon.TestHelper {
        if(name == "Ice.properties.Client") {
            return IceProperties.Client()
        }
        throw NSError(domain: "Test not found \(name)", code:1024)
    }
}
