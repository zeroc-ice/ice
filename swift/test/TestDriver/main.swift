//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import TestCommon

if CommandLine.arguments.count < 2 {
    print("Usage: \(CommandLine.arguments[0]) <test> <args>")
    exit(1)
}

var name = CommandLine.arguments[1]

do {
    let client = try TestFactory.create(name: name)
    if try client.run(args: CommandLine.arguments) == nil {
        print("Unknown client \(name)")
        exit(1)
    }
} catch {
    print("Error \(error)")
    exit(1)
}
