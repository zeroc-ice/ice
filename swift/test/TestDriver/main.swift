//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import IceAcm
import TestCommon

var client: TestHelper?

if CommandLine.arguments.count < 2 {
    print("Usage: \(CommandLine.arguments[0]) <test> <args>")
    exit(1)
}

var name = CommandLine.arguments[1]

if name == "Ice.acm.Client" {
    client = IceStream.Client()
}

do {
    if try client?.run(args: CommandLine.arguments) == nil {
        print("Unknown client \(name)")
        exit(1)
    }
} catch {
    print("Error \(error)")
    exit(1)
}
